#include "check_spdy_zlib.h"
#include "check_spdy_frame.h"
#include "check_spdy_control_frame.h"
#include "check_spdy_data_frame.h"
#include "check_spdy_nv_block.h"
#include "check_spdy_syn_stream.h"
#include "check_spdy_syn_reply.h"
#include "check_spdy_rst_stream.h"
#include "check_spindly_phys.h"

#include <stdlib.h>

int main() {
	int number_failed;
	Suite *spdy_suite = suite_create("spdy");
	SRunner *sr = srunner_create(spdy_suite);
	srunner_add_suite(sr, spdy_zlib_suite());
	srunner_add_suite(sr, spdy_control_frame_suite());
	srunner_add_suite(sr, spdy_data_frame_suite());
	srunner_add_suite(sr, spdy_frame_suite());
	srunner_add_suite(sr, spdy_nv_block_suite());
	srunner_add_suite(sr, spdy_syn_stream_suite());
	srunner_add_suite(sr, spdy_syn_reply_suite());
	srunner_add_suite(sr, spdy_rst_stream_suite());
	srunner_add_suite(sr, spindly_phys_suite());
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

