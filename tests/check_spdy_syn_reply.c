#include "spdy_setup.h"
#include "check_spdy_syn_reply.h"
#include "spdy_syn_reply.h"

#include "testdata.h"

START_TEST (test_spdy_syn_reply_parse_header)
{
	int ret;
	spdy_syn_reply syn_reply;
	spdy_data data;
	ret = spdy_syn_reply_parse_header(&syn_reply,
			spdy_data_use(&data, test_control_syn_reply_frame+8, 55));
	fail_unless(ret == 0, "spdy_syn_reply_parse failed.");
	fail_unless(syn_reply.stream_id == 1, "Stream ID parsing failed.");
}
END_TEST

START_TEST (test_spdy_syn_reply_parse)
{
#if 0
	int ret;
	spdy_syn_reply syn_reply;
	spdy_data data;
        struct hash *hash = NULL;

        spdy_data_use(&data, test_control_syn_reply_frame+8, 55);
	ret = spdy_syn_reply_parse(&syn_reply, hash, &data, 55);
	fail_unless(ret == 0, "spdy_syn_reply_parse failed.");
        spdy_syn_reply_destroy(&syn_reply);
#endif
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

