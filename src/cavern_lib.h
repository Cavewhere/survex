/* cavern_lib.h
 * Public API for embedding cavern as a library.
 */

#ifndef CAVERN_LIB_H
#define CAVERN_LIB_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int cavern_run(int argc, char **argv);
void cavern_pause_if_requested(void);

#ifdef __cplusplus
}
#endif

#endif /* CAVERN_LIB_H */
