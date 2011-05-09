#include "check_spdy_frame.h"
#include "../src/spdy_frame.h"
#include "../src/spdy_control_frame.h"
#include "../src/spdy_zlib.h"
#include "../src/spdy_error.h"

#include "testdata.h"

START_TEST (test_spdy_frame_parse_header)
{
	spdy_frame frame;
	spdy_frame_parse_header(&frame, test_control_syn_stream_frame, 8);
	fail_unless(frame.type == SPDY_CONTROL_FRAME, "Frame type detection failed.");
	spdy_control_frame *ctrl = (spdy_control_frame*)frame.frame;
	fail_unless(ctrl->version == 2, "Creation of control frame failed.");
}
END_TEST

START_TEST (test_spdy_frame_parse)
{
	int ret;
	spdy_zlib_context zlib_ctx;
	spdy_zlib_inflate_init(&zlib_ctx);
	spdy_frame frame;
	size_t data_used=0;
	ret = spdy_frame_parse(&frame, test_control_syn_stream_frame, 296, &data_used, &zlib_ctx);
	fail_unless(ret == SPDY_ERROR_NONE, "spdy_frame_parse failed.");
}
END_TEST

Suite * spdy_frame_suite()
{
	Suite *s = suite_create("spdy_frame");
	TCase *tc_core = tcase_create("spdy_frame_parse_header");
	tcase_add_test(tc_core, test_spdy_frame_parse_header);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_frame_parse");
	tcase_add_test(tc_core, test_spdy_frame_parse);
	suite_add_tcase (s, tc_core);
	return s;
}

