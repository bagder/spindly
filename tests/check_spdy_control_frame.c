#include "check_spdy_control_frame.h"
#include "spdy_data.h"
#include "spdy_control_frame.h"
#include "spdy_error.h"

#include "testdata.h"

START_TEST (test_spdy_control_frame_parse_header)
{
	spdy_control_frame frame;
	spdy_data data;
	spdy_control_frame_init(&frame);
	spdy_control_frame_parse_header(&frame, 
			spdy_data_use(&data, test_control_syn_stream_frame, 8));
	fail_unless(frame.version == 2, "Version parsing failed.");
	fail_unless(frame.type == 1, "Type parsing failed.");
	fail_unless(frame.flags == 1, "Flag parsing failed.");
	fail_unless(frame.length == 288, "Length parsing failed.");
}
END_TEST

START_TEST (test_spdy_control_frame_pack_header)
{
	char *out=NULL;
	int ret;
	spdy_control_frame frame;
	frame.version = 2;
	frame.type = 1;
	frame.flags = 1;
	frame.length = 288;
	ret = spdy_control_frame_pack_header(&out, &frame);
	fail_unless(ret == 0, "spdy_control_frame_pack_header failed.");
	fail_unless(memcmp(out, test_control_syn_stream_frame, 8) == 0, "Packed data is invalid.");
}
END_TEST

START_TEST (test_spdy_control_frame_parse_pack_header)
{
	spdy_control_frame frame;
	char *out;
	spdy_data data;
	int ret;
	spdy_control_frame_init(&frame);
	ret = spdy_control_frame_parse_header(&frame,
			spdy_data_use(&data, test_control_syn_stream_frame, 8));
	fail_unless(ret == 0, "spdy_control_frame_parse_header failed.");
	ret = spdy_control_frame_pack_header(&out, &frame);
	fail_unless(ret == 0, "spdy_control_frame_pack_header failed.");
	fail_unless(memcmp(out, test_control_syn_stream_frame, 8) == 0, "Input is different than repacked frame.");
}
END_TEST

START_TEST (test_spdy_control_frame_parse)
{
	spdy_zlib_context zlib_ctx;
	spdy_control_frame frame;
	spdy_data data;
	int ret;

	spdy_zlib_inflate_init(&zlib_ctx);

	spdy_control_frame_init(&frame);
	ret = spdy_control_frame_parse(
			&frame,
			spdy_data_use(&data, test_control_syn_stream_frame, 296),
			&zlib_ctx);
	fail_unless(ret == SPDY_ERROR_NONE, "spdy_control_frame_parse failed.");
	fail_unless(data.cursor - data.data == 296, "data_used is incorrect.");
	fail_unless(frame.version == 2, "Version failed.");
	fail_unless(frame.type == SPDY_CTRL_SYN_STREAM, "Type failed.");
	fail_unless(frame.flags == 1, "Flag failed.");
	fail_unless(frame.length == 288, "Length failed.");
        spdy_control_frame_destroy(&frame);
        spdy_zlib_inflate_end(&zlib_ctx);
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
	tc_core = tcase_create("spdy_control_frame_parse_pack_header");
	tcase_add_test(tc_core, test_spdy_control_frame_parse_pack_header);
	suite_add_tcase(s, tc_core);
	tc_core = tcase_create("spdy_control_frame_parse");
	tcase_add_test(tc_core, test_spdy_control_frame_parse);
	suite_add_tcase(s, tc_core);
	return s;
}

