#include "spdy_setup.h"
#include "check_spdy_syn_stream.h"
#include "spdy_syn_stream.h"

#include "testdata.h"

START_TEST (test_spdy_syn_stream_parse_header)
{
	int ret;
	spdy_syn_stream syn_stream;
	spdy_data data;
	ret = spdy_syn_stream_parse_header(&syn_stream,
			spdy_data_use(&data, test_control_syn_stream_frame+8, 288));
	fail_unless(ret == 0, "spdy_syn_stream_parse failed.");
	fail_unless(syn_stream.stream_id == 1, "Stream ID parsing failed.");
	fail_unless(syn_stream.associated_to == 0, "Associated to parsing failed.");
	fail_unless(syn_stream.priority == 0, "Priority parsing failed.");
}
END_TEST

START_TEST (test_spdy_syn_stream_parse)
{
#if 0
	int ret;
	spdy_syn_stream syn_stream;
	spdy_data data;
        struct hash *hash = NULL;
        spdy_data_use(&data, test_control_syn_stream_frame+8, 288);
	ret = spdy_syn_stream_parse(&syn_stream, hash, &data, 288);
	fail_unless(ret == 0, "spdy_syn_stream_parse failed.");
        spdy_syn_stream_destroy(&syn_stream);
#endif
}
END_TEST

Suite * spdy_syn_stream_suite()
{
	Suite *s = suite_create("spdy_syn_stream");
	TCase *tc_core = tcase_create("spdy_syn_stream_parse_header");
	tcase_add_test(tc_core, test_spdy_syn_stream_parse_header);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_syn_stream_parse");
	tcase_add_test(tc_core, test_spdy_syn_stream_parse);
	suite_add_tcase (s, tc_core);

	return s;
}

