/* 
	Created by Ivan Pizhenko <ivanp2015@users.noreply.github.com>
	https://github.com/ivanp2015/pHash-0.9.6
	Public domain, GPL3 license, https://www.gnu.org/licenses/gpl-3.0.en.html

	Based on the recommendations from the following materials:
	1. https://github.com/beberlei/whitewashing.de/blob/master/drafts/porting_extension_to_php7.rst
	2. http://www.sklar.com/software/php/2015/03/23/converting-a-php-extension-to-php-7/
	3. https://wiki.php.net/phpng-upgrading
*/

#ifndef PHASH_PHP__ZEND_API_PORTS_H__
#define PHASH_PHP__ZEND_API_PORTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#if PHP_MAJOR_VERSION < 7

#define PORTABLE_ZVAL_STRING(str, len) ZVAL_STRING(str, len, 0)
#define PORTABLE_RETURN_STRING(str) RETURN_STRING(str, 0)
#define PORTABLE_ZEND_RESOURCE zend_rsrc_list_entry
#define PORTABLE_ZEND_REGISTER_RESOURCE(callresult, return_value, result, le_result) \
	callresult = ZEND_REGISTER_RESOURCE(return_value, result, le_result)
#define PORTABLE_ZEND_FETCH_RESOURCE ZEND_FETCH_RESOURCE
#define PORTABLE_DECLARE_RETURN_RES_ID long return_res_id = -1;

#define PORTABLE_DECLARE_ZVAL(name) zval * name
#define PORTABLE_ALLOC_INIT_ZVAL(name) ALLOC_INIT_ZVAL(name)
#define PORTABLE_MAKE_STD_ZVAL(name) MAKE_STD_ZVAL(name)
#define portable_hp_ptr_dtor(val) zval_ptr_dtor(&val);
#define PORTABLE_LONG_PARAMETER long
#define PORTABLE_STRING_PARAMETER_LENGTH int
	
#else

#define PORTABLE_ZVAL_STRING(str, len) ZVAL_STRING(str, len)
#define PORTABLE_RETURN_STRING(str) RETURN_STRING(str)
#define PORTABLE_ZEND_RESOURCE zend_resource
#define PORTABLE_ZEND_REGISTER_RESOURCE(callresult, return_value, result, le_result) \
	RETURN_RES(zend_register_resource(result, le_result))
#define PORTABLE_ZEND_FETCH_RESOURCE(rsrc, rsrc_type, rsrc_id, default_rsrc_id, resource_type_name, resource_type) \
	rsrc = (rsrc_type) zend_fetch_resource_ex(*(rsrc_id), resource_type_name, resource_type);
#define PORTABLE_DECLARE_RETURN_RES_ID

#define PORTABLE_DECLARE_ZVAL(name) zval name ## _v; zval * name = &name ## _v
#define PORTABLE_ALLOC_INIT_ZVAL(name) ZVAL_NULL(name)
#define PORTABLE_MAKE_STD_ZVAL(name) ZVAL_NULL(name)
#define portable_hp_ptr_dtor(val) zval_ptr_dtor(val);
#define PORTABLE_LONG_PARAMETER zend_long
#define PORTABLE_STRING_PARAMETER_LENGTH size_t

#endif
	
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
