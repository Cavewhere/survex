/* svx_exit.h
 * Interposable exit handling so embedded builds can intercept `exit()`.
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef SVX_EXIT_H
#define SVX_EXIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*svx_exit_handler)(int code, void *ctx);

typedef struct {
    svx_exit_handler handler;
    void *ctx;
} svx_exit_state;

svx_exit_state svx_enter_exit_handler(svx_exit_handler handler, void *ctx);
void svx_leave_exit_handler(svx_exit_state state);
void svx_exit(int code);

#ifdef __cplusplus
}
#endif

#endif /* SVX_EXIT_H */
