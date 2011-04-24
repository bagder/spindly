#include "check_spdy_zlib.h"
#include "../src/spdy_zlib.h"
#include "../src/spdy_nv_block.h"

#include "testdata.h"

START_TEST (test_spdy_zlib_deflate)
{
	char *dest;
	size_t dest_size;
	int ret = spdy_zlib_deflate(test_nv_block, 436, &dest, &dest_size);
	fail_unless(ret == 0, "spdy_zlib_deflate failed.");
}
END_TEST

START_TEST (test_spdy_zlib_inflate)
{
	char *dest;
	size_t dest_size;
	spdy_zlib_context ctx;
	int ret = spdy_zlib_inflate_init(&ctx);
	fail_unless(ret == 0, "spdy_zlib_inflate_init failed.");
	ret = spdy_zlib_inflate(&ctx, test_control_syn_stream_frame+18, 280, &dest, &dest_size);
	fail_unless(memcmp(dest, test_nv_block, 436)==0, "Difference to testdata.");
	spdy_nv_block nv_block;
	ret = spdy_nv_block_parse(&nv_block, dest,dest_size);
	fail_unless(ret == 0, "spdy_nv_block_parse failed.");
	free(dest);
}
END_TEST

START_TEST (test_spdy_zlib_deflate_inflate)
{
	char *deflate, *inflate;
	size_t deflate_size, inflate_size;
	spdy_zlib_context ctx;
	int ret;
	ret = spdy_zlib_deflate(test_nv_block, 436, &deflate, &deflate_size);
	fail_unless(ret == 0, "spdy_zlib_deflate failed.");

	ret = spdy_zlib_inflate_init(&ctx);
	fail_unless(ret == 0, "spdy_zlib_inflate_init failed.");
	ret = spdy_zlib_inflate(&ctx, deflate, deflate_size, &inflate, &inflate_size);
	fail_unless(ret == 0, "spdy_zlib_inflate failed.");
	fail_unless(memcmp(inflate, test_nv_block, inflate_size)==0, "Data changed.");

	free(deflate);
	free(inflate);
}
END_TEST

Suite * spdy_zlib_suite()
{
	Suite *s = suite_create("spdy_zlib");
	TCase *tc_core = tcase_create("spdy_zlib_inflate");
	tcase_add_test(tc_core, test_spdy_zlib_inflate);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_zlib_deflate");
	tcase_add_test(tc_core, test_spdy_zlib_deflate);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_zlib_deflate_inflate");
	tcase_add_test(tc_core, test_spdy_zlib_deflate_inflate);
	suite_add_tcase (s, tc_core);
	return s;
}

