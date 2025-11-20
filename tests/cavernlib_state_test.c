#include <config.h>

#include "cavern_lib.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir_p(path) _mkdir(path)
#define access_exists(path) (_access(path, 0) == 0)
#define CHDIR _chdir
#define GETCWD _getcwd
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define mkdir_p(path) mkdir((path), 0777)
#define access_exists(path) (access((path), F_OK) == 0)
#define CHDIR chdir
#define GETCWD getcwd
#endif

#ifndef TEST_SRCDIR
#define TEST_SRCDIR "."
#endif

#ifndef SURVEX_LIBDIR
#define SURVEX_LIBDIR "."
#endif

#ifndef TEST_OUTPUT_DIR
#define TEST_OUTPUT_DIR "./cavernlib_state_out"
#endif

#ifndef RUNS_PER_TEST
#define RUNS_PER_TEST 5
#endif

#ifndef MAX_TESTS
#define MAX_TESTS 50
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static const char *test_cases[] = {
    "singlefix", "singlereffix", "oneleg", "midpoint", "lollipop",
    "fixedlollipop", "cross", "firststn", "deltastar", "deltastar2",
    "deltastarhanging", "bug3", "calibrate_tape", "nosurvey2", "cartesian",
    "cartesian2", "lengthunits", "angleunits", "cmd_alias", "cmd_alias_bad",
    "cmd_case", "cmd_case_bad", "cmd_copyright", "cmd_copyright_bad",
    "cmd_fix", "cmd_fix2", "cmd_fix_bad", "cmd_fix_bad2", "cmd_solve",
    "cmd_entrance", "cmd_entrance_bad", "cmd_ref", "cmd_ref_bad", "cmd_sd",
    "cmd_sd_bad", "cmd_set", "cmd_set_bad", "cmd_set_dot_in_name",
    "cmd_truncate", "cmd_truncate_bad", "beginroot", "revcomplist",
    "break_replace_pfx", "bug0", "bug1", "bug2", "bug4", "bug5", "equate_bug",
    "expobug", "require", "export", "export2", "includecomment", "self_loop",
    "self_eq_loop", "reenterwarn", "cmd_default", "cmd_default_bad",
    "cmd_prefix", "cmd_prefix_bad", "cmd_begin_bad", "cmd_equate_bad",
    "cmd_export_bad", "singlefixerr", "singlereffixerr", "begin_no_end",
    "end_no_begin", "end_no_begin_nest", "require_fail", "exporterr1",
    "exporterr2", "exporterr3", "exporterr4", "exporterr5", "exporterr1b",
    "exporterr2b", "exporterr3b", "exporterr6", "exporterr6b",
    "hanging_cpt", "badinc", "badinc2", "badinc3", "badinc4", "badinc5.mak",
    "nonexistent_file", "ONELEG", "stnsurvey1", "stnsurvey2",
    "tapelessthandepth", "longname", "chinabug", "chinabug2", "multinormal",
    "multinormignall", "multidiving", "multicylpolar", "multicartesian",
    "multinosurv", "multinormalbad", "multibug", "cmd_team", "cmd_team_bad",
    "cmd_title", "cmd_titlebad", "cmd_dummy", "cmd_infer", "cmd_date",
    "cmd_datebad", "cmd_datebad2", "cartes", "diving", "cylpolar", "normal",
    "normal_bad", "normignall", "nosurv", "cmd_flags", "bad_cmd_flags",
    "plumb", "unusedstation", "exportnakedbegin", "oldestyle", "bugdz",
    "baddatacylpolar", "baddatanosurv", "badnewline", "badquantities",
    "imgoffbyone", "infereqtopofil", "3sdfixbug", "omitclino", "back",
    "back2", "bad_back", "notentranceorexport", "inferunknown", "inferexports",
    "bad_units_factor", "bad_units_qlist", "percent_gradient", "dotinsurvey",
    "leandroclino", "lowsd", "revdir", "gettokennullderef", "nosurveyhanging",
    "nosurveyhanging2", "cmd_solve_nothing", "cmd_solve_nothing_implicit",
    "cmd_cartesian", "cmd_cartesian_bad", "cmd_calibrate", "cmd_declination",
    "cmd_declination_auto", "cmd_declination_auto_bad", "cmd_declination_conv",
    "cmd_declination_conv_proj_bug", "lech", "level", "2fixbug", "dot17",
    "3dcorner", "unconnected-bug", "backread.dat", "corrections.dat",
    "depthguage.dat", "flags.dat", "karstcompat.dat", "lrud.dat",
    "nomeasure.dat", "noteam.dat", "badmak.mak", "fixfeet.mak", "utm.mak",
    "clptest.dat", "clptest.clp", "walls.srv", "badopts.srv", "wallsbaddatum.wpj",
    "wallsdecl.wpj", "passage", "hanging_lrud", "equatenosuchstn", "surveytypo",
    "skipafterbadomit", "passagebad", "badreadingdotplus", "badcalibrate",
    "calibrate_clino", "badunits", "badbegin", "anonstn", "anonstnbad",
    "anonstnrev", "doubleinc", "reenterlots", "cs", "csbad", "csbadsdfix",
    "csfeet", "cslonglat", "omitfixaroundsolve", "repeatreading", "mixedeols",
    "utf8bom", "nonewlineateof", "suspectreadings", "cmd_data_default",
    "cmd_data_ignore", "quadrant_bearing", "bad_quadrant_bearing", "samename",
    "tabinhighlight", "legacytokens", "component_count_bug",
    "component_count_bug2", "3dexport", "dxffullcoords", "dxfsurfequate",
    "gpxexport", "hpglexport", "jsonexport", "kmlexport", "pltexport",
    "svgexport", NULL
};

static void
set_env_default(const char *key, const char *val)
{
#ifdef _WIN32
    _putenv_s(key, val);
#else
    setenv(key, val, 1);
#endif
}

static bool
ensure_directory(const char *path)
{
    if (mkdir_p(path) == 0) {
	return true;
    }
    if (errno == EEXIST) return true;
    perror("mkdir");
    return false;
}

static bool
read_first_line(const char *path, char *buf, size_t buf_len)
{
    FILE *fp = fopen(path, "r");
    if (!fp) return false;
    bool ok = fgets(buf, (int)buf_len, fp) != NULL;
    fclose(fp);
    return ok;
}

static bool
line_has_pos_fail(const char *line)
{
    if (!line) return false;
    const char *pos = strstr(line, "pos=");
    if (!pos) return false;
    pos += 4;
    char token[32];
    size_t i = 0;
    while (pos[i] && !isspace((unsigned char)pos[i]) &&
	   pos[i] != '/' && pos[i] != ';' && i + 1 < sizeof(token)) {
	token[i] = pos[i];
	++i;
    }
    token[i] = '\0';
    return strcmp(token, "fail") == 0;
}

static bool
should_skip_test(const char *name)
{
    if (strcmp(name, "ONELEG") == 0) {
	char tmp[PATH_MAX];
	snprintf(tmp, sizeof(tmp), "%s/ONELEG.SVX", TEST_SRCDIR);
	if (access_exists(tmp)) {
	    return true;
	}
    }
    if (strcmp(name, "cmd_fix") == 0) {
	return true;
    }
    return false;
}

static bool
should_expect_failure(const char *name, const char *input_path)
{
    if (strncmp(name, "nonexistent_file", 16) == 0) return true;
    if (strcmp(name, "ONELEG") == 0) return true;
    if (strcmp(name, "wallsbaddatum.wpj") == 0) return true;
    char line[1024];
    if (access_exists(input_path) && read_first_line(input_path, line, sizeof(line))) {
	if (line_has_pos_fail(line)) return true;
    }
    return false;
}

static void
build_relative_input_path(const char *name, char *buf, size_t buf_len)
{
    if (strchr(name, '.')) {
	snprintf(buf, buf_len, "./%s", name);
    } else {
	snprintf(buf, buf_len, "./%s.svx", name);
    }
}

static void
build_absolute_input_path(const char *name, char *buf, size_t buf_len)
{
    if (strchr(name, '.')) {
	snprintf(buf, buf_len, "%s/%s", TEST_SRCDIR, name);
    } else {
	snprintf(buf, buf_len, "%s/%s.svx", TEST_SRCDIR, name);
    }
}

static void
remove_previous_outputs(const char *base)
{
    const char *exts[] = {".3d", ".err", ".log", ".msg", NULL};
    char path[PATH_MAX];
    for (const char **ext = exts; *ext; ++ext) {
	snprintf(path, sizeof(path), "%s%s", base, *ext);
	remove(path);
    }
}

static bool
check_output_created(const char *base)
{
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s.3d", base);
    return access_exists(path);
}

static void
default_output_components(const char *abs_path, char *dir, size_t dir_len,
			  char *base, size_t base_len)
{
    const char *leaf = abs_path;
    const char *slash = strrchr(abs_path, '/');
#ifdef _WIN32
    const char *bslash = strrchr(abs_path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
    const char *colon = strrchr(abs_path, ':');
    if (!slash || (colon && colon > slash)) slash = colon;
#endif
    if (slash) {
	size_t len = (size_t)(slash - abs_path);
	if (len >= dir_len) len = dir_len - 1;
	memcpy(dir, abs_path, len);
	dir[len] = '\0';
	leaf = slash + 1;
    } else {
	snprintf(dir, dir_len, ".");
    }

    const char *dot = strrchr(leaf, '.');
    if (dot) {
	size_t len = (size_t)(dot - leaf);
	if (len >= base_len) len = base_len - 1;
	memcpy(base, leaf, len);
	base[len] = '\0';
    } else {
	snprintf(base, base_len, "%s", leaf);
    }
}

static bool
default_output_exists(const char *abs_path)
{
    char dir[PATH_MAX];
    char base[PATH_MAX];
    default_output_components(abs_path, dir, sizeof(dir), base, sizeof(base));
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s.3d", dir, base);
    return access_exists(path);
}

static void
remove_default_outputs(const char *abs_path)
{
    char dir[PATH_MAX];
    char base[PATH_MAX];
    default_output_components(abs_path, dir, sizeof(dir), base, sizeof(base));
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s.3d", dir, base);
    remove(path);
    snprintf(path, sizeof(path), "%s/%s.err", dir, base);
    remove(path);
}

int
main(void)
{
    set_env_default("SURVEXLIB", SURVEX_LIBDIR);
    set_env_default("LC_ALL", "C");
    set_env_default("SURVEXLANG", "en");
    set_env_default("LSAN_OPTIONS", "leak_check_at_exit=0");
    set_env_default("SOURCE_DATE_EPOCH", "1");

    if (!ensure_directory(TEST_OUTPUT_DIR)) {
	return EXIT_FAILURE;
    }

    char base_out[PATH_MAX];
    snprintf(base_out, sizeof(base_out), "%s/tmp", TEST_OUTPUT_DIR);

    char orig_cwd[PATH_MAX];
    if (!GETCWD(orig_cwd, sizeof(orig_cwd))) {
	perror("getcwd");
	return EXIT_FAILURE;
    }

    for (int run = 0; run < RUNS_PER_TEST; ++run) {
	for (size_t idx = 0; idx < MAX_TESTS && test_cases[idx]; ++idx) {
	    const char *name = test_cases[idx];
	    if (should_skip_test(name)) {
		continue;
	    }
	    char abs_input[PATH_MAX];
	    build_absolute_input_path(name, abs_input, sizeof(abs_input));
	    bool expect_failure = should_expect_failure(name, abs_input);
	    if (!expect_failure && !access_exists(abs_input)) {
		fprintf(stderr, "Missing input file for %s: %s\n", name, abs_input);
		return EXIT_FAILURE;
	    }

	    remove_previous_outputs(base_out);
	    remove_default_outputs(abs_input);

	    if (CHDIR(TEST_SRCDIR) != 0) {
		perror("chdir");
		return EXIT_FAILURE;
	    }

	    char rel_input[PATH_MAX];
	    build_relative_input_path(name, rel_input, sizeof(rel_input));

	    char *argv_run[] = {
		"cavernlib_state_test",
		"--output",
		base_out,
		rel_input,
		NULL
	    };

	    int rc = cavern_run(4, argv_run);

	    if (CHDIR(orig_cwd) != 0) {
		perror("chdir restore");
		return EXIT_FAILURE;
	    }

	    if (expect_failure) {
		if (rc == EXIT_SUCCESS) {
		    fprintf(stderr, "Expected failure for %s but succeeded\n", name);
		    return EXIT_FAILURE;
		}
	    } else {
		if (rc != EXIT_SUCCESS) {
		    fprintf(stderr, "Expected success for %s (run %d) but rc=%d\n",
			    name, run, rc);
		    return EXIT_FAILURE;
		}
		bool produced = check_output_created(base_out);
		if (!produced && default_output_exists(abs_input)) {
		    produced = true;
		}
		if (!produced) {
		    fprintf(stderr, "Run %d for %s produced no .3d output\n",
			    run, name);
		    return EXIT_FAILURE;
		}
	    }

	    remove_default_outputs(abs_input);
	}
    }

    return EXIT_SUCCESS;
}
