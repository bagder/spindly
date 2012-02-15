#include "spdy_setup.h"
#include "check_spdy_nv_block.h"
#include "spdy_nv_block.h"
#include "spdy_error.h"
#include <string.h>

#include "testdata.h"

spdy_nv_pair test_nv_pairs[] = {
		{"accept", 1, "application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5"},
		{"accept-charset", 1, "ISO-8859-1,utf-8;q=0.7,*;q=0.3"},
		{"accept-encoding", 1, "gzip,deflate,sdch"},
		{"accept-language", 1, "en-US,en;q=0.8"},
		{"cache-control", 1, "max-age=0"},
		{"host", 1, "localhost:3800"},
		{"method", 1, "GET"},
		{"scheme", 1, "http"},
		{"url", 1, "/"},
		{"user-agent", 1, "Mozilla/5.0 (X11; Linux i686) AppleWebKit/534.24 (KHTML, like Gecko) Chrome/11.0.696.16 Safari/534.24"},
		{"version", 1, "HTTP/1.1"}
	};

START_TEST (test_spdy_nv_block_parse)
{
	spdy_nv_block nv_block;
        int ret;
	int i;

	spdy_nv_block_init(&nv_block);
	/* Test with insufficient data. */
	/* TODO: Check pairs_parsed count etc. */
	ret = spdy_nv_block_parse(&nv_block, test_nv_block, 400);
	fail_unless(ret == SPDY_ERROR_INSUFFICIENT_DATA,
			"Couldn't determine that data was insufficient.");

        spdy_nv_block_destroy(&nv_block);
	spdy_nv_block_init(&nv_block);
	/* Test with right amount of data. */
	ret = spdy_nv_block_parse(&nv_block, test_nv_block,436);
	/* Check return value */
	fail_unless(ret == 0, "spdy_nv_block_parse failed.");
	/* Check pair number */
	fail_unless(nv_block.count == 11, "Number of pairs is wrong.");
	/* Check names and values */
	for(i=0;i< nv_block.count;i++) {
		fail_unless(strcmp(nv_block.pairs[i].name, test_nv_pairs[i].name) == 0, "Valuename is wrong.");
		fail_unless(strcmp(nv_block.pairs[i].values, test_nv_pairs[i].values) == 0, "Value is wrong.");
	}
        spdy_nv_block_destroy(&nv_block);
}
END_TEST

START_TEST (test_spdy_nv_block_pack)
{

	spdy_nv_block nv_block;
	char *dest;
	size_t dest_size;
	int ret;
	nv_block.count = 3;
	nv_block.pairs = test_nv_pairs;
	ret = spdy_nv_block_pack(&dest, &dest_size, &nv_block);
	fail_unless(ret == 0, "spdy_nv_block_pack failed.");
        free(dest);
}
END_TEST

START_TEST (test_spdy_nv_block_parse_pack)
{
	spdy_nv_block nv_block;
	char *dest;
	size_t dest_size;
	int ret;
        spdy_nv_block_init(&nv_block);
	ret = spdy_nv_block_parse(&nv_block, test_nv_block,436);
	fail_unless(ret == 0, "spdy_nv_block_parse failed.");
	ret = spdy_nv_block_pack(&dest, &dest_size, &nv_block);
	fail_unless(ret == 0, "spdy_nv_block_pack failed.");
	fail_unless(memcmp(dest, test_nv_block, dest_size)==0, "Packed data differs from testdata.");
        free(dest);
        spdy_nv_block_destroy(&nv_block);
}
END_TEST

Suite * spdy_nv_block_suite()
{
	Suite *s = suite_create("spdy_nv_block");
	TCase *tc_core = tcase_create("spdy_nv_block_parse");
	tcase_add_test(tc_core, test_spdy_nv_block_parse);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_nv_block_pack");
	tcase_add_test(tc_core, test_spdy_nv_block_pack);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_nv_block_parse_pack");
	tcase_add_test(tc_core, test_spdy_nv_block_parse_pack);
	suite_add_tcase (s, tc_core);
	return s;
}

