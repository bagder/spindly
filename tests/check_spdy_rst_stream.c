#include "check_spdy_rst_stream.h"
#include "spdy_rst_stream.h"

#include "testdata.h"

START_TEST(test_spdy_rst_stream_parse)
{
	int ret;
	spdy_rst_stream rst_stream;
	ret = spdy_rst_stream_parse(&rst_stream, test_control_rst_stream_frame+8, 8);
	fail_unless(ret == 0, "spdy_rst_stream_parse failed.");
	fail_unless(rst_stream.stream_id == 1, "Stream ID parsing failed.");
	fail_unless(rst_stream.status_code == SPDY_PROTOCOL_ERROR, "Status code parsing failed.");
}
END_TEST

Suite * spdy_rst_stream_suite()
{
	Suite *s = suite_create("spdy_rst_stream");
	TCase *tc_core = tcase_create("spdy_rst_stream_parse");
	tcase_add_test(tc_core, test_spdy_rst_stream_parse);
	suite_add_tcase(s, tc_core);

	return s;
}

