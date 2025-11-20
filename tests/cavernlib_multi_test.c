#include <config.h>

#include "cavern_lib.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define MKDIR(path) _mkdir(path)
#define ACCESS(path) _access(path, 0)
#define DIR_SEP '\\'
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0777)
#define ACCESS(path) access((path), F_OK)
#define DIR_SEP '/'
#endif

static bool
ensure_dir(const char *path)
{
    errno = 0;
    if (MKDIR(path) == 0) {
	return true;
    }
    if (errno == EEXIST) return true;
    perror("mkdir");
    return false;
}

static char *
leaf_without_ext(const char *path)
{
    const char *leaf = path;
    const char *slash = strrchr(path, '/');
    if (slash && slash >= leaf) leaf = slash + 1;
#ifdef _WIN32
    const char *bslash = strrchr(path, '\\');
    if (bslash && bslash >= leaf) leaf = bslash + 1;
    const char *colon = strrchr(path, ':');
    if (colon && colon >= leaf) leaf = colon + 1;
#endif
    const char *dot = strrchr(leaf, '.');
    size_t len = dot ? (size_t)(dot - leaf) : strlen(leaf);
    char *res = malloc(len + 1);
    if (!res) {
	perror("malloc");
	exit(EXIT_FAILURE);
    }
    memcpy(res, leaf, len);
    res[len] = '\0';
    return res;
}

static char *
make_path(const char *dir, const char *leaf, const char *ext)
{
    size_t dir_len = strlen(dir);
    bool need_sep = dir_len > 0 && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\';
    size_t ext_len = ext ? strlen(ext) : 0;
    size_t len = dir_len + (need_sep ? 1 : 0) + strlen(leaf) + ext_len + 1;
    char *path = malloc(len);
    if (!path) {
	perror("malloc");
	exit(EXIT_FAILURE);
    }
    snprintf(path, len, "%s%s%s%s",
	     dir,
	     need_sep ? (DIR_SEP == '\\' ? "\\" : "/") : "",
	     leaf,
	     ext ? ext : "");
    return path;
}

static bool
file_exists(const char *path)
{
    return ACCESS(path) == 0;
}

static void
remove_if_exists(const char *path)
{
    if (file_exists(path)) remove(path);
}

static int
run_cavern(const char *input, const char *outdir)
{
    char *argv_run[] = {
	"cavernlib_multi_test",
	"--output",
	(char *)outdir,
	(char *)input,
	NULL
    };
    return cavern_run(4, argv_run);
}

int
main(int argc, char **argv)
{
    if (argc != 3) {
	fprintf(stderr, "Usage: %s <input.svx> <output-dir>\n", argv[0]);
	return EXIT_FAILURE;
    }
    const char *input = argv[1];
    const char *outdir = argv[2];

    if (!ensure_dir(outdir)) {
	fprintf(stderr, "Failed to create output dir %s\n", outdir);
	return EXIT_FAILURE;
    }

    char *leaf = leaf_without_ext(input);
    char *out3d = make_path(outdir, leaf, ".3d");
    char *outerr = make_path(outdir, leaf, ".err");

    remove_if_exists(out3d);
    remove_if_exists(outerr);

    int rc1 = run_cavern(input, outdir);
    if (rc1 != EXIT_SUCCESS || !file_exists(out3d)) {
	fprintf(stderr, "First cavern_run failed (rc=%d)\n", rc1);
	free(leaf);
	free(out3d);
	free(outerr);
	return EXIT_FAILURE;
    }

    int rc2 = run_cavern(input, outdir);
    if (rc2 != EXIT_SUCCESS || !file_exists(out3d)) {
	fprintf(stderr, "Second cavern_run failed (rc=%d)\n", rc2);
	free(leaf);
	free(out3d);
	free(outerr);
	return EXIT_FAILURE;
    }

    free(leaf);
    free(out3d);
    free(outerr);
    return EXIT_SUCCESS;
}
