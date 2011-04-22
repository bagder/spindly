#include "spdy_zlib.h"

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

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

/**
 * This is the dictionary that is used by the zlib compression in SPDY.
 */
char *spdy_zlib_dictionary = "optionsgetheadpostputdeletetraceacceptaccept-charsetaccept-encodingaccept-languageauthorizationexpectfromhostif-modified-sinceif-matchif-none-matchif-rangeif-unmodifiedsincemax-forwardsproxy-authorizationrangerefererteuser-agent100101200201202203204205206300301302303304305306307400401402403404405406407408409410411412413414415416417500501502503504505accept-rangesageetaglocationproxy-authenticatepublicretry-afterservervarywarningwww-authenticateallowcontent-basecontent-encodingcache-controlconnectiondatetrailertransfer-encodingupgradeviawarningcontent-languagecontent-lengthcontent-locationcontent-md5content-rangecontent-typeetagexpireslast-modifiedset-cookieMondayTuesdayWednesdayThursdayFridaySaturdaySundayJanFebMarAprMayJunJulAugSepOctNovDecchunkedtext/htmlimage/pngimage/jpgimage/gifapplication/xmlapplication/xhtmltext/plainpublicmax-agecharset=iso-8859-1utf-8gzipdeflateHTTP/1.1statusversionurl";

#define CHUNK 16384 // TODO: Is this smart enough?

/**
 * Deflate data as used in the header compression of spdy.
 * @param src Data to deflate
 * @param length Length of data
 * @param dest Destination of deflated data
 * @param ret_size Pointer to size of deflated data.
 * @see spdy_zlib_inflate
 * @return 0 on success, -1 on failure.
 */
int spdy_zlib_deflate(char *src, uint32_t length, char **dest, size_t *dest_size) {
	int ret, flush;
	unsigned int have;
	z_stream strm;
	unsigned char out[CHUNK];
	*dest = NULL;
	*dest_size=0;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, -1);
	if(ret != Z_OK) {
		return -1;
	}

	// The zlib compression in spdy uses a dictionary which is available
	// in spdy_zlib_dictionary. Please note that the termination NUL-byte
	// is used. (May changes in a future version of SPDY.)
	ret = deflateSetDictionary(
			&strm,
			(unsigned char*)spdy_zlib_dictionary,
			strlen(spdy_zlib_dictionary)+1);
	if(ret != Z_OK) {
		deflateEnd(&strm);
		return -1;
	}

	// Loop while flush is not Z_FINISH
	do {
		if(length > CHUNK) {
			strm.avail_in = CHUNK;
			length -= CHUNK;

			// flush is used to detect if we still need to supply additional
			// data to the stream via avail_in and next_in.
			flush = Z_NO_FLUSH;
		} else {
			strm.avail_in = length;
			flush = Z_FINISH;
		}

		strm.next_in = (unsigned char*)src;

		// Loop while output data is available
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;

			// No need to check return value of deflate.
			// (See zlib documentation at http://www.zlib.net/zlib_how.html
			ret = deflate(&strm, flush);
			// Should only happen if some other part of the application
			// clobbered the memory of the stream.
			assert(ret != Z_STREAM_ERROR);
			have = CHUNK - strm.avail_out;

			// (Re)allocate memory for dest and keep track of it's size.
			*dest_size += have;
			*dest = realloc(*dest, *dest_size);
			if(!*dest) {
				deflateEnd(&strm);
				return -1;
			}
			memcpy((*dest)+((*dest_size)-have), out, have);

		} while (strm.avail_out == 0);
		// At this point, all of the input data should already
		// have been used.
		assert(strm.avail_in == 0);
	} while (flush != Z_FINISH);
	// If the stream is not complete, something went very wrong.
	assert(ret == Z_STREAM_END);

	deflateEnd(&strm);
	return 0;
}

/**
 * Inflate data as used in the header compression of spdy.
 * @param src Data to inflate
 * @param length Length of data
 * @param dest Destination of inflated data
 * @param dest_size Pointer to size of inflated data.
 * @see spdy_zlib_deflate
 * @return 0 on success, -1 on failure.
 */
int spdy_zlib_inflate(char *src, uint32_t length, char **dest, size_t *dest_size) {
	int ret;
	unsigned int have;
	z_stream strm;
	unsigned char out[CHUNK];
	*dest = NULL;
	*dest_size=0;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	ret = inflateInit(&strm);
	if(ret != Z_OK) {
		return -1;
	}

	// Loop while inflate return is not Z_STREAM_END
	do {

		// Only read CHUNK amount of data if supplied data is bigger then
		// CHUNK.
		if(length > CHUNK) {
			strm.avail_in = CHUNK;
			length -= CHUNK;
		} else {
			strm.avail_in = length;
			length = 0;
		}

		// Determine if we actually have data for inflate.
		if(strm.avail_in == 0)
			break;

		strm.next_in = (unsigned char*)src;

		// Loop while output data is available
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);

			// Should only happen if some other part of the application
			// clobbered the memory of the stream.
			assert(ret != Z_STREAM_ERROR); /* state not clobbered */

			switch(ret) {
				case Z_NEED_DICT:
					// Setting the dictionary for the SPDY zlib compression.
					ret = inflateSetDictionary(
							&strm,
							(unsigned char*)spdy_zlib_dictionary,
							strlen(spdy_zlib_dictionary)+1);
					if(ret != Z_OK) {
						inflateEnd(&strm);
						return -1;
					}
					ret = inflate(&strm, Z_NO_FLUSH);
					break;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					inflateEnd(&strm);
					return -1;
			}
			have = CHUNK - strm.avail_out;

			// (Re)allocate and copy to destination.
			*dest_size += have;
			*dest = realloc(*dest, *dest_size);
			if(!*dest) {
				inflateEnd(&strm);
				return -1;
			}
			memcpy((*dest)+((*dest_size)-have), out, have);
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);
	inflateEnd(&strm);
	return Z_STREAM_END ? 0 : -1;
}

