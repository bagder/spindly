#include "check_spdy_syn_reply.h"
#include "../src/spdy_syn_reply.h"

#include "testdata.h"

START_TEST (test_spdy_syn_reply_parse_header)
{
	int ret;
	spdy_syn_reply syn_reply;
	ret = spdy_syn_reply_parse_header(&syn_reply, test_control_syn_reply_frame+8, 55);
	fail_unless(ret == 0, "spdy_syn_reply_parse failed.");
	fail_unless(syn_reply.stream_id == 1, "Stream ID parsing failed.");
}
END_TEST

START_TEST (test_spdy_syn_reply_parse)
{
	int ret;
	spdy_zlib_context zlib_ctx;
	spdy_syn_reply syn_reply;
	size_t data_used=0;
	ret = spdy_zlib_inflate_init(&zlib_ctx);
	fail_unless(ret == 0, "spdy_zlib_inflate_init failed.");
	ret = spdy_syn_reply_parse(&syn_reply, test_control_syn_reply_frame+8, 55, &data_used, &zlib_ctx);
	fail_unless(ret == 0, "spdy_syn_reply_parse failed.");
}
END_TEST

Suite * spdy_syn_reply_suite()
{
	Suite *s = suite_create("spdy_syn_reply");
	TCase *tc_core = tcase_create("spdy_syn_reply_parse_header");
	tcase_add_test(tc_core, test_spdy_syn_reply_parse_header);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_syn_reply_parse");
	tcase_add_test(tc_core, test_spdy_syn_reply_parse);
	suite_add_tcase (s, tc_core);

	return s;
}

