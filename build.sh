#!/bin/bash

set -e

INSTALL_PREFIX=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --install)
      INSTALL_PREFIX="/usr/local"
      shift
      ;;
    --prefix)
      if [[ $# -lt 2 ]]; then
        echo "error: --prefix requires a path" >&2
        exit 1
      fi
      INSTALL_PREFIX="$2"
      shift 2
      ;;
    *)
      echo "usage: $0 [--install] [--prefix <path>]" >&2
      exit 1
      ;;
  esac
done

if [ ! -d build ]; then
    mkdir "build"
fi

rm -rf "$(pwd)"/build/*

cmake -S . -B build && cmake --build build

if [[ -n "$INSTALL_PREFIX" ]]; then
  cmake --install build --prefix "$INSTALL_PREFIX"
fi