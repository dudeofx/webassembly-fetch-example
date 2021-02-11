emcc fetch_test.c -s WASM=1 -s FETCH=1 -s RESERVED_FUNCTION_POINTERS=20 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap", "ccall", "addFunction", "UTF8ToString"]' -o fetch_test.js



