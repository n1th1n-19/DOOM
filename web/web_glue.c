// Emscripten-only glue. Kept apart because emscripten.h redefines true/false,
// which breaks DOOM's `typedef enum {false, true} boolean`.
#include <emscripten.h>

void Web_SetMainLoop (void (*fn)(void))
{
    emscripten_set_main_loop (fn, 0, 1);   // 0 = requestAnimationFrame
}

void Web_Quit (void)
{
    emscripten_cancel_main_loop ();
    EM_ASM( if (Module.onDoomQuit) Module.onDoomQuit(); );
}

void Web_Error (const char* msg)
{
    EM_ASM({ if (Module.onDoomError) Module.onDoomError(UTF8ToString($0)); }, msg);
}
