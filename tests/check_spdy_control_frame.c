#include "check_spdy_control_frame.h"
#include "../src/spdy_control_frame.h"

#include "testdata.h"

START_TEST (test_spdy_control_frame_parse_header)
{
	spdy_control_frame frame;
	spdy_control_frame_parse_header(&frame, test_frame_header);
	fail_unless(frame.version == 2, "Version parsing failed.");
	fail_unless(frame.type == 1, "Type parsing failed.");
	fail_unless(frame.flags == 1, "Flag parsing failed.");
	fail_unless(frame.length == 288, "Length parsing failed.");
}
END_TEST

START_TEST (test_spdy_control_frame_pack_header)
{
	spdy_control_frame frame = {
		.version = 2,
		.type = 1,
		.flags = 1,
		.length = 288
	};
	char *out=NULL;
	int ret = spdy_control_frame_pack_header(&out, &frame);
	fail_unless(ret == 0, "spdy_control_frame_pack_header failed.");
	fail_unless(memcmp(out, test_frame_header, 8) == 0, "Packed data is invalid.");
}
END_TEST

Suite * spdy_control_frame_suite()
{
	Suite *s = suite_create("spdy_control_frame");
	TCase *tc_core = tcase_create("spdy_control_frame_parse_header");
	tcase_add_test(tc_core, test_spdy_control_frame_parse_header);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_control_frame_pack_header");
	tcase_add_test(tc_core, test_spdy_control_frame_pack_header);
	suite_add_tcase(s, tc_core);
	return s;
}

