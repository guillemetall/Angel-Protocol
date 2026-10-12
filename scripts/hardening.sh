#!/bin/sh
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="$ROOT/CMakeLists.txt"
BACKUP_DIR="$ROOT/.hardening-backups"
REPORT_DIR="$ROOT/.hardening-reports"
FLAGS='-Wall -Wextra -Wvla -Wshadow -Wconversion -Wsign-conversion -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wstack-usage=2048 -fstack-protector-strong'

cmd_flags() {
    echo "Aplicando flags..."
    mkdir -p "$BACKUP_DIR"
    cp "$CMAKE" "$BACKUP_DIR/CMakeLists.$(date +%s).bak"
    if grep -q "hardening:compile-flags" "$CMAKE"; then
        echo "ya aplicado"
        return
    fi
    printf '\n# hardening:compile-flags\nadd_compile_options(\n    %s\n)\n' "$FLAGS" >> "$CMAKE"
    echo "OK. Backup en $BACKUP_DIR/"
}

cmd_build() {
    echo "Compilando..."
    mkdir -p "$REPORT_DIR"
    idf.py build 2>&1 | tee "$REPORT_DIR/build-$(date +%s).log" | grep -E "warning:|error:" | sort | uniq -c | sort -rn | head -40 || true
}

cmd_static() {
    echo "cppcheck..."
    mkdir -p "$REPORT_DIR"
    command -v cppcheck >/dev/null 2>&1 || { echo "falta cppcheck: apk add cppcheck"; exit 1; }
    cppcheck --enable=warning,style,performance,portability --inline-suppr --suppress=missingIncludeSystem --template='{file}:{line}: [{severity}] {message}' "$ROOT/main/" 2>&1 | tee "$REPORT_DIR/cppcheck-$(date +%s).log" || true
}

case "${1:-help}" in
    flags)  cmd_flags ;;
    build)  cmd_build ;;
    static) cmd_static ;;
    *)      echo "Uso: $0 [flags|build|static]" ;;
esac
