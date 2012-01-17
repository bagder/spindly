#ifndef SPINDLY_PHYS_H
#define SPINDLY_PHYS_H

#include "list.h"

/* The default number of slots allocated for streams in a phys handle */
#define PHYS_DEFAULT_NUM_STREAMS 5

struct spindly_phys
{
  spindly_side_t side;
  spindly_spdyver_t protver;

  /* all the streams on this physical connection */
  struct list_head streams;
  int num_streams;             /* how many have been added so far */
  uint32_t streamid;            /* the next streamid to ask for */

  /* list of handles to go over for outgoing traffic */
  struct list_head outq;

  struct spindly_phys_config *config;
};

/* internal functions */

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s);

#endif /* SPINDLY_PHYS_H */
