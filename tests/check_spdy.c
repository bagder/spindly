#include "check_spdy_nv_block.h"

#include <stdlib.h>

int run_suite(Suite *suite) {
	int number_failed;
	SRunner *sr = srunner_create(suite);
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return number_failed;
}

int main() {
	int number_failed;
	number_failed = run_suite(spdy_nv_block_suite());
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

