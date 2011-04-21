#include "check_spdy_frame.h"
#include "../src/spdy_frame.h"

#include "testdata.h"

START_TEST (test_spdy_frame_parse_header)
{
	spdy_frame frame;
	spdy_frame_parse_header(&frame, test_frame_header);
	fail_unless(frame.type == SPDY_CONTROL_FRAME, "Frame type detection failed.");
}
END_TEST

Suite * spdy_frame_suite()
{
	Suite *s = suite_create("spdy_frame");
	TCase *tc_core = tcase_create("spdy_frame_parse_header");
	tcase_add_test(tc_core, test_spdy_frame_parse_header);
	suite_add_tcase (s, tc_core);

	return s;
}

