#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_zlib.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <zlib.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/**
 * According to zlib documentation, this is required to avoid
 * corruption of input and output data on windows.
 */
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

/**
 * This is the dictionary that is used by the zlib compression in SPDY.
 */
char *spdy_zlib_dictionary =
  "optionsgetheadpostputdeletetraceacceptaccept-charsetaccept-encodingaccept-languageauthorizationexpectfromhostif-modified-sinceif-matchif-none-matchif-rangeif-unmodifiedsincemax-forwardsproxy-authorizationrangerefererteuser-agent100101200201202203204205206300301302303304305306307400401402403404405406407408409410411412413414415416417500501502503504505accept-rangesageetaglocationproxy-authenticatepublicretry-afterservervarywarningwww-authenticateallowcontent-basecontent-encodingcache-controlconnectiondatetrailertransfer-encodingupgradeviawarningcontent-languagecontent-lengthcontent-locationcontent-md5content-rangecontent-typeetagexpireslast-modifiedset-cookieMondayTuesdayWednesdayThursdayFridaySaturdaySundayJanFebMarAprMayJunJulAugSepOctNovDecchunkedtext/htmlimage/pngimage/jpgimage/gifapplication/xmlapplication/xhtmltext/plainpublicmax-agecharset=iso-8859-1utf-8gzipdeflateHTTP/1.1statusversionurl";

#define SPDY_ZLIB_CHUNK 16384   /* TODO: Is this smart enough? */

/**
 * Deflate data as used in the header compression of spdy.
 * @param src - Data to deflate
 * @param length - Length of data
 * @param data_used - Amount of data used by zlib.
 * @param dest - Destination of deflated data
 * @param dest_size - Pointer to size of deflated data.
 * @see spdy_zlib_inflate
 * @return Errorcode
 */
int spdy_zlib_deflate(char *src, uint32_t length, size_t *data_used,
                      char **dest, size_t *dest_size)
{
  z_stream strm;
  int ret, flush;
  unsigned int have;
  unsigned char out[SPDY_ZLIB_CHUNK];
  *dest = NULL;
  *dest_size = 0;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, -1);
  if(ret != Z_OK) {
    SPDYDEBUG("Deflate init failed.");
    return SPDY_ERROR_ZLIB_DEFLATE_INIT_FAILED;
  }

  /* The zlib compression in spdy uses a dictionary which is available
   * in spdy_zlib_dictionary. Please note that the termination NUL-byte
   * is used. (May changes in a future version of SPDY.) */
  ret = deflateSetDictionary(&strm,
                             (unsigned char *)spdy_zlib_dictionary,
                             strlen(spdy_zlib_dictionary) + 1);
  if(ret != Z_OK) {
    deflateEnd(&strm);
    SPDYDEBUG("Deflate set dictionary failed.");
    return SPDY_ERROR_ZLIB_DEFLATE_DICT_FAILED;
  }

  /* Loop while flush is not Z_FINISH */
  do {
    if(length > SPDY_ZLIB_CHUNK) {
      strm.avail_in = SPDY_ZLIB_CHUNK;
      length -= SPDY_ZLIB_CHUNK;

      /* flush is used to detect if we still need to supply additional
       * data to the stream via avail_in and next_in. */
      flush = Z_NO_FLUSH;
    } else {
      strm.avail_in = length;
      flush = Z_FINISH;
    }
    *data_used += strm.avail_in;

    strm.next_in = (unsigned char *)src;

    /* Loop while output data is available */
    do {
      strm.avail_out = SPDY_ZLIB_CHUNK;
      strm.next_out = out;

      /* No need to check return value of deflate.
       * (See zlib documentation at http://www.zlib.net/zlib_how.html */
      ret = deflate(&strm, flush);
      /* Should only happen if some other part of the application
       * clobbered the memory of the stream. */
      assert(ret != Z_STREAM_ERROR);
      have = SPDY_ZLIB_CHUNK - strm.avail_out;

      /* (Re)allocate memory for dest and keep track of it's size. */
      *dest_size += have;
      *dest = realloc(*dest, *dest_size);
      if(!*dest) {
        deflateEnd(&strm);
        SPDYDEBUG("(Re)allocating memory for destination buffer failed.");
        return SPDY_ERROR_MALLOC_FAILED;
      }
      memcpy((*dest) + ((*dest_size) - have), out, have);

    } while(strm.avail_out == 0);
    /* At this point, all of the input data should already
     * have been used. */
    assert(strm.avail_in == 0);
  } while(flush != Z_FINISH);
  /* If the stream is not complete, something went very wrong. */
  assert(ret == Z_STREAM_END);

  deflateEnd(&strm);
  return SPDY_ERROR_NONE;
}

/**
 * Initialize an inflate context.
 * @param ctx - Context to initialize
 * @todo Testcase!
 * @return Errorcode
 */
int spdy_zlib_inflate_init(spdy_zlib_context *ctx)
{
  int ret;
  ctx->stream.zalloc = Z_NULL;
  ctx->stream.zfree = Z_NULL;
  ctx->stream.opaque = Z_NULL;
  ctx->stream.avail_in = 0;
  ctx->stream.next_in = Z_NULL;
  ret = inflateInit2(&ctx->stream, 15);
  if(ret != Z_OK) {
    return SPDY_ERROR_ZLIB_INFLATE_INIT_FAILED;
  }
  return SPDY_ERROR_NONE;
}

/**
 * End an inflate context. (Like on connection close.)
 * @param ctx - Context to end.
 * @todo Testcase (including leak check?)
 */
void spdy_zlib_inflate_end(spdy_zlib_context *ctx)
{
  inflateEnd(&ctx->stream);
}

/**
 * Inflate data as used in the header compression of spdy.
 * @param ctx - Compression context
 * @param src - Data to inflate
 * @param length - Length of data
 * @param dest - Destination of inflated data
 * @param dest_size - Pointer to size of inflated data.
 * @see spdy_zlib_deflate
 * @return Errorcode
 */
int spdy_zlib_inflate(spdy_zlib_context *ctx,
                      char *src,
                      uint32_t length, char **dest, size_t *dest_size)
{
  int ret;
  unsigned int have;
  unsigned char out[SPDY_ZLIB_CHUNK];
  *dest = NULL;
  *dest_size = 0;

  assert(ctx != NULL);

  /* Loop while inflate return is not Z_STREAM_END */
  do {

    /* Only read CHUNK amount of data if supplied data is bigger then
     * CHUNK. */
    if(length > SPDY_ZLIB_CHUNK) {
      ctx->stream.avail_in = SPDY_ZLIB_CHUNK;
      length -= SPDY_ZLIB_CHUNK;
    } else {
      ctx->stream.avail_in = length;
      length = 0;
    }

    /* Determine if we actually have data for inflate. */
    if(ctx->stream.avail_in == 0)
      break;

    ctx->stream.next_in = (unsigned char *)src;

    /* Loop while output data is available */
    do {
      ctx->stream.avail_out = SPDY_ZLIB_CHUNK;
      ctx->stream.next_out = out;
      ret = inflate(&ctx->stream, Z_SYNC_FLUSH);

      /* Should only happen if some other part of the application
       * clobbered the memory of the stream. */
      assert(ret != Z_STREAM_ERROR);    /* state not clobbered */

      switch (ret) {
      case Z_NEED_DICT:
        /* Setting the dictionary for the SPDY zlib compression. */
        ret = inflateSetDictionary(&ctx->stream,
                                   (unsigned char *)spdy_zlib_dictionary,
                                   strlen(spdy_zlib_dictionary) + 1);
        if(ret != Z_OK) {
          inflateEnd(&ctx->stream);
          SPDYDEBUG("inflateSetDictionary failed.");
          return SPDY_ERROR_ZLIB_INFLATE_DICT_FAILED;
        }
        ret = inflate(&ctx->stream, Z_SYNC_FLUSH);
        break;
      case Z_DATA_ERROR:
        inflateEnd(&ctx->stream);
        SPDYDEBUG("DATA ERROR");
        return SPDY_ERROR_ZLIB_INFLATE_FAILED;
      case Z_MEM_ERROR:
        inflateEnd(&ctx->stream);
        SPDYDEBUG("MEM ERROR");
        return SPDY_ERROR_ZLIB_INFLATE_FAILED;
      }
      have = SPDY_ZLIB_CHUNK - ctx->stream.avail_out;

      /* (Re)allocate and copy to destination. */
      *dest_size += have;
      *dest = realloc(*dest, *dest_size);
      if(!*dest) {
        SPDYDEBUG("REALLOC FAILED");
        inflateEnd(&ctx->stream);
        return SPDY_ERROR_MALLOC_FAILED;
      }
      /* Copy to destination + size of the destination.
       * As dest_size has always been increased by 'have', we need
       * to decrease it. */
      memcpy((*dest) + ((*dest_size) - have), out, have);
    } while(ctx->stream.avail_out == 0);
  } while(ret != Z_STREAM_END);

  return Z_STREAM_END ? SPDY_ERROR_NONE : SPDY_ERROR_ZLIB_INFLATE_FAILED;
}
