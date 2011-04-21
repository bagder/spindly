#include "check_spdy_nv_block.h"
#include "../src/spdy_nv_block.h"
#include <string.h>

#include "testdata.h"

char *test_nv_pairs[] = 
{
	"accept", "application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5",
	"accept-charset","ISO-8859-1,utf-8;q=0.7,*;q=0.3",
	"accept-encoding", "gzip,deflate,sdch",
	"accept-language", "en-US,en;q=0.8",
	"cache-control", "max-age=0",
	"host", "localhost:3800",
	"method", "GET",
	"scheme", "http",
	"url", "/",
	"user-agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/534.24 (KHTML, like Gecko) Chrome/11.0.696.16 Safari/534.24",
	"version", "HTTP/1.1"
};
START_TEST (test_spdy_nv_block_parse)
{
	spdy_nv_block nv_block;
	int ret = spdy_nv_block_parse(&nv_block, test_nv_block);
	// Check return value
	fail_unless(ret == 0, "spdy_nv_block_parse failed.");
	// Check pair number
	fail_unless(nv_block.count == 11, "Number of pairs is wrong.");
	// Check names and values
	for(int i=0;i< nv_block.count;i++) {
		fail_unless(strcmp(nv_block.pairs[i].name, test_nv_pairs[i*2]) == 0, "Valuename is wrong.");
		fail_unless(strcmp(nv_block.pairs[i].values, test_nv_pairs[(i*2)+1]) == 0, "Value is wrong.");
	}
}
END_TEST

Suite * spdy_nv_block_suite()
{
	Suite *s = suite_create("spdy_nv_block");
	TCase *tc_core = tcase_create("spdy_nv_block_parse");
	tcase_add_test(tc_core, test_spdy_nv_block_parse);
	suite_add_tcase (s, tc_core);

	return s;
}

