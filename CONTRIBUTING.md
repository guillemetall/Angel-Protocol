# Contributing to Angel Protocol

Thanks for your interest. This project is an offline mesh firmware for ESP32-C6, built for emergency scenarios where infrastructure has failed. Contributions that improve robustness, range, or battery life are especially welcome.

## Before you start

Read these files to understand the project:
- `README.md` — what it does, how to flash, hardware
- `HARDENING.md` — invariants, known bugs, size limits
- `docs/manifest.json` — web installer configuration

## Development setup

Requirements:
- ESP-IDF v5.3
- Seeed Studio XIAO ESP32-C6 (or any ESP32-C6 board)
- USB-C cable

Build:
    idf.py set-target esp32c6
    idf.py build
    idf.py -p /dev/ttyUSB0 flash monitor

Strict compile flags are already enabled (see root CMakeLists.txt). Do not disable them to make code compile — fix the warning instead.

## Code rules

1. Every memcpy(dst, src, n) with variable n needs a guard:

       if (n > sizeof(dst)) return false;

   This is non-negotiable. See HARDENING.md for the size limits.

2. No VLAs (uint8_t buf[len]). Use fixed-size buffers or static.

3. No large stack buffers (>512 B). Move them to file-scope static or .bss.

4. Validate every network input before using it. Assume frames arrive malformed.

5. No strcpy, sprintf, or unsized strcat. Use bounded variants.

## Pull requests

- One bug or feature per PR.
- Reference the issue number if there is one.
- Include a short explanation of the why, not just the what.
- CI must be green (build + no new warnings).
- Update HARDENING.md if you introduce new invariants.

## Reporting bugs

Open an issue with:
- Board model (e.g. XIAO ESP32-C6)
- IDF version (idf.py --version)
- Steps to reproduce
- Expected vs actual behavior
- Serial log if available

Security issues: please do not open public issues. Contact the maintainer directly.
