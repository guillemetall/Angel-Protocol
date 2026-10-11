#!/bin/bash
cd ~/Angel-Protocol
sed -i 's/project(xiaomesh)/project(angel)/' CMakeLists.txt
sed -i 's|build/xiaomesh.bin|build/angel.bin|' .github/workflows/build.yml
sed -i 's/Xiaomesh v%s (Fase 2: BLE)/Angel Protocol v%s/' main/app_main.c
sed -i 's/Xiaomesh v5.0.0/Angel Protocol v5.0.0/' docs/manifest.json
sed -i 's|"path": "xiaomesh.bin"|"path": "angel.bin"|' docs/manifest.json
sed -i 's/Xiaomesh Firmware Installer/Angel Protocol Installer/' docs/index.html
sed -i 's/Install Xiaomesh v5.0.0/Install Angel Protocol v5.0.0/' docs/index.html
sed -i 's|Meshxiaoo-v2|Angel-Protocol|g' docs/index.html docs/README.md README.md
sed -i 's/Xiaomesh/Angel Protocol/g' docs/README.md README.md
sed -i 's/xiaomesh.bin/angel.bin/g' README.md
rm -f docs/xiaomesh.bin
git add .
git commit -m "Rename: Xiaomesh to Angel Protocol"
git push
