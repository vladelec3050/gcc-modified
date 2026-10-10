/* Runtime obfuscation of narrow string arguments.
   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "fold-const.h"
#include "stringpool.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "function.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-ssa.h"
#include "tree-ssanames.h"
#include "cgraph.h"
#include "toplev.h"
#include "options.h"

namespace {

/* A simple compile-time stream generator.  This is an obfuscation format,
   not a cryptographic cipher.  */
static uint64_t
stream_byte (uint64_t seed, unsigned HOST_WIDE_INT pos)
{
  uint64_t z = seed + UINT64_C (0x9e3779b97f4a7c15) * (pos + 1);
  z = (z ^ (z >> 30)) * UINT64_C (0xbf58476d1ce4e5b9);
  z = (z ^ (z >> 27)) * UINT64_C (0x94d049bb133111eb);
  z ^= z >> 31;
  return z >> 56;
}

static unsigned HOST_WIDE_INT obfuscation_ordinal;
static GTY(()) tree obfuscation_ctor_body;

/* The wide character type is language-specific state in c-family; inspect
   the front-end type node so this pass remains linkable into LTO executables.  */
static bool
obfuscation_code_unit_p (tree type)
{
  type = TYPE_MAIN_VARIANT (type);
  if (type == char_type_node)
    return true;
  tree name = TYPE_NAME (type);
  if (name && TREE_CODE (name) == TYPE_DECL)
    name = DECL_NAME (name);
  return (name && TREE_CODE (name) == IDENTIFIER_NODE
          && !strcmp (IDENTIFIER_POINTER (name), "wchar_t"));
}

/* Make a writable data object containing CIPHERTEXT and append stores which
   decrypt it to CTOR_BODY.  */
static tree
make_obfuscated_buffer (tree string, tree *ctor_body)
{
  const unsigned len = TREE_STRING_LENGTH (string);
  const char *plain = TREE_STRING_POINTER (string);
  char *cipher = XNEWVEC (char, len);
  uint64_t seed = (uint64_t) get_random_seed (false);
  seed ^= (uint64_t) ++obfuscation_ordinal
    * UINT64_C (0xd6e8feb86659fd93);

  for (unsigned i = 0; i < len; ++i)
    cipher[i] = plain[i] ^ (unsigned char) stream_byte (seed, i);

  tree byte_type = unsigned_char_type_node;
  tree array_type = build_array_type_nelts (byte_type, len);
  char name[64];
  snprintf (name, sizeof (name), "__gcc_obfuscated_string_%llu",
            (unsigned long long) obfuscation_ordinal);
  tree buffer = build_decl (UNKNOWN_LOCATION, VAR_DECL,
                            get_identifier (name), array_type);
  TREE_STATIC (buffer) = 1;
  TREE_PUBLIC (buffer) = 0;
  TREE_ADDRESSABLE (buffer) = 1;
  tree element_type = TREE_TYPE (TREE_TYPE (string));
  if (TYPE_MAIN_VARIANT (element_type) != char_type_node)
    SET_DECL_ALIGN (buffer, TYPE_ALIGN (element_type));
  DECL_ARTIFICIAL (buffer) = 1;
  DECL_IGNORED_P (buffer) = 1;
  DECL_INITIAL (buffer) = build_string (len, cipher);
  TREE_TYPE (DECL_INITIAL (buffer)) = array_type;
  varpool_node::finalize_decl (buffer);
  varpool_node::get_create (buffer)->force_output = true;
  XDELETEVEC (cipher);

  for (unsigned i = 0; i < len; ++i)
    {
      tree index = build_int_cst (size_type_node, i);
      /* Keep the store and load references distinct.  Gimplification may
        rewrite either reference in place, so sharing one ARRAY_REF node
        between both sides of the assignment corrupts the tree.  */
      tree lhs = build4 (ARRAY_REF, byte_type, buffer, index,
                         NULL_TREE, NULL_TREE);
      tree rhs = build4 (ARRAY_REF, byte_type, buffer,
                         build_int_cst (size_type_node, i),
                         NULL_TREE, NULL_TREE);
      tree key = build_int_cst (byte_type, stream_byte (seed, i));
      tree value = build2 (BIT_XOR_EXPR, byte_type, rhs, key);
      tree assign = build2 (MODIFY_EXPR, byte_type, lhs, value);
      append_to_statement_list (assign, ctor_body);
    }
  return buffer;
}

/* Return a narrow or wide STRING_CST for the restricted form &"...".  */
static tree
direct_string_argument (tree arg, tree formal_type)
{
  if (!formal_type)
    return NULL_TREE;
  tree formal_element = formal_type;
  if (TREE_CODE (formal_element) == REFERENCE_TYPE
      || TREE_CODE (formal_element) == POINTER_TYPE)
    formal_element = TREE_TYPE (formal_element);
  if (TREE_CODE (formal_element) == ARRAY_TYPE)
    formal_element = TREE_TYPE (formal_element);
  if (!obfuscation_code_unit_p (formal_element))
    return NULL_TREE;
  tree string = arg;
  if (TREE_CODE (string) == ADDR_EXPR)
    string = TREE_OPERAND (string, 0);
  if (TREE_CODE (string) != STRING_CST
      || TYPE_MAIN_VARIANT (TREE_TYPE (TREE_TYPE (string)))
         != TYPE_MAIN_VARIANT (formal_element))
    return NULL_TREE;
  return string;
}

struct initializer_obfuscation_data
{
  tree *ctor_body;
};

/* Replace literal addresses in compiler-generated static initializer-list
   backing arrays.  These objects are runtime-only scaffolding, unlike
   user-declared constexpr data.  */
static tree
obfuscate_initializer_string (tree *tp, int *walk_subtrees, void *data)
{
  if (TREE_CODE (*tp) != ADDR_EXPR
      || TREE_CODE (TREE_OPERAND (*tp, 0)) != STRING_CST)
    return NULL_TREE;

  initializer_obfuscation_data *info
    = static_cast<initializer_obfuscation_data *> (data);
  tree string = TREE_OPERAND (*tp, 0);
  tree buffer = make_obfuscated_buffer (string, info->ctor_body);
  *tp = fold_convert (TREE_TYPE (*tp), build_fold_addr_expr (buffer));
  *walk_subtrees = 0;
  return NULL_TREE;
}

static void
obfuscate_artificial_initializer (tree arg, tree *ctor_body)
{
  if (TREE_CODE (arg) != ADDR_EXPR)
    return;
  tree decl = TREE_OPERAND (arg, 0);
  if (TREE_CODE (decl) != VAR_DECL
      || !DECL_ARTIFICIAL (decl)
      || !TREE_STATIC (decl)
      || !DECL_INITIAL (decl)
      || DECL_INITIAL (decl) == error_mark_node)
    return;

  tree initializer = DECL_INITIAL (decl);
  initializer_obfuscation_data data = { ctor_body };
  walk_tree (&initializer, obfuscate_initializer_string, &data, NULL);
  DECL_INITIAL (decl) = initializer;
}

const pass_data pass_data_obfuscate_strings =
{
  GIMPLE_PASS, /* type */
  "obfstr", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_gimple_any, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0 /* todo_flags_finish */
};

class pass_obfuscate_strings : public gimple_opt_pass
{
public:
  pass_obfuscate_strings (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_obfuscate_strings, ctxt)
  {}

  bool gate (function *) final override
  {
    return flag_obfuscate_strings;
  }

  unsigned int execute (function *fun) final override
  {
    bool changed = false;
    basic_block bb;
    FOR_EACH_BB_FN (bb, fun)
      {
        for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
             !gsi_end_p (gsi); gsi_next (&gsi))
          {
            gimple *stmt = gsi_stmt (gsi);
            if (!is_gimple_call (stmt)
                || gimple_call_internal_p (stmt))
              continue;

            gcall *call = as_a<gcall *> (stmt);
            tree fndecl = gimple_call_fndecl (call);
            if (!fndecl)
              continue;
            tree formal = DECL_ARGUMENTS (fndecl);
            for (unsigned i = 0; i < gimple_call_num_args (call); ++i)
              {
                tree arg = gimple_call_arg (call, i);
                if (!formal)
                  break;
                tree formal_type = TREE_TYPE (formal);
                formal = TREE_CHAIN (formal);
                tree string = direct_string_argument (arg, formal_type);
                if (!string)
                  {
                    obfuscate_artificial_initializer
                      (arg, &obfuscation_ctor_body);
                    continue;
                  }
                if (TREE_STRING_LENGTH (string) == 0)
                  continue;

                tree buffer = make_obfuscated_buffer (string,
                                                       &obfuscation_ctor_body);
                tree replacement_type = formal_type;
                if (TREE_CODE (formal_type) == REFERENCE_TYPE)
                  replacement_type
                    = build_pointer_type (TREE_TYPE (formal_type));
                tree replacement = fold_convert
                  (replacement_type, build_fold_addr_expr (buffer));
                tree pointer = make_temp_ssa_name (replacement_type, NULL,
                                                    "obfstr");
                gimple *init = gimple_build_assign (pointer, replacement);
                gsi_insert_before (&gsi, init, GSI_SAME_STMT);
                gimple_call_set_arg (call, i, pointer);
                changed = true;
              }
          }
      }

    return changed ? TODO_update_ssa : 0;
  }
};

const pass_data pass_data_obfuscate_strings_ipa =
{
  SIMPLE_IPA_PASS, /* type */
  "obfstr_ipa", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  0, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0 /* todo_flags_finish */
};

class pass_obfuscate_strings_ipa : public simple_ipa_opt_pass
{
public:
  pass_obfuscate_strings_ipa (gcc::context *ctxt)
    : simple_ipa_opt_pass (pass_data_obfuscate_strings_ipa, ctxt)
  {}

  bool gate (function *) final override
  {
    return flag_obfuscate_strings && obfuscation_ctor_body != NULL_TREE;
  }

  unsigned int execute (function *) final override
  {
    cgraph_build_static_cdtor ('I', obfuscation_ctor_body,
                               MAX_RESERVED_INIT_PRIORITY - 1);
    obfuscation_ctor_body = NULL_TREE;
    return 0;
  }
};

} // anon namespace

gimple_opt_pass *
make_pass_obfuscate_strings (gcc::context *ctxt)
{
  return new pass_obfuscate_strings (ctxt);
}

simple_ipa_opt_pass *
make_pass_obfuscate_strings_ipa (gcc::context *ctxt)
{
  return new pass_obfuscate_strings_ipa (ctxt);
}

#include "gt-tree-obfuscate-strings.h"
