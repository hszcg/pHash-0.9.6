/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Evan Klinger <eklinger@phash.org>                           |
   +----------------------------------------------------------------------+
*/

/* 
	Modified by Ivan Pizhenko <ivanp2015@users.noreply.github.com>
	https://github.com/ivanp2015/pHash-0.9.6
*/

/* $ Id: $ */

#include "php_pHash.h"
#include <cctype>
#include <cstring>
#include <algorithm>
#include "debug.h"


#if HAVE_PHASH

struct ph_audio_hash
{
	uint32_t *hash;
	int len;
};
struct ph_video_hash
{
	ulong64 *hash;
	int len;
};
struct ph_text_hash
{
	TxtHashPoint *p;
	int count;
};

/* {{{ phpinfo logo definitions */

#if PHP_VERSION_ID < 50500
#include "php_logos.h"
#endif


static unsigned char pHash_logo[] = {
#include "pHash_logos.h"
};
/* }}} */

/* {{{ Resource destructors */
int le_ph_video_hash;
extern "C" void ph_video_hash_dtor(PORTABLE_ZEND_RESOURCE *rsrc TSRMLS_DC)
{
	ph_video_hash * resource = (ph_video_hash *)(rsrc->ptr);

	if(resource)
	{
		free(resource->hash);
		free(resource);
	}
}

int le_ph_image_hash;
extern "C" void ph_image_hash_dtor(PORTABLE_ZEND_RESOURCE *rsrc TSRMLS_DC)
{
	ulong64 * resource = (ulong64 *)(rsrc->ptr);

	if(resource)
		free(resource);
}

int le_ph_audio_hash;
extern "C" void ph_audio_hash_dtor(PORTABLE_ZEND_RESOURCE *rsrc TSRMLS_DC)
{
	ph_audio_hash * resource = (ph_audio_hash *)(rsrc->ptr);

	if(resource)
	{
		free(resource->hash);
		free(resource);
	}
}

int le_ph_txt_hash;
extern "C" void ph_txt_hash_dtor(PORTABLE_ZEND_RESOURCE *rsrc TSRMLS_DC)
{
	ph_text_hash * resource = (ph_text_hash *)(rsrc->ptr);

	if(resource)
	{
		free(resource->p);
		free(resource);
	}
}

/* }}} */

/* {{{ pHash_functions[] */
zend_function_entry pHash_functions[] = {
#if HAVE_VIDEO_HASH
	PHP_FE(ph_dct_videohash    , ph_dct_videohash_arg_info)
	PHP_FE(ph_dct_videohash2   , ph_dct_videohash2_arg_info) // [IP] custom function
	PHP_FE(ph_dct_videohash3   , ph_dct_videohash3_arg_info) // [IP] custom function
#endif /* HAVE_VIDEO_HASH */
#if HAVE_IMAGE_HASH
	PHP_FE(ph_dct_imagehash    , ph_dct_imagehash_arg_info)
#endif /* HAVE_IMAGE_HASH */
	PHP_FE(ph_texthash         , ph_texthash_arg_info)
#if HAVE_AUDIO_HASH
	PHP_FE(ph_audiohash        , ph_audiohash_arg_info)
	PHP_FE(ph_audiohash2       , ph_audiohash2_arg_info) // [IP] custom function
#endif /* HAVE_AUDIO_HASH */
#if HAVE_IMAGE_HASH
	PHP_FE(ph_image_dist       , ph_image_dist_arg_info)
#endif /* HAVE_IMAGE_HASH */
#if HAVE_VIDEO_HASH
	PHP_FE(ph_video_dist       , ph_video_dist_arg_info)
	PHP_FE(ph_video_dist2      , ph_video_dist2_arg_info) // [IP] custom function
#endif /* HAVE_VIDEO_HASH */
#if HAVE_AUDIO_HASH
	PHP_FE(ph_audio_dist       , ph_audio_dist_arg_info)
	PHP_FE(ph_audio_dist2      , ph_audio_dist2_arg_info) // [IP] custom function
#endif /* HAVE_AUDIO_HASH */
	PHP_FE(ph_compare_text_hashes, ph_compare_text_hashes_arg_info)
	{ NULL, NULL, NULL }
};
/* }}} */


/* {{{ pHash_module_entry
 */
zend_module_entry pHash_module_entry = {
	STANDARD_MODULE_HEADER,
	"pHash",
	pHash_functions,
	PHP_MINIT(pHash),     /* Replace with NULL if there is nothing to do at php startup   */
	PHP_MSHUTDOWN(pHash), /* Replace with NULL if there is nothing to do at php shutdown  */
	PHP_RINIT(pHash),     /* Replace with NULL if there is nothing to do at request start */
	PHP_RSHUTDOWN(pHash), /* Replace with NULL if there is nothing to do at request end   */
	PHP_MINFO(pHash),
	PHP_PHASH_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHASH
extern "C" {
ZEND_GET_MODULE(pHash)
} // extern "C"
#endif


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(pHash)
{
#if PHP_VERSION_ID < 50500
	php_register_info_logo("PHASH_LOGO_ID", "", pHash_logo, 49651);
#endif
	le_ph_video_hash = zend_register_list_destructors_ex(ph_video_hash_dtor,
						   NULL, "ph_video_hash", module_number);
	le_ph_image_hash = zend_register_list_destructors_ex(ph_image_hash_dtor,
						   NULL, "ph_image_hash", module_number);
	le_ph_audio_hash = zend_register_list_destructors_ex(ph_audio_hash_dtor,
						   NULL, "ph_audio_hash", module_number);
	le_ph_txt_hash = zend_register_list_destructors_ex(ph_txt_hash_dtor,
						   NULL, "ph_txt_hash", module_number);

	/* add your stuff here */

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(pHash)
{
#if PHP_VERSION_ID < 50500
	php_unregister_info_logo("PHASH_LOGO_ID");
#endif

	/* add your stuff here */

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(pHash)
{
	/* add your stuff here */

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(pHash)
{
	/* add your stuff here */

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(pHash)
{
	if (!sapi_module.phpinfo_as_text) {

	php_printf("<img src='");
	if (SG(request_info).request_uri) {
		php_printf("%s", (SG(request_info).request_uri));
	}
	php_printf("?=%s", "PHASH_LOGO_ID");
	php_printf("' align='right' alt='image' border='0'>\n");

	php_printf("pHash\n");
	php_info_print_table_start();
	php_info_print_table_row(2, "Version",PHP_PHASH_VERSION " (beta)");
	php_info_print_table_row(2, "Released", "2013-04-23");
	php_info_print_table_row(2, "CVS Revision", "$Id: $");
	php_info_print_table_row(2, "Authors", "Evan Klinger 'eklinger@phash.org' (lead)\n");
	php_info_print_table_row(2, "Modified-By", "Ivan Pizhenko 'ivanp2015@users.noreply.github.com'\n"); 
	php_info_print_table_row(2, "Modified-Source", "https://github.com/ivanp2015/pHash-0.9.6\n");
	php_info_print_table_row(2, "Modification-Date", "2016-12-31");
	php_info_print_table_end();
	/* add your stuff here */
}
}
/* }}} */

static inline void put_hex_symbol(char* pdest, unsigned char hex)
{
	*pdest = hex + ((hex < 10) ? '0' : 'A' - 10);
}


#if HAVE_VIDEO_HASH
/* {{{ proto resource ph_video_hash ph_dct_videohash(string file)
  pHash DCT video hash */
PHP_FUNCTION(ph_dct_videohash)
{
	ph_video_hash * return_res;
	PORTABLE_DECLARE_RETURN_RES_ID;

	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	int len;
	ulong64 *video_hash = ph_dct_videohash(file, len);
	if(video_hash)
	{
		ph_video_hash *p = (ph_video_hash *)malloc(sizeof(ph_video_hash));
		p->hash = video_hash;
		p->len = len;
		return_res = p;

	}
	else
		RETURN_FALSE;

	PORTABLE_ZEND_REGISTER_RESOURCE(return_res_id, return_value, return_res, le_ph_video_hash);
}
/* }}} ph_dct_videohash */


// [IP] our custom function
/* {{{ proto string ph_video_hash ph_dct_videohash2(string file)
  pHash DCT video hash as string */
PHP_FUNCTION(ph_dct_videohash2)
{
	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	debug_printf(("php: ph_dct_videohash2: File: %s\n", file));
	
	// compute pHash
	size_t len = 0;
	ulong64 *video_hash = ph_dct_videohash2(file, len);
	if(video_hash) {
		debug_printf(("php: ph_dct_videohash2: Hash Length: %zu\n", len));
		// allocate memory for output string
		char* hashstr =  static_cast<char*>(emalloc(len * sizeof(ulong64) * 2 + 1));
		if (hashstr) {
			// build output string
			char* dest = hashstr;
			const unsigned char* src = reinterpret_cast<unsigned char*>(video_hash);
			const unsigned char* src_end = src + len * sizeof(ulong64);
			while (src != src_end) {
				put_hex_symbol(dest++, (*src) >> 4);
				put_hex_symbol(dest++, (*src) & 0xF);
				++src;
			}
			*dest = 0;
			
			// free memory allocated by pHash
			free(video_hash);
			
			debug_printf(("php: ph_dct_videohash2: Hash: %s\n", hashstr));

			PORTABLE_RETURN_STRING(hashstr);
		} else {
			// free memory allocated by pHash
			free(video_hash);
		}
	}
	RETURN_FALSE;
}
/* }}} ph_dct_videohash2 */


// [IP] our custom function
/* {{{ proto string ph_video_hash ph_dct_videohash2(string file)
  pHash DCT video hash as string */
PHP_FUNCTION(ph_dct_videohash3)
{
	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	debug_printf(("php: ph_dct_videohash3: File: %s\n", file));
	
	// compute pHash
	int len0 = 0;
	ulong64 *video_hash = ph_dct_videohash(file, len0);
	size_t len = len0;
	if(video_hash) {
		debug_printf(("php: ph_dct_videohash3: Hash Length: %zu\n", len));
		// allocate memory for output string
		char* hashstr =  static_cast<char*>(emalloc(len * sizeof(ulong64) * 2 + 1));
		if (hashstr) {
			// build output string
			char* dest = hashstr;
			const unsigned char* src = reinterpret_cast<unsigned char*>(video_hash);
			const unsigned char* src_end = src + len * sizeof(ulong64);
			while (src != src_end) {
				put_hex_symbol(dest++, (*src) >> 4);
				put_hex_symbol(dest++, (*src) & 0xF);
				++src;
			}
			*dest = 0;
			
			// free memory allocated by pHash
			free(video_hash);
			
			debug_printf(("php: ph_dct_videohash3: Hash: %s\n", hashstr));
			
			PORTABLE_RETURN_STRING(hashstr);
		} else {
			// free memory allocated by pHash
			free(video_hash);
		}
	}
	RETURN_FALSE;
}
/* }}} ph_dct_videohash3 */

#endif /* HAVE_VIDEO_HASH */

#if HAVE_IMAGE_HASH
/* {{{ proto long ph_image_hash ph_dct_imagehash(string file)
  pHash DCT image hash */
PHP_FUNCTION(ph_dct_imagehash)
{
	ulong64 * return_res;
	long return_res_id = -1;

	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;
	char buffer [64];
	int n;
	char *str;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	ulong64 *hash = (ulong64 *)malloc(sizeof(ulong64));
	int ret = ph_dct_imagehash(file, *hash);
	if(ret != 0)
	{
		free(hash);
		RETURN_FALSE;
	}
	else
	{
		n = sprintf(buffer, "%016llx", *hash);
		str = estrdup(buffer);
		free(hash);
		PORTABLE_RETURN_STRING(str);
	}
}
/* }}} ph_dct_imagehash */

#endif /* HAVE_IMAGE_HASH */

/* {{{ proto resource ph_txt_hash ph_texthash(string file)
  pHash cyclic text hash */
PHP_FUNCTION(ph_texthash)
{
	ph_text_hash * return_res;
	PORTABLE_DECLARE_RETURN_RES_ID;

	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	int num;
	TxtHashPoint *txtHash = ph_texthash(file, &num);
	if(txtHash)
	{
		ph_text_hash *h = (ph_text_hash *)malloc(sizeof(ph_text_hash));
		h->p = txtHash;
		h->count = num;
		return_res = h;
	}
	else
		RETURN_FALSE;

	PORTABLE_ZEND_REGISTER_RESOURCE(return_res_id, return_value, return_res, le_ph_txt_hash);
}
/* }}} ph_texthash */


#if HAVE_AUDIO_HASH

/* {{{ proto resource ph_audio_hash ph_audiohash(string file, int sample_rate=5512, int channels=1)
  pHash audio hash */
PHP_FUNCTION(ph_audiohash)
{
	ph_audio_hash * return_res;
	PORTABLE_DECLARE_RETURN_RES_ID;

	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;
	PORTABLE_LONG_PARAMETER sample_rate = 5512;
	PORTABLE_LONG_PARAMETER channels = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &file, &file_len, &sample_rate, &channels) == FAILURE) {
		return;
	}

	int n;
	float *audiobuf = ph_readaudio(file, sample_rate, channels, NULL, n);
	if(audiobuf)
	{
		int nb_frames;
		uint32_t *hash = ph_audiohash(audiobuf, n, sample_rate, nb_frames);
		free(audiobuf);

		if(hash)
		{
			ph_audio_hash *h = (ph_audio_hash *)malloc(sizeof(ph_audio_hash));
			h->hash = hash;
			h->len = nb_frames;
			return_res = h;
		}
		else
			RETURN_FALSE;
	}
	else
		RETURN_FALSE;

	PORTABLE_ZEND_REGISTER_RESOURCE(return_res_id, return_value, return_res, le_ph_audio_hash);
}
/* }}} ph_audiohash */

/* {{{ proto string ph_audiohash2(string file, int sample_rate=5512, int channels=1)
  pHash audio hash as string */
PHP_FUNCTION(ph_audiohash2)
{
	ph_audio_hash * return_res;

	const char * file = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH file_len = 0;
	PORTABLE_LONG_PARAMETER sample_rate = 5512;
	PORTABLE_LONG_PARAMETER channels = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", 
		&file, &file_len, &sample_rate, &channels) == FAILURE) {
		return;
	}

	int n = 0;
	float *audiobuf = ph_readaudio(file, sample_rate, channels, NULL, n);
	if(audiobuf)
	{
		int nb_frames = 0;
		uint32_t *audio_hash = ph_audiohash(audiobuf, n, sample_rate, nb_frames);
		free(audiobuf);

		if(audio_hash) {
			// allocate memory for output string
			size_t len = nb_frames;
			char* hashstr =  static_cast<char*>(emalloc(len * sizeof(uint32_t) * 2 + 1));
			if (hashstr) {
				// build output string
				char* dest = hashstr;
				const unsigned char* src = reinterpret_cast<unsigned char*>(audio_hash);
				const unsigned char* src_end = src + len * sizeof(uint32_t);
				while (src != src_end) {
					put_hex_symbol(dest++, (*src) >> 4);
					put_hex_symbol(dest++, (*src) & 0xF);
					++src;
				}
				*dest = 0;
				
				// free memory allocated by pHash
				free(audio_hash);
				
				debug_printf(("php: ph_audiohash2: Hash: %s\n", hashstr));
				
				PORTABLE_RETURN_STRING(hashstr);
			} else {
				// free memory allocated by pHash
				debug_printf(("php: ph_audiohash2: emalloc hashstr failed"));
				free(audio_hash);
			}
		} else {
			debug_printf(("php: ph_audiohash2: audio hash failed"));
		}
	} else {
		debug_printf(("php: ph_audiohash2: read audio failed"));
	}
	RETURN_FALSE;
}
/* }}} ph_audiohash */


#endif /* HAVE_AUDIO_HASH */

#if HAVE_IMAGE_HASH
/* {{{ proto float ph_image_dist(resource ph_image_hash h1,resource ph_image_hash h2)
  pHash image distance. */
PHP_FUNCTION(ph_image_dist)
{
	zval * h1_res = NULL;
	int h1_resid = -1;
	zval * h2_res = NULL;
	int h2_resid = -1;

	char *num1 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH num1_len = 0;
	char *num2 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH num2_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &num1, &num1_len, &num2, &num2_len) == FAILURE) {
		return;
	}

	ulong64 h1;
	ulong64 h2;

	h1 = strtoull(num1, NULL, 16);
	h2 = strtoull(num2, NULL, 16);


	if(h1 && h2)
	{
		int dist = ph_hamming_distance(h1, h2);
		RETURN_DOUBLE(dist);
	}
	else
		RETURN_DOUBLE(-1);
}
/* }}} ph_image_dist */

#endif /* HAVE_IMAGE_HASH */

static inline unsigned char decode_hex_char(char ch)
{
	if (isdigit(ch)) {
		return ch - '0';
	} else if (isupper(ch)) {
		return ch - 'A' + 10;
	} else {
		return ch - 'a' + 10;
	}	
}

static bool decode_hex_string(unsigned char* dest, const char* src)
{
	while (*src) {
		char ch_hi = *src++;
		if (!isxdigit(ch_hi)) {
			return false;
		}

		char ch_lo = *src++;
		if (!isxdigit(ch_lo)) {
			return false;
		}

		*dest++ = (decode_hex_char(ch_hi) << 4) 
			| decode_hex_char(ch_lo);
	}
	return true;
}

template<class T>
struct emem_ptr
{
	emem_ptr(T* p) : m_p(p) {}
	emem_ptr(void* p) : m_p(static_cast<T*>(p)) {}
	~emem_ptr() { if(m_p) efree(m_p); }
	operator bool() const { return m_p != 0; }
	T* m_p;
};


#if HAVE_VIDEO_HASH

/* {{{ proto float ph_video_dist2(resource ph_video_hash h1,resource ph_video_hash h2, int thresh=21)
  pHash video distance. */
PHP_FUNCTION(ph_video_dist)
{
	zval * h1_res = NULL;
	ph_video_hash * h1 = NULL;

	zval * h2_res = NULL;
	ph_video_hash * h2 = NULL;

	PORTABLE_LONG_PARAMETER thresh = 21;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|l", &h1_res, &h2_res, &thresh) == FAILURE) {
		return;
	}

	PORTABLE_ZEND_FETCH_RESOURCE(h1, ph_video_hash *, &h1_res, -1, "ph_video_hash", le_ph_video_hash);
	PORTABLE_ZEND_FETCH_RESOURCE(h2, ph_video_hash *, &h2_res, -1, "ph_video_hash", le_ph_video_hash);

	if(h1 && h2)
	{
		double sim = ph_dct_videohash_dist(h1->hash, h1->len, h2->hash, h2->len, thresh);
		RETURN_DOUBLE(sim);
	}
	else
		RETURN_DOUBLE(-1);
}
/* }}} ph_video_dist */



// [IP] Our custom functon
/* {{{ proto float ph_video_dist2(string h1, string h2, int thresh=21)
  pHash video distance (hashes as strings). */
PHP_FUNCTION(ph_video_dist2)
{
	const char* str_h1 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH h1_len = 0;

	const char* str_h2 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH h2_len = 0;

	PORTABLE_LONG_PARAMETER thresh = 21;

	// parse functions parameters
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l", &str_h1, &h1_len, &str_h2, &h2_len, &thresh) == FAILURE) {
		return;
	}
	
	// printf("h1=%s\nh2=%s\n", str_h1, str_h2);

	// validate hash string lengths
	if(h1_len == 0 || h2_len == 0 || h1_len % (sizeof(ulong64) * 2) != 0 || h2_len % (sizeof(ulong64) * 2) != 0) {
		RETURN_DOUBLE(-1);
	}
	//printf(">>> 1\n");
	
	// allocate buffer for hash1
	emem_ptr<unsigned char> bh1(emalloc(h1_len / 2));
	if(!bh1) {
		RETURN_DOUBLE(-2);
	}
	//printf(">>> 2\n");

	// decode string hash #1 into the binary form
	if(!decode_hex_string(bh1.m_p, str_h1)) {
		RETURN_DOUBLE(-3);
	}
	//printf(">>> 3\n");
	
	// allocate buffer for hash2
	emem_ptr<unsigned char> bh2(emalloc(h2_len / 2));
	if(!bh2) {
		RETURN_DOUBLE(-4);
	}
	//printf(">>> 4\n");

	// decode string hash #1 into the binary form
	if(!decode_hex_string(bh2.m_p, str_h2)) {
		RETURN_DOUBLE(-5);
	}
	//printf(">>> 5\n");

	double sim = 0.0;
	if(ph_dct_videohash_dist2(
		reinterpret_cast<const ulong64*>(bh1.m_p), h1_len / 16, 
		reinterpret_cast<const ulong64*>(bh2.m_p), h2_len / 16,
		thresh, &sim) < 0) {
		//printf(">>> 6\n");
		RETURN_DOUBLE(-6);
	}

	//printf(">>> 7\n");
	//printf("Result: %.8f\n", sim);

	RETURN_DOUBLE(sim);
}
/* }}} ph_video_dist */

#endif /* HAVE_VIDEO_HASH */


#if HAVE_AUDIO_HASH

/* {{{ proto float ph_audio_dist(resource ph_audio_hash h1,resource ph_audio_hash h2, 
		int block_size=256, float thresh=0.30)
  pHash audio distance. */
PHP_FUNCTION(ph_audio_dist)
{
	zval * h1_res = NULL;
	ph_audio_hash * h1;

	zval * h2_res = NULL;
	ph_audio_hash * h2;

	PORTABLE_LONG_PARAMETER block_size = 256;
	double thresh = 0.30;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|ld", &h1_res, &h2_res, &block_size, &thresh) == 
		FAILURE) {
		return;
	}

	PORTABLE_ZEND_FETCH_RESOURCE(h1, ph_audio_hash *, &h1_res, -1, "ph_audio_hash", le_ph_audio_hash);
	PORTABLE_ZEND_FETCH_RESOURCE(h2, ph_audio_hash *, &h2_res, -1, "ph_audio_hash", le_ph_audio_hash);
	
	if(h1 && h2)
	{
		int Nc;
		double *cs = ph_audio_distance_ber(h1->hash, h1->len, h2->hash, h2->len, 
				thresh, block_size, Nc);
		if(cs)
		{
			double max_cs = *std::max_element(cs, cs + Nc);
			free(cs);
			RETURN_DOUBLE(max_cs);
		}
		else
			RETURN_DOUBLE(-1);
	}
	else
		RETURN_DOUBLE(-1);
}
/* }}} ph_audio_dist */


/* {{{ proto float ph_audio_dist2(string h1, string h2,
		int block_size=256, float thresh=0.30)
  pHash audio distance. */
PHP_FUNCTION(ph_audio_dist2)
{
	const char* str_h1 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH h1_len = 0;

	const char* str_h2 = NULL;
	PORTABLE_STRING_PARAMETER_LENGTH h2_len = 0;

	PORTABLE_LONG_PARAMETER block_size = 256;
	double thresh = 0.30;

	// parse functions parameters
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|ld", &str_h1, &h1_len, &str_h2, &h2_len, 
		&block_size, &thresh) == FAILURE) {
		return;
	}
	
	// validate hash string lengths
	if(h1_len == 0 || h2_len == 0 || h1_len % (sizeof(uint32_t) * 2) != 0 || h2_len % (sizeof(uint32_t) * 2) != 0) {
		RETURN_DOUBLE(-1);
	}
	//printf(">>> 1\n");
	
	// allocate buffer for hash1
	emem_ptr<unsigned char> bh1(emalloc(h1_len / 2));
	if(!bh1) {
		RETURN_DOUBLE(-2);
	}
	//printf(">>> 2\n");

	// decode string hash #1 into the binary form
	if(!decode_hex_string(bh1.m_p, str_h1)) {
		RETURN_DOUBLE(-3);
	}
	//printf(">>> 3\n");
	
	// allocate buffer for hash2
	emem_ptr<unsigned char> bh2(emalloc(h2_len / 2));
	if(!bh2) {
		RETURN_DOUBLE(-4);
	}
	//printf(">>> 4\n");

	// decode string hash #1 into the binary form
	if(!decode_hex_string(bh2.m_p, str_h2)) {
		RETURN_DOUBLE(-5);
	}
	//printf(">>> 5\n");

	int Nc;
	double *cs = ph_audio_distance_ber(
		reinterpret_cast<uint32_t*>(bh1.m_p), h1_len, 
		reinterpret_cast<uint32_t*>(bh2.m_p), h2_len,
		thresh, block_size, Nc);
	if(cs)
	{
		double max_cs = *std::max_element(cs, cs + Nc);
		free(cs);
		RETURN_DOUBLE(max_cs);
	}
	else
		RETURN_DOUBLE(-6);
}
/* }}} ph_audio_dist2 */


#endif /* HAVE_AUDIO_HASH */

/* {{{ proto array ph_compare_text_hashes(resource ph_txt_hash h1,resource ph_txt_hash h2)
  pHash text distance. */
PHP_FUNCTION(ph_compare_text_hashes)
{
	zval * h1_res = NULL;
	ph_text_hash * h1;

	zval * h2_res = NULL;
	ph_text_hash * h2;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &h1_res, &h2_res) == FAILURE) {
		return;
	}

	PORTABLE_ZEND_FETCH_RESOURCE(h1, ph_text_hash *, &h1_res, -1, "ph_txt_hash", le_ph_txt_hash);
	PORTABLE_ZEND_FETCH_RESOURCE(h2, ph_text_hash *, &h2_res, -1, "ph_txt_hash", le_ph_txt_hash);

	array_init(return_value);

	if(h1 && h2)
	{
		int count = 0;
		TxtMatch *m = ph_compare_text_hashes(h1->p, h1->count, h2->p, h2->count, &count);
		if(m)
		{
			for(int i = 0; i < count; ++i)
			{
				PORTABLE_DECLARE_ZVAL(array);
				PORTABLE_MAKE_STD_ZVAL(array);
				array_init(array);
				add_assoc_long(array, "begin", m[i].first_index);
				add_assoc_long(array, "end", m[i].second_index);
				add_assoc_long(array, "length", m[i].length);
				add_next_index_zval(return_value, array);
			}
			free(m);
		}
		else
			RETURN_FALSE;

	}
	else
		RETURN_FALSE;
}
/* }}} ph_compare_text_hashes */

#endif /* HAVE_PHASH */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
