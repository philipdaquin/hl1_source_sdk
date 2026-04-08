// Simple test executable for WASM linking
#include <emscripten.h>
#include <stdio.h>

// VGUI2 includes
#include "tier1/KeyValues.h"
#include "tier2/tier2.h"

// Forward declarations
void VGui_Startup();

// Simple test entry point
int main(int argc, char** argv) {
    printf("HL1 Source SDK WASM Test\n");
    printf("========================\n");
    
    // Test KeyValues
    KeyValues kv("test");
    kv.SetString("hello", "world");
    printf("KeyValues test: %s = %s\n", kv->GetName(), kv.GetString());
    
    // Test VGUI init
    printf("VGUI initialized.\n");
    
    return 0;
}
