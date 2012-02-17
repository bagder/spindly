#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_frame.h"
#include "spdy_control_frame.h"
#include "spdy_data_frame.h"
#include "spdy_error.h"
#include "spdy_log.h"

#include <stdlib.h>

int spdy_frame_init(spdy_frame *frame)
{
  frame->_header_parsed = 0;
  frame->prev = NULL;
  frame->next = NULL;
  return SPDY_ERROR_NONE;
}

/**
 * Parse the header of a frame.
 * @param frame - Target frame.
 * @param data - Data to parse.
 * @see spdy_frame
 * @return Errorcode
 */
int spdy_frame_parse_header(spdy_frame *frame, spdy_data *data)
{
  /**
   * Read the type bit
   * (The mask equals 0x10000000, filtering all but the first bit.)
   */
  frame->type = (data->cursor[0] & 0x80) ? SPDY_CONTROL_FRAME :
    SPDY_DATA_FRAME;

  return SPDY_ERROR_NONE;
}

/**
 * Parse a frame.
 * @param frame - Target frame.
 * @param hash - hash for streamid lookup
 * @param data - Data to parse.
 * @see spdy_frame
 * @return Errorcode
 */
int spdy_frame_parse(spdy_frame *frame, struct spindly_phys *phys,
                     spdy_data *data)
{
  int ret;
  if(!frame->_header_parsed) {
    ret = spdy_frame_parse_header(frame, data);
    if(ret != SPDY_ERROR_NONE) {
      SPDYDEBUG("Frame parse header failed.");
      return ret;
    }
    if(frame->type == SPDY_CONTROL_FRAME)
      spdy_control_frame_init(&frame->frame.control);
    else if(frame->type == SPDY_DATA_FRAME)
      spdy_data_frame_init(&frame->frame.data);

    frame->_header_parsed = true;
  }
  switch (frame->type) {
  case SPDY_CONTROL_FRAME:
    ret = spdy_control_frame_parse(&frame->frame.control, phys, data);
    if(ret != SPDY_ERROR_NONE) {
      SPDYDEBUG("Control frame parse failed.");
      return ret;
    }
    break;
  case SPDY_DATA_FRAME:
    ret = spdy_data_frame_parse(&frame->frame.data, data);
    if(ret != SPDY_ERROR_NONE) {
      SPDYDEBUG("Data frame parse failed.");
      return ret;
    }
    break;
  default:
    SPDYDEBUG("UNSUPPORTED");
    break;
  }
  return SPDY_ERROR_NONE;
}

void spdy_frame_destroy(spdy_frame *frame)
{
  switch (frame->type) {
  case SPDY_CONTROL_FRAME:
    spdy_control_frame_destroy(&frame->frame.control);
    break;
  case SPDY_DATA_FRAME:
    break;
  }


}
