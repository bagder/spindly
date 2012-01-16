#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "spdy_setup.h"
#include "spdy_nv_block.h"

int main(int argc, char *argv[])
{
  spdy_nv_block nvb;
  nvb.count = 0;
  nvb.pairs = NULL;

  int i;
  for(i = 1; i < argc; i++) {
    /* strtok manipulates the string, so we need to copy it to keep
       argv[i] consistent. */
    char *tmp_argv = malloc(strlen(argv[i]) + 1);
    char *tmp_argv_ref = tmp_argv;
    if(!tmp_argv)
      exit(1);
    memcpy(tmp_argv, argv[i], strlen(argv[i]) + 1);
    char *pair_name = strtok(tmp_argv, "=");
    if(!pair_name)
      exit(1);
    char *pair_value = strtok(NULL, "=");
    if(!pair_value)
      exit(1);
    nvb.pairs = realloc(nvb.pairs, sizeof(spdy_nv_pair) * (nvb.count + 1));
    spdy_nv_pair *nvp = &nvb.pairs[nvb.count];
    nvp->name = malloc(strlen(pair_name) + 1);
    if(!nvp->name)
      exit(1);
    strcpy(nvp->name, pair_name);
    nvp->values = malloc(strlen(pair_value) + 1);
    if(!nvp->values)
      exit(1);
    strcpy(nvp->values, pair_value);
    nvp->values_count = 1;

    nvb.count++;

    free(tmp_argv_ref);
  }

  char *nv_block_data = NULL;
  size_t nv_block_data_size = 0;
  spdy_nv_block_pack(&nv_block_data, &nv_block_data_size, &nvb);
  printf("Size: %d\n", nv_block_data_size);
  for(i = 0; i < nv_block_data_size; i++) {
    printf("%02x ", nv_block_data[i]);
    /* Printed 8 bytes, make a newline. */
    if(((i + 1) % 8) == 0) {
      printf("\n");
    } else if(((i + 1) % 4) == 0) {
      printf("   ");
    }
  }
  printf("\n");
  exit(0);
}
