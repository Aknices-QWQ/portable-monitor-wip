#!/usr/bin/env bash
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

make -C "${SCRIPT_DIR}" -f Makefile.device "$@"
