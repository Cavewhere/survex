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
#define mkdir_p(path) _mkdir(path)
#define access_exists(path) (_access(path, 0) == 0)
#define GETCWD _getcwd
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define mkdir_p(path) mkdir((path), 0777)
#define access_exists(path) (access(path, F_OK) == 0)
#define GETCWD getcwd
#endif

#ifndef TEST_SRCDIR
#define TEST_SRCDIR "."
#endif

#ifndef SURVEX_LIBDIR
#define SURVEX_LIBDIR "."
#endif

#ifndef SURVEX_BINDIR
#define SURVEX_BINDIR "."
#endif

#ifndef TEST_OUTPUT_DIR
#define TEST_OUTPUT_DIR "./cavernlib_validation_out"
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static const char *tests_with_pos[] = {
    "singlefix",
    "singlereffix",
    "oneleg",
    "midpoint",
    "lollipop",
    "fixedlollipop",
    "cross",
    "firststn",
    "deltastar",
    "bug3",
    "cartesian",
    "lengthunits",
    "angleunits",
    "cmd_alias",
    "cmd_case",
    NULL
};

static void
set_env(const char *key, const char *value)
{
#ifdef _WIN32
    _putenv_s(key, value);
#else
    setenv(key, value, 1);
#endif
}

static bool
ensure_directory(const char *path)
{
    if (mkdir_p(path) == 0 || errno == EEXIST) return true;
    perror("mkdir");
    return false;
}

static bool
file_exists(const char *path)
{
    return access_exists(path);
}

static bool
run_diffpos(const char *expected, const char *actual)
{
    char cmd[PATH_MAX * 2];
    snprintf(cmd, sizeof(cmd), "\"%s/diffpos\" \"%s\" \"%s\" > /dev/null",
	     SURVEX_BINDIR, expected, actual);
    int rc = system(cmd);
    return rc == 0;
}

static void
default_output_path(const char *input, const char *cwd, char *buf, size_t buf_len)
{
    const char *leaf = input;
    const char *sep = strrchr(input, '/');
#ifdef _WIN32
    const char *bsep = strrchr(input, '\\');
    if (!sep || (bsep && bsep > sep)) sep = bsep;
#endif
    if (sep) leaf = sep + 1;
    char base[PATH_MAX];
    const char *dot = strrchr(leaf, '.');
    if (dot) {
	size_t len = (size_t)(dot - leaf);
	if (len >= sizeof(base)) len = sizeof(base) - 1;
	memcpy(base, leaf, len);
	base[len] = '\0';
    } else {
	snprintf(base, sizeof(base), "%s", leaf);
    }
    snprintf(buf, buf_len, "%s/%s.3d", cwd, base);
}

int
main(void)
{
    set_env("SURVEXLIB", SURVEX_LIBDIR);
    set_env("LC_ALL", "C");
    set_env("SURVEXLANG", "en");
    set_env("LSAN_OPTIONS", "leak_check_at_exit=0");
    set_env("SOURCE_DATE_EPOCH", "1");

    if (!ensure_directory(TEST_OUTPUT_DIR)) {
	return EXIT_FAILURE;
    }

    char cwd[PATH_MAX];
    if (!GETCWD(cwd, sizeof(cwd))) {
	perror("getcwd");
	return EXIT_FAILURE;
    }

    for (size_t i = 0; tests_with_pos[i]; ++i) {
	const char *name = tests_with_pos[i];
	char input[PATH_MAX];
	snprintf(input, sizeof(input), "%s/%s.svx", TEST_SRCDIR, name);
	if (!file_exists(input)) {
	    fprintf(stderr, "Missing input %s\n", input);
	    return EXIT_FAILURE;
	}

	char out_base[PATH_MAX];
	snprintf(out_base, sizeof(out_base), "%s/%s", TEST_OUTPUT_DIR, name);
	char tmp_path[PATH_MAX];
	snprintf(tmp_path, sizeof(tmp_path), "%s.3d", out_base);
	remove(tmp_path);
	snprintf(tmp_path, sizeof(tmp_path), "%s.err", out_base);
	remove(tmp_path);

	char *argv_run[] = {
	    "cavernlib_validation_test",
	    "--output",
	    out_base,
	    input,
	    NULL
	};

	int rc = cavern_run(4, argv_run);
	if (rc != EXIT_SUCCESS) {
	    fprintf(stderr, "cavern_run failed for %s (rc=%d)\n", name, rc);
	    return EXIT_FAILURE;
	}

	char actual_3d[PATH_MAX];
	snprintf(actual_3d, sizeof(actual_3d), "%s.3d", out_base);
	const char *actual_path = actual_3d;
	if (!file_exists(actual_3d)) {
	    char fallback[PATH_MAX];
	    default_output_path(input, cwd, fallback, sizeof(fallback));
	    if (file_exists(fallback)) {
		actual_path = fallback;
	    } else {
	    fprintf(stderr, "No 3d output for %s\n", name);
	    return EXIT_FAILURE;
	}
	}
	char expected_pos[PATH_MAX];
	snprintf(expected_pos, sizeof(expected_pos), "%s/%s.pos", TEST_SRCDIR, name);

	if (!run_diffpos(expected_pos, actual_path)) {
	    fprintf(stderr, "Output mismatch for %s\n", name);
	    return EXIT_FAILURE;
	}
    }

    return EXIT_SUCCESS;
}
