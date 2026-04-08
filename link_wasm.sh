#!/bin/bash
# Link script for HL1 Source SDK WASM build
# Usage: ./link_wasm.sh

set -e

BUILD_DIR="/src/build-wasm"
OUTPUT_NAME="game"

echo "Linking HL1 Source SDK WASM..."

docker run --rm \
    -v /Users/philipdaquin/Desktop/cs1.6/cs-client/hl1_source_sdk:/src \
    -u $(id -u):$(id -g) \
    emscripten/emsdk \
    emcc \
    /src/src/test_main.c \
    ${BUILD_DIR}/tier1/libtier1.a \
    ${BUILD_DIR}/tier2/libtier2.a \
    ${BUILD_DIR}/vgui2/vgui_controls/libvgui_controls.a \
    -o ${BUILD_DIR}/${OUTPUT_NAME}.html \
    -sWASM=1 \
    -sALLOW_MEMORY_GROWTH=1 \
    -sINITIAL_MEMORY=67108864 \
    -sUSE_SDL=2 \
    -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
    -sEXPORTED_FUNCTIONS="['_main','_printf']" \
    --shell-file /emsdk/upstream/emscripten/cache/sysroot/wasm_js_without_shipping.html \
    ${1:-}  # Optional extra args

echo "Link complete!"
echo "Output files:"
ls -la ${BUILD_DIR}/${OUTPUT_NAME}.*
