/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "Zend/zend_interfaces.h"
#include "ext/standard/info.h"
#include "ext/pcre/php_pcre.h"
#include "php_bmvc.h"

/* If you declare any globals in php_bmvc.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(bmvc)
*/

/* True global resources - no need for thread safety here */
static int le_bmvc;
static zend_class_entry* bmvc_app_entry_ptr;
static zend_class_entry* bmvc_router_entry_ptr;
static zend_class_entry* bmvc_route_entry_ptr;
static zend_class_entry* bmvc_controller_entry_ptr;
/* {{{ bmvc_functions[]
 *
 * Every user visible function must have an entry in bmvc_functions[].
 */
const zend_function_entry bmvc_functions[] = {
	PHP_FE_END	/* Must be the last line in bmvc_functions[] */
};

const zend_function_entry bmvc_app_class_methods[] = {
	PHP_ME(BMvcApp, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcApp, run, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

const zend_function_entry bmvc_router_class_methods[] = {
	PHP_ME(BMvcRouter, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRouter, addRoute, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRouter, getMatchingRoute, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

const zend_function_entry bmvc_route_class_methods[] = {
	PHP_ME(BMvcRoute, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRoute, isMatch, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRoute, getController, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRoute, getAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(BMvcRoute, getParam, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

const zend_function_entry bmvc_controller_class_methods[] = {
	PHP_ME(BMvcController, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/* }}} */

/* {{{ bmvc_module_entry
 */
zend_module_entry bmvc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"bmvc",
	bmvc_functions,
	PHP_MINIT(bmvc),
	PHP_MSHUTDOWN(bmvc),
	PHP_RINIT(bmvc),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(bmvc),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(bmvc),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BMVC
ZEND_GET_MODULE(bmvc)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("bmvc.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_bmvc_globals, bmvc_globals)
    STD_PHP_INI_ENTRY("bmvc.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_bmvc_globals, bmvc_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_bmvc_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_bmvc_init_globals(zend_bmvc_globals *bmvc_globals)
{
	bmvc_globals->global_value = 0;
	bmvc_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(bmvc)
{
	zend_class_entry bmvc_app_entry;
	INIT_CLASS_ENTRY(bmvc_app_entry, "BMvcApp", bmvc_app_class_methods);
	bmvc_app_entry_ptr = zend_register_internal_class(&bmvc_app_entry TSRMLS_CC);

	zend_declare_property_null(bmvc_app_entry_ptr, ZEND_STRL("_config"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(bmvc_app_entry_ptr, ZEND_STRL("_router"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_class_entry bmvc_route_entry;
	INIT_CLASS_ENTRY(bmvc_route_entry, "BMvcRoute", bmvc_route_class_methods);
	bmvc_route_entry_ptr = zend_register_internal_class(&bmvc_route_entry TSRMLS_CC);

	zend_declare_property_null(bmvc_route_entry_ptr, ZEND_STRL("_pattern"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(bmvc_route_entry_ptr, ZEND_STRL("_controller_name"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(bmvc_route_entry_ptr, ZEND_STRL("_action_name"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(bmvc_route_entry_ptr, ZEND_STRL("_param"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(bmvc_route_entry_ptr, ZEND_STRL("_map"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_class_entry bmvc_router_entry;
	INIT_CLASS_ENTRY(bmvc_router_entry, "BMvcRouter", bmvc_router_class_methods);
	bmvc_router_entry_ptr = zend_register_internal_class(&bmvc_router_entry TSRMLS_CC);

	zend_declare_property_null(bmvc_router_entry_ptr, ZEND_STRL("_routes"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_class_entry bmvc_controller_entry;
	INIT_CLASS_ENTRY(bmvc_controller_entry, "BMvcController", bmvc_controller_class_methods);
	bmvc_controller_entry_ptr = zend_register_internal_class(&bmvc_controller_entry TSRMLS_CC);

	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(bmvc)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(bmvc)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(bmvc)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(bmvc)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "bmvc support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_bmvc_compiled(string arg)
   Return a string to confirm that the module is compiled in */
zval* bmvc_router_new(zval* this_ptr TSRMLS_DC) {
	zval* routes;
	zval* instance;

	if (this_ptr) {
		instance = this_ptr;
	} else {
		MAKE_STD_ZVAL(instance);
		object_init_ex(instance, bmvc_router_entry_ptr);
	}

	MAKE_STD_ZVAL(routes);
	array_init(routes);
	zend_update_property(bmvc_router_entry_ptr, instance, ZEND_STRL("_routes"), routes TSRMLS_CC);
	zval_ptr_dtor(&routes);
	
	return instance;
}

PHP_METHOD(BMvcApp, __construct) {
	zval* self = getThis();
	zval* config = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &config) == FAILURE) {
		return;
	}

	if (!config) {
		zend_update_property_null(bmvc_app_entry_ptr, self, ZEND_STRL("_config") TSRMLS_CC);
	} else {
		zend_update_property(bmvc_app_entry_ptr, self, ZEND_STRL("_config"), config TSRMLS_CC);
	}

	zval* router;
	router = bmvc_router_new(NULL TSRMLS_CC);
	zend_update_property(bmvc_app_entry_ptr, self, ZEND_STRL("_router"), router TSRMLS_CC);
	
	zval_ptr_dtor(&router);

	RETURN_ZVAL(self, 1, 0);
}

PHP_METHOD(BMvcApp, run) {
	char* url;
	int url_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_len) == FAILURE) {
		RETURN_FALSE;
	}

	zval* self = getThis();	
	zval* router = zend_read_property(bmvc_app_entry_ptr, self, ZEND_STRL("_router"), 1 TSRMLS_CC);

	zval* route;
	zval* zurl;
	MAKE_STD_ZVAL(zurl);
	ZVAL_STRING(zurl, url, url_len);
	zend_call_method_with_1_params(&router, Z_OBJCE_P(router), NULL, "getmatchingroute", &route, zurl);
	if (Z_TYPE_P(route) == IS_OBJECT) {
		zval* zcontroller_name;
		zval* zaction_name;
		zval* zparam;

		zend_call_method_with_0_params(&route, Z_OBJCE_P(route), NULL, "getcontroller", &zcontroller_name);
		zend_call_method_with_0_params(&route, Z_OBJCE_P(route), NULL, "getaction", &zaction_name);
		zend_call_method_with_0_params(&route, Z_OBJCE_P(route), NULL, "getparam", &zparam);

		char* controller_name;
		char* action_name;
		controller_name = Z_STRVAL_P(zcontroller_name);
		action_name = Z_STRVAL_P(zaction_name);
		
		char* class = NULL;
		char* class_lowercase = NULL;
		int class_len = 0;	
		zend_class_entry** ce = NULL;
		
		class_len = spprintf(&class, 0, "%s%s", controller_name, "Controller");
		class_lowercase = zend_str_tolower_dup(class, class_len);


		if (zend_hash_find(EG(class_table), class_lowercase, class_len + 1, (void*)&ce) != SUCCESS) {
			zval* ret;
			zval* zfunc_name;
			zval* zargs[1];

			MAKE_STD_ZVAL(ret);
			MAKE_STD_ZVAL(zfunc_name);
			ZVAL_STRING(zfunc_name, "spl_autoload_call", sizeof("spl_autoload_call"));
			MAKE_STD_ZVAL(zargs[0]);
			ZVAL_STRING(zargs[0], class, class_len);

			call_user_function(EG(function_table), NULL, zfunc_name, ret, 1, zargs TSRMLS_CC);
			zval_ptr_dtor(&zargs[0]);
			zval_ptr_dtor(&ret);
		}

		int rc = 0;
		if (zend_hash_find(EG(class_table), class_lowercase, class_len + 1, (void*)&ce) == SUCCESS) {
			zval* icontroller;
			MAKE_STD_ZVAL(icontroller);
			object_init_ex(icontroller, *ce);
			
			char* method = NULL;
			int method_len = 0;
			method_len = spprintf(&method, 0, "%s%s", action_name, "Action");
			char* method_lowercase = zend_str_tolower_dup(method, method_len);
			zend_function* fptr;
			if (zend_hash_find((&(*ce)->function_table), method_lowercase, method_len + 1, (void**)&fptr) == SUCCESS) {
				zval* ret;
				zval* zmethod;
				zval* zargs[1];

				MAKE_STD_ZVAL(ret);
				MAKE_STD_ZVAL(zmethod);
				ZVAL_STRING(zmethod, method_lowercase, method_len);

				zval_add_ref(&zparam);
				zargs[0] = zparam;

				call_user_function(&(*ce)->function_table, &icontroller, zmethod, ret, 1, zargs TSRMLS_CC);
				zval_ptr_dtor(&ret);
				rc = 1;
			}
			efree(method);
			efree(method_lowercase);
		}
		
		efree(class);
		efree(class_lowercase);
		zval_ptr_dtor(&route);
		zval_ptr_dtor(&zcontroller_name);
		zval_ptr_dtor(&zaction_name);

		if (rc == 1) {
			RETURN_TRUE;
		} else {
			RETURN_FALSE;
		}
	}
	RETURN_FALSE;
}

PHP_METHOD(BMvcRoute, __construct) {
	char* pattern;
	int pattern_len;
	char* controller_name;
	int controller_name_len;
	char* action_name;
	int action_name_len;

	zval* map;
	MAKE_STD_ZVAL(map);
	array_init(map);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|a", &pattern, &pattern_len, &controller_name, &controller_name_len, &action_name, &action_name_len, &map) == FAILURE) {
		RETURN_FALSE;
	}
	zval* self = getThis();

	zval* param;	
	MAKE_STD_ZVAL(param);
	array_init(param);

	zval* zpattern;
	MAKE_STD_ZVAL(zpattern);
	ZVAL_STRING(zpattern, pattern, pattern_len);

	zval* zcontroller_name;
	MAKE_STD_ZVAL(zcontroller_name);
	ZVAL_STRING(zcontroller_name, controller_name, controller_name_len);

	zval* zaction_name;
	MAKE_STD_ZVAL(zaction_name);
	ZVAL_STRING(zaction_name, action_name, action_name_len);

	zend_update_property(bmvc_route_entry_ptr, self, ZEND_STRL("_pattern"),  zpattern TSRMLS_CC);
	zend_update_property(bmvc_route_entry_ptr, self, ZEND_STRL("_controller_name"),  zcontroller_name TSRMLS_CC);
	zend_update_property(bmvc_route_entry_ptr, self, ZEND_STRL("_action_name"),  zaction_name TSRMLS_CC);
	zend_update_property(bmvc_route_entry_ptr, self, ZEND_STRL("_param"), param TSRMLS_CC);
	zend_update_property(bmvc_route_entry_ptr, self, ZEND_STRL("_map"), map TSRMLS_CC);
	
	zval_ptr_dtor(&param);
	RETURN_ZVAL(self, 1, 0);
}

PHP_METHOD(BMvcRoute, isMatch) {
	char* url;
	int url_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_len) == FAILURE) {
		return;
	}

	zval* self = getThis();
	zval* pattern = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_pattern"), 1 TSRMLS_CC);

	pcre_cache_entry* regexp;
	if ((regexp = pcre_get_compiled_regex_cache(Z_STRVAL_P(pattern), Z_STRLEN_P(pattern) TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	zval* matches;
	zval* subparts;

	MAKE_STD_ZVAL(matches);
	MAKE_STD_ZVAL(subparts);
	ZVAL_NULL(subparts);
	
	php_pcre_match_impl(regexp, url, url_len, matches, subparts, 0, 0, 0, 0 TSRMLS_CC);

	if (!Z_LVAL_P(matches)) {
		zval_ptr_dtor(&matches);
		zval_ptr_dtor(&subparts);
		RETURN_FALSE;
	}

	zval** name;
	zval** ppzval;
	char* key = NULL;
	int key_len = 0;
	long idx = 0;
	zval* map = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_map"), 1 TSRMLS_CC);
	zval* param = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_param"), 1 TSRMLS_CC);

	HashTable* htb;
	htb = Z_ARRVAL_P(subparts);

	for (zend_hash_internal_pointer_reset(htb); 
			zend_hash_has_more_elements(htb) == SUCCESS; 
			zend_hash_move_forward(htb)) {
		if (zend_hash_get_current_data(htb, (void**)&ppzval) == FAILURE) {
			continue;
		}
		
		if (zend_hash_get_current_key_ex(htb, &key, &key_len, &idx, 0, NULL) == HASH_KEY_IS_LONG) {
			if (zend_hash_index_find(Z_ARRVAL_P(map), idx, (void**)&name) == SUCCESS) {
				Z_ADDREF_P(*ppzval);
				zend_hash_update(Z_ARRVAL_P(param), Z_STRVAL_PP(name), Z_STRLEN_PP(name) + 1, (void**)ppzval, sizeof(zval*), NULL);
			}
		} else {
			Z_ADDREF_P(*ppzval);
			zend_hash_update(Z_ARRVAL_P(param), key, key_len, (void**)ppzval, sizeof(zval*), NULL);
		}
	}
	zval_ptr_dtor(&matches);
	zval_ptr_dtor(&subparts);
	RETURN_TRUE;
}

PHP_METHOD(BMvcRoute, getController) {
	zval* self = getThis();
	zval* zcontroller_name = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_controller_name"), 1 TSRMLS_CC);
	RETURN_STRING(Z_STRVAL_P(zcontroller_name), Z_STRLEN_P(zcontroller_name));
}

PHP_METHOD(BMvcRoute, getAction) {
	zval* self = getThis();
	zval* zaction_name = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_action_name"), 1 TSRMLS_CC);
	RETURN_STRING(Z_STRVAL_P(zaction_name), Z_STRLEN_P(zaction_name));
}

PHP_METHOD(BMvcRoute, getParam) {
	zval* self = getThis();
	zval* param = zend_read_property(bmvc_route_entry_ptr, self, ZEND_STRL("_param"), 1 TSRMLS_CC);
	RETURN_ZVAL(param, 1, 0);
}

PHP_METHOD(BMvcRouter, __construct) {
	bmvc_router_new(getThis() TSRMLS_CC);
}

PHP_METHOD(BMvcRouter, addRoute) {
	char* name;
	int name_len;
	zval* route;
	zval* routes;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &route) == FAILURE) {
		return;
	}

	if (!name_len) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(route) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(route), bmvc_route_entry_ptr TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expects a %s instance", bmvc_route_entry_ptr->name);
		RETURN_FALSE;
	}
	zval* self = getThis();

	routes = zend_read_property(bmvc_router_entry_ptr, self, ZEND_STRL("_routes"), 1 TSRMLS_CC);

	Z_ADDREF_P(route);
	zend_hash_update(Z_ARRVAL_P(routes), name, name_len + 1, (void**) &route, sizeof(zval*), NULL);

	RETURN_ZVAL(self, 1, 0);
}

PHP_METHOD(BMvcRouter, getMatchingRoute) {
	char* url;
	int url_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_len) == FAILURE) {
		return;
	}

	zval* self = getThis();
	zval* routes;
	zval** route;
	zval* ret;
	HashTable* htb;

	zval* zurl;
	MAKE_STD_ZVAL(zurl);
	ZVAL_STRING(zurl, url, url_len);

	routes = zend_read_property(bmvc_router_entry_ptr, self,  ZEND_STRL("_routes"), 1 TSRMLS_CC);
	htb = Z_ARRVAL_P(routes);
	for (zend_hash_internal_pointer_end(htb); 
			zend_hash_has_more_elements(htb) == SUCCESS; 
			zend_hash_move_backwards(htb)) {

		if (zend_hash_get_current_data(htb, (void**)&route) == FAILURE) {
			continue;
		}
	
		zend_call_method_with_1_params(route, Z_OBJCE_PP(route), NULL, "ismatch", &ret, zurl);
		
		if (IS_BOOL != Z_TYPE_P(ret) || !Z_BVAL_P(ret)) {
			zval_ptr_dtor(&ret);
			continue;
		}

		zval_ptr_dtor(&ret);

		RETURN_ZVAL(*route, 1, 0);
	}
	RETURN_FALSE;
}

PHP_METHOD(BMvcController, __construct) {

}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
