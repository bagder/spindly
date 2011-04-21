#include "check_spdy_nv_block.h"
#include "check_spdy_frame.h"

#include <stdlib.h>

int main() {
	int number_failed;
	Suite *spdy_suite = suite_create("spdy");
	SRunner *sr = srunner_create(spdy_suite);
	srunner_add_suite(sr, spdy_frame_suite());
	srunner_add_suite(sr, spdy_nv_block_suite());
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

