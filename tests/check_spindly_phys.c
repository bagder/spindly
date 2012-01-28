#include "spindly.h"

#include <check.h>

START_TEST (test_spindly_phys_init)
{
  struct spindly_phys *phys_client;
  struct spindly_phys *phys_server;

  phys_client = spindly_phys_init(SPINDLY_SIDE_CLIENT, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_client != NULL, "spindly_phys_init() failed");

  phys_server = spindly_phys_init(SPINDLY_SIDE_SERVER, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_server != NULL, "spindly_phys_init() failed");

  spindly_phys_cleanup(phys_client);
  spindly_phys_cleanup(phys_server);
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

