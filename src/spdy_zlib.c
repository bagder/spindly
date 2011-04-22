#include "spdy_zlib.h"

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define SPDY_DEBUG

#ifdef SPDY_DEBUG
	#define DEBUGMSG(string) puts(string)
#else
	#define DEBUGMSG(string)
#endif

/**
 * According to zlib documentation, this is required to avoid
 * corruption of input and output data on windows.
 */
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

char *spdy_zlib_dictionary = "optionsgetheadpostputdeletetraceacceptaccept-charsetaccept-encodingaccept-languageauthorizationexpectfromhostif-modified-sinceif-matchif-none-matchif-rangeif-unmodifiedsincemax-forwardsproxy-authorizationrangerefererteuser-agent100101200201202203204205206300301302303304305306307400401402403404405406407408409410411412413414415416417500501502503504505accept-rangesageetaglocationproxy-authenticatepublicretry-afterservervarywarningwww-authenticateallowcontent-basecontent-encodingcache-controlconnectiondatetrailertransfer-encodingupgradeviawarningcontent-languagecontent-lengthcontent-locationcontent-md5content-rangecontent-typeetagexpireslast-modifiedset-cookieMondayTuesdayWednesdayThursdayFridaySaturdaySundayJanFebMarAprMayJunJulAugSepOctNovDecchunkedtext/htmlimage/pngimage/jpgimage/gifapplication/xmlapplication/xhtmltext/plainpublicmax-agecharset=iso-8859-1utf-8gzipdeflateHTTP/1.1statusversionurl";

#define CHUNK 16384 // TODO: Is this smart enough?

int spdy_zlib_deflate(char *src, uint32_t length, char **dest) {
	int ret, flush;
	unsigned int have;
	z_stream strm;
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, -1);
	if(ret != Z_OK) {
		return -1;
	}

	do {
		if(length > CHUNK) {
			strm.avail_in = CHUNK;
			length -= CHUNK;
			flush = Z_NO_FLUSH;
		} else {
			strm.avail_in = length;
			flush = Z_FINISH;
		}
		strm.next_in = (unsigned char*)src;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush); /* no bad return value */
			assert(ret != Z_STREAM_ERROR); /* state clobbered */
			have = CHUNK - strm.avail_out;
			/* TODO: Handle/Write output to dest. */
			(void)dest;
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0); /* all of input used */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END); /* stream must be complete */

	deflateEnd(&strm);
	return 0;
}
int spdy_zlib_inflate(char *src, uint32_t length, char **dest) {
	int ret;
	unsigned int have;
	z_stream strm;
	unsigned char out[CHUNK];
	*dest = NULL;
	size_t dest_size=0;
	size_t dest_size_old=0;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if(ret != Z_OK) {
		DEBUGMSG("INIT FAIL");
		return -1;
	}

	do {
		if(length > CHUNK) {
			strm.avail_in = CHUNK;
			length -= CHUNK;
		} else {
			strm.avail_in = length;
			length = length - strm.avail_in;
		}
		if(strm.avail_in == 0)
			break;

		strm.next_in = (unsigned char*)src;
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR); /* state not clobbered */
			switch(ret) {
				case Z_NEED_DICT:
					ret = inflateSetDictionary(
							&strm,
							(unsigned char*)spdy_zlib_dictionary,
							strlen(spdy_zlib_dictionary)+1);
					if(ret != Z_OK) {
						inflateEnd(&strm);
						DEBUGMSG("DICTFAIL");
						return -1;
					}
					ret = inflate(&strm, Z_NO_FLUSH);
					break;
				case Z_DATA_ERROR:
					DEBUGMSG("DATAERROR");
				case Z_MEM_ERROR:
					inflateEnd(&strm);
					DEBUGMSG("MEMERROR");
					return -1;
			}
			have = CHUNK - strm.avail_out;
			/* TODO: Optimizie allocation? */
			(void)dest;
			dest_size_old = dest_size;
			dest_size += have;
			*dest = realloc(*dest, dest_size);
			if(!*dest) {
				inflateEnd(&strm);
				return -1;
			}
			memcpy((*dest)+dest_size_old, out, have);
		} while (strm.avail_out == 0);
		/* We're done when inflate() says it's done */
	} while (ret != Z_STREAM_END);
	inflateEnd(&strm);
	return Z_STREAM_END ? 0 : -1;
}

