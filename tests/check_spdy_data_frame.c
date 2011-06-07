#include "check_spdy_data_frame.h"
#include "../src/spdy_data_frame.h"
#include "../src/spdy_error.h"

#include "testdata.h"

START_TEST (test_spdy_data_frame_parse_header)
{
	spdy_data_frame frame;
	spdy_data_frame_parse_header(&frame, test_data_frame_header, 8);
	fail_unless(frame.stream_id == 1, "Stream ID parsing failed.");
	fail_unless(frame.flags == 1, "Flag parsing failed.");
	fail_unless(frame.length == 0, "Length parsing failed.");
}
END_TEST

START_TEST (test_spdy_data_frame_parse)
{
	int ret;
	spdy_data_frame frame;
	spdy_data data;
	ret = spdy_data_frame_parse(
			&frame,
			spdy_data_use(&data, test_data_frame, 23));
	fail_unless(
			ret == SPDY_ERROR_NONE,
			"spdy_data_frame_parse failed.");
	fail_unless(
			frame.length == 15,
			"Length parsing failed.");
	fail_unless(
			memcmp("123456789012345", frame.data, 15) == 0,
			"Parsed data is wrong.");
}
END_TEST

START_TEST (test_spdy_data_frame_pack_header)
{
	spdy_data_frame frame = {
		.stream_id = 1,
		.flags = 1,
		.length = 0
	};
	char *out=NULL;
	int ret = spdy_data_frame_pack_header(&out, &frame);
	fail_unless(ret == 0, "spdy_data_frame_pack_header failed.");
	fail_unless(memcmp(out, test_data_frame_header, 8) == 0, "Packed data is invalid.");
}
END_TEST

START_TEST (test_spdy_data_frame_parse_pack)
{
	spdy_data_frame frame;
	char *out;
	int ret = spdy_data_frame_parse_header(&frame, test_data_frame_header, 8);
	fail_unless(ret == 0, "spdy_data_frame_parse_header failed.");
	ret = spdy_data_frame_pack_header(&out, &frame);
	fail_unless(ret == 0, "spdy_data_frame_pack_header failed.");
	fail_unless(memcmp(out, test_data_frame_header, 8) == 0, "Packed data is invalid.");
}
END_TEST

Suite * spdy_data_frame_suite()
{
	Suite *s = suite_create("spdy_data_frame");
	TCase *tc_core = tcase_create("spdy_data_frame_parse_header");
	tcase_add_test(tc_core, test_spdy_data_frame_parse_header);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_data_frame_parse");
	tcase_add_test(tc_core, test_spdy_data_frame_parse);
	suite_add_tcase (s, tc_core);
	tc_core = tcase_create("spdy_data_frame_pack_header");
	tcase_add_test(tc_core, test_spdy_data_frame_pack_header);
	suite_add_tcase(s, tc_core);
	tc_core = tcase_create("spdy_data_frame_parse_pack");
	tcase_add_test(tc_core, test_spdy_data_frame_parse_pack);
	suite_add_tcase(s, tc_core);
	return s;
}

