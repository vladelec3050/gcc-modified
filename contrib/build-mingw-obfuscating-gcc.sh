#!/bin/sh
# Configure and build this GCC checkout as a separate MinGW-w64 compiler.
set -eu

src_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
parent_dir=$(dirname -- "$src_dir")
build_dir=${BUILD_DIR:-"$parent_dir/build-gcc-obf-mingw"}
prefix=${PREFIX:-"$parent_dir/install-gcc-obf-mingw"}
target=${TARGET:-x86_64-w64-mingw32}
jobs=${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
prereq_prefix=${PREREQ_PREFIX:-}

for tool in gcc g++ make bison awk; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "required host tool not found: $tool" >&2
    exit 1
  fi
done
for tool in "$target-gcc" "$target-as" "$target-ld" "$target-ar"; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "required MinGW-w64 tool not found: $tool" >&2
    exit 1
  fi
done

toolchain_root=$($target-gcc -print-sysroot)
if [ -z "$toolchain_root" ]; then
  # Debian's MinGW-w64 packages keep headers and libraries in this target root
  # while the compiler reports an empty --print-sysroot.
  toolchain_root=/usr/$target
fi
if [ ! -f "$toolchain_root/include/windows.h" ] || [ ! -d "$toolchain_root/lib" ]; then
  echo "MinGW-w64 headers/libraries not found under $toolchain_root" >&2
  exit 1
fi

# GCC's MinGW target headers are searched below /mingw in the sysroot.  The
# Debian cross package keeps the same files directly under /usr/$target, so
# build a private sysroot view without changing the installed toolchain.
sysroot=${SYSROOT:-"$build_dir/sysroot"}
if [ -z "${SYSROOT:-}" ]; then
  if [ ! -e "$sysroot/include" ]; then
    ln -s "$toolchain_root/include" "$sysroot/include"
  fi
  if [ ! -e "$sysroot/lib" ]; then
    ln -s "$toolchain_root/lib" "$sysroot/lib"
  fi
  mkdir -p "$sysroot/mingw"
  if [ ! -e "$sysroot/mingw/include" ]; then
    ln -s "$toolchain_root/include" "$sysroot/mingw/include"
  fi
  if [ ! -e "$sysroot/mingw/lib" ]; then
    ln -s "$toolchain_root/lib" "$sysroot/mingw/lib"
  fi
fi
if [ ! -f "$sysroot/mingw/include/windows.h" ] \
   || [ ! -d "$sysroot/mingw/lib" ]; then
  echo "MinGW-w64 headers/libraries not found under $sysroot/mingw" >&2
  exit 1
fi

with_prereqs=
if [ -n "$prereq_prefix" ]; then
  if [ ! -f "$prereq_prefix/include/gmp.h" ] \
     || [ ! -f "$prereq_prefix/include/mpfr.h" ] \
     || [ ! -f "$prereq_prefix/include/mpc.h" ]; then
    echo "GMP/MPFR/MPC development files not found under $prereq_prefix" >&2
    exit 1
  fi
  with_prereqs="--with-gmp=$prereq_prefix --with-mpfr=$prereq_prefix --with-mpc=$prereq_prefix"
  export CPPFLAGS="-I$prereq_prefix/include ${CPPFLAGS:-}"
  export LDFLAGS="-L$prereq_prefix/lib ${LDFLAGS:-}"
fi

mkdir -p "$build_dir"
cd "$build_dir"
if [ ! -f Makefile ] || [ "${RECONFIGURE:-0}" = 1 ]; then
  "$src_dir/configure" \
    --target="$target" \
    --prefix="$prefix" \
    --with-as="$(command -v "$target-as")" \
    --with-ld="$(command -v "$target-ld")" \
    --with-sysroot="$sysroot" \
    --with-native-system-header-dir=/include \
    --enable-languages=c,c++ \
    --disable-bootstrap \
    --disable-multilib \
    --disable-nls \
    --disable-werror \
    $with_prereqs
fi

make -j"$jobs" all-gcc
make install-gcc

# This compiler build intentionally reuses the installed target runtime rather
# than building libgcc against the Debian MinGW header layout.  GCC searches
# its own versioned libdir first, so expose the compatible target runtime there.
target_libgcc=$($target-gcc -print-file-name=libgcc.a)
if [ ! -f "$target_libgcc" ]; then
  echo "installed MinGW target libgcc not found" >&2
  exit 1
fi
target_runtime_dir=$(dirname "$target_libgcc")
gcc_version=$("$prefix/bin/$target-gcc" -dumpversion)
gcc_libdir="$prefix/lib/gcc/$target/$gcc_version"
mkdir -p "$gcc_libdir"
for runtime_file in "$target_runtime_dir"/libgcc*.a \
                    "$target_runtime_dir"/libgcc*.dll \
                    "$target_runtime_dir"/libstdc++* \
                    "$target_runtime_dir"/libsupc++* \
                    "$target_runtime_dir"/crtbegin*.o \
                    "$target_runtime_dir"/crtend*.o \
                    "$target_runtime_dir"/libgcov*.a; do
  if [ -f "$runtime_file" ]; then
    ln -sf "$runtime_file" "$gcc_libdir/$(basename "$runtime_file")"
  fi
done
target_cxx_include="$target_runtime_dir/include/c++"
if [ -d "$target_cxx_include" ]; then
  gcc_cxx_include="$prefix/$target/include/c++/$gcc_version"
  mkdir -p "$(dirname "$gcc_cxx_include")"
  ln -sfn "$target_cxx_include" "$gcc_cxx_include"
fi
echo "Installed compiler under $prefix"
