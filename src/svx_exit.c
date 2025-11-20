/* svx_exit.c
 * Replace raw exit() usage with an interposable handler so we can trap it.
 */

#include "svx_exit.h"

#include <stdlib.h>

static void
default_exit_handler(int code, void *ctx)
{
    (void)ctx;
    exit(code);
}

static svx_exit_handler current_handler = default_exit_handler;
static void *current_ctx = NULL;

svx_exit_state
svx_enter_exit_handler(svx_exit_handler handler, void *ctx)
{
    svx_exit_state prev = { current_handler, current_ctx };
    current_handler = handler ? handler : default_exit_handler;
    current_ctx = ctx;
    return prev;
}

void
svx_leave_exit_handler(svx_exit_state state)
{
    current_handler = state.handler ? state.handler : default_exit_handler;
    current_ctx = state.ctx;
}

void
svx_exit(int code)
{
    current_handler(code, current_ctx);
}
