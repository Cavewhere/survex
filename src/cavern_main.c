#include <config.h>

#include "cavern_lib.h"

int
main(int argc, char **argv)
{
    int rc = cavern_run(argc, argv);
    cavern_pause_if_requested();
    return rc;
}
