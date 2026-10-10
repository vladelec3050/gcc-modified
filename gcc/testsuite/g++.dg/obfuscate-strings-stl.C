/* { dg-do run } */
/* { dg-options "-O0 -fobfuscate-strings" } */

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct Config
{
  std::string name;
  std::wstring description;
};

int
main ()
{
  constexpr char compile_time[] = {'C', 'O', 'N', 'S', 'T', 'E', 'X', 'P', 'R', 0};
  static_assert (sizeof (compile_time) == 10 && compile_time[0] == 'C');

  std::string init = "TEST_STRING_INIT_72931";
  std::string ctor ("TEST_STRING_CTOR_83942");
  std::string assigned;
  assigned = "TEST_STRING_ASSIGN_29471";
  std::string adjacent = "TEST_ADJACENT_" "LITERAL_78231";

  std::wstring wide_init = L"TEST_WSTRING_INIT_58139";
  std::wstring wide_ctor (L"TEST_WSTRING_CTOR_62914");
  std::wstring wide_assigned;
  wide_assigned = L"TEST_WSTRING_ASSIGN_74820";

  std::vector<std::string> strings = {
    "TEST_VECTOR_FIRST_18273", "TEST_VECTOR_SECOND_92831"
  };
  strings.push_back ("TEST_VECTOR_PUSH_51928");
  strings.emplace_back ("TEST_VECTOR_EMPLACE_39281");

  std::vector<std::wstring> wide_strings = {
    L"TEST_WVECTOR_FIRST_10384", L"TEST_WVECTOR_SECOND_92740"
  };
  wide_strings.push_back (L"TEST_WVECTOR_PUSH_82714");
  wide_strings.emplace_back (L"TEST_WVECTOR_EMPLACE_50831");

  std::map<std::string, std::wstring> mappings = {
    {"TEST_MAP_KEY_70284", L"TEST_MAP_VALUE_63108"}
  };
  std::unordered_map<std::string, std::string> settings = {
    {"TEST_UMAP_KEY_81430", "TEST_UMAP_VALUE_28947"}
  };
  std::vector<std::vector<std::string>> nested = {
    {"TEST_NESTED_FIRST_31640", "TEST_NESTED_SECOND_90217"}
  };
  Config config = {"TEST_STRUCT_NAME_72196", L"TEST_STRUCT_DESC_40318"};

  if (init != "TEST_STRING_INIT_72931"
      || ctor != "TEST_STRING_CTOR_83942"
      || assigned != "TEST_STRING_ASSIGN_29471"
      || adjacent != "TEST_ADJACENT_" "LITERAL_78231"
      || wide_init != L"TEST_WSTRING_INIT_58139"
      || wide_ctor != L"TEST_WSTRING_CTOR_62914"
      || wide_assigned != L"TEST_WSTRING_ASSIGN_74820"
      || strings.size () != 4
      || strings[0] != "TEST_VECTOR_FIRST_18273"
      || strings[1] != "TEST_VECTOR_SECOND_92831"
      || strings[2] != "TEST_VECTOR_PUSH_51928"
      || strings[3] != "TEST_VECTOR_EMPLACE_39281"
      || wide_strings.size () != 4
      || wide_strings[0] != L"TEST_WVECTOR_FIRST_10384"
      || wide_strings[1] != L"TEST_WVECTOR_SECOND_92740"
      || wide_strings[2] != L"TEST_WVECTOR_PUSH_82714"
      || wide_strings[3] != L"TEST_WVECTOR_EMPLACE_50831"
      || mappings["TEST_MAP_KEY_70284"] != L"TEST_MAP_VALUE_63108"
      || settings["TEST_UMAP_KEY_81430"] != "TEST_UMAP_VALUE_28947"
      || nested[0][0] != "TEST_NESTED_FIRST_31640"
      || nested[0][1] != "TEST_NESTED_SECOND_90217"
      || config.name != "TEST_STRUCT_NAME_72196"
      || config.description != L"TEST_STRUCT_DESC_40318")
    return 1;

  return 0;
}
