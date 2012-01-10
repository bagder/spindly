#include "spindly.h"

#include <check.h>

START_TEST (test_spindly_phys_init)
{

  struct spindly_phys *phys;

  phys = spindly_phys_init(SPINDLY_SIDE_CLIENT, SPINDLY_DEFAULT, NULL);

  fail_unless(phys != NULL, "spindly_phys_init() failed");

  spindly_phys_cleanup(phys);
}
END_TEST


Suite *spindly_phys_suite()
{
  Suite *s = suite_create("spindly_phys");
  TCase *tc_core = tcase_create("spindly_phys_init");
  tcase_add_test(tc_core, test_spindly_phys_init);
  suite_add_tcase(s, tc_core);
  return s;
}

