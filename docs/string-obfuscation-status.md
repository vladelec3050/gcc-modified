# GCC string obfuscation status

## Implemented subset

This checkout is GCC 17.0.0. The new common option `-fobfuscate-strings` is
disabled by default. When enabled, the GIMPLE pass transforms eligible `char`
and `wchar_t` string literal arguments in direct calls, including C++ member
calls and array-reference parameters. It also rewrites literal addresses in
compiler-generated static initializer-list backing arrays when those arrays
are passed to runtime calls. This covers tested `std::string`, `std::wstring`,
vector, map, unordered-map, nested-vector, and user-structure construction
without matching STL names. The pass leaves original `STRING_CST` nodes
unchanged, stores XOR-encrypted target bytes in private static arrays, and adds
initialization code to decode them before normal program constructors. The
bytes are obfuscation, not cryptographic protection.

The implementation does not handle indirect calls, variadic arguments, UTF
character types other than `wchar_t`, arbitrary user-declared global/static
initializers, literals used outside supported calls and initializer-list
backing arrays, or LTO. Other argument types are left untouched. Decoded
buffers have static lifetime. Each eligible use gets a distinct buffer, so
pointer identity can differ from a non-obfuscated build.

## Build script

`contrib/build-mingw-obfuscating-gcc.sh` configures an out-of-tree
`x86_64-w64-mingw32` compiler. It builds and installs GCC itself, using the
installed MinGW-w64 headers, libraries, and binutils through a private sysroot
view. It reuses the installed target runtime libraries; building target
`libgcc` from this source checkout is not part of the script because this
Debian MinGW sysroot's header layout caused GCC's `all-target-libgcc` build to
fail at `#include_next <stddef.h>`.

## Verification

The cross compiler built from this source is installed under
`/home/bound/Desktop/install-gcc-obf-mingw`; its out-of-tree build is under
`/home/bound/Desktop/build-gcc-obf-mingw`. GCC and the C++ driver built and
installed successfully with `contrib/build-mingw-obfuscating-gcc.sh`. Target
`libgcc` could not be built because the Debian MinGW headers fail GCC's
`#include_next <stddef.h>` check, so the script links the installed MinGW 14
`libgcc`, C++ runtime, headers, and CRT objects into the GCC 17 install.

The C runtime regression and the C++ STL integration test were compiled with
the modified compiler at `-O0`, `-O2`, and `-O3`, linked to PE executables, and
run under Wine. The C++ test covers narrow and wide string initialization,
construction and assignment; adjacent literals; vector initialization,
`push_back` and `emplace_back`; map and unordered-map entries; nested vectors;
and a user structure with string fields. It also contains a `constexpr`
character array and `static_assert`. Every run exited successfully. A whole-file
PE scan checked all 25 unique test literals in ASCII and UTF-16LE and found no
plaintext matches at any optimization level. A default-options C++ build ran
successfully and retained plaintext markers, confirming the option is off by
default. The GCC DejaGNU tests were added but the complete GCC testsuite was
not run. No broader support is claimed.

The tests are `gcc/testsuite/gcc.dg/obfuscate-strings-run.c` and
`gcc/testsuite/g++.dg/obfuscate-strings-stl.C`; they check runtime value
recovery, including STL and wide-string uses.
