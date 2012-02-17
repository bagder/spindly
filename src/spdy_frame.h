#ifndef SPDY_FRAME_H_
#define SPDY_FRAME_H_ 1

#include "spdy_data.h"
#include "spdy_control_frame.h"
#include "spdy_data_frame.h"

#include <stdint.h>
#include <stdlib.h>

struct hash; /* forward declaration see hash.h */

/**
 * Frame type enum
 * Contains the identifiers for the different frame types.
 */
enum SPDY_FRAME_TYPE
{
  SPDY_DATA_FRAME = 0,          /*!< SPDY Data Frame */
  SPDY_CONTROL_FRAME = 1        /*!< SPDY Control Frame */
};

/**
 * Frame
 * Indicates the type of the frame and keeps a pointer to a
 * control or data frame.
 */
typedef struct spdy_frame spdy_frame;
struct spdy_frame
{
  bool _header_parsed;
  enum SPDY_FRAME_TYPE type;    /*!< Type of the frame */
  union
  {
    spdy_control_frame control;
    spdy_data_frame data;
  } frame;

  /* Used to chain related frames. */
  spdy_frame *prev;
  spdy_frame *next;
};

int spdy_frame_init(spdy_frame *frame);
int spdy_frame_parse_header(spdy_frame *frame, spdy_data *data);
int spdy_frame_parse(spdy_frame *frame, struct spindly_phys *phys,
                     spdy_data *data);
void spdy_frame_destroy(spdy_frame *frame);

#endif
