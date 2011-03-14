/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2010, Radu M
 *
 * Radu Maierean <radu dot maierean at g-mail>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * Please follow coding guidelines 
 * http://svn.digium.com/view/asterisk/trunk/doc/CODING-GUIDELINES
 */

/*! \file
 *
 * \brief JSONPRETTY() formats a json document for easy read
 * \brief JSONCOMPRESS() formats a json document for minimal footprint
 * \brief JSONELEMENT() get element at path from a json document
 * \brief jsonvariables sets a list of variables from a single-level json document
 * \brief jsonadd add an element at path in a json document
 * \brief jsonset set value of an element at path in a json document
 * \brief jsondelete delete element at path from a json document
 *
 * \author\verbatim Radu Maierean <radu dot maierean at gmail> \endverbatim
 * 
 * \ingroup applications
 */

/*** MODULEINFO
	<defaultenabled>yes</defaultenabled>
 ***/

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 200656 $")

#include "asterisk/file.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/app.h"
#include "asterisk/utils.h"
#include "asterisk/cJSON.h"

/*** DOCUMENTATION
	<function name="JSONPRETTY" language="en_US">
		<synopsis>
			nicely formats a json string for printing
		</synopsis>	
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
		</syntax>
		<description>
			<para>nicely formats a json string for printing. cosmetic functionality only.</para>
		</description>
	</function>
	<function name="JSONCOMPRESS" language="en_US">
		<synopsis>
			formats a json string for minimum footprint
		</synopsis>	
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
		</syntax>
		<description>
			<para>formats a json string for minimum footprint (eliminates all unnecessary 
			characters). cosmetic functionality only.</para>
		</description>
	</function>
	<function name="JSONELEMENT" language="en_US">
		<synopsis>
			gets the value of an element at a given path in a json document
		</synopsis>	
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
			<parameter name="path" required="true">
				<para>path to where the element were looking for (like "/path/to/element", or 
				"/path/to/element/3" to identify the element with index 3 in an array)</para>
			</parameter>
		</syntax>
		<description>
			<para>returns the value of an element at the given path. the element type is returned 
			in the dialplan variable JSONTYPE.</para>
		</description>
	</function>
	<application name="jsonvariables" language="en_US">
		<synopsis>
			parse a single-level json structure (key-value pairs) as dialplan variables
		</synopsis>
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
		</syntax>
		<description>
			<para>considers that the json string offered as input is a list of key-value pairs; 
			associates each key with an asterisk variable name, and sets the value for it.</para>
			<para>depending on the type of the json variable, the values are:</para>
			<para>   True, False: 1, 0</para>
			<para>   NULL: (resulting variable contains an empty string)</para>
			<para>   Number, String: the number or the string</para>
			<para>   Array: !array! (literal)</para>
			<para>   Object: string, the json representation of the underlying object</para>
			<para>note that this is mainly intended for simple key-value lists; if you have 
			variables that are arrays or objects, things may get screwed up because of the 
			separators and braces...</para>
		</description>
		<see-also>
			<ref type="application">jsonelement</ref>
		</see-also>
	</application>
	<application name="jsonadd" language="en_US">
		<synopsis>
			adds a new element in a json document
		</synopsis>
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
			<parameter name="path" required="true">
				<para>path to where the element will be added (like "/path/to/element", or 
				"/path/to/element/3" to identify the element with index 3 in an array); if empty, 
				the element becomes the root element</para>
			</parameter>
			<parameter name="type" required="true">
				<para>type of element to be added: bool, null, number, string or array</para>
			</parameter>
			<parameter name="name" required="true">
				<para>name of element to be added; if adding to an array element, the new element is 
				appended and the name is ignored</para>
			</parameter>
			<parameter name="value" required="true">
				<para>the actual value; ignored if adding null-type elements; for bool type elements 
				any of the following 0, n, no, f, false or empty string (case insensitive) are 
				considered false, anything else is considered true</para>
			</parameter>
		</syntax>
		<description>
			<para>the first parameter is interpreted as a variable name, and its contents is 
			considered to be a json document. the json document is parsed and the path is followed 
			to the insertion point. a new element of the given type, with the given name and value 
			is created, then added to the element at the end of the path (or appended, if the element 
			at the end of the path is an array). the contents of the json document variable is 
			updated to reflect the element added.</para>
		</description>
		<see-also>
			<ref type="application">jsonset</ref>
			<ref type="application">jsondelete</ref>
		</see-also>
	</application>
	<application name="jsonset" language="en_US">
		<synopsis>
			changes the value of an element in a json document
		</synopsis>
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
			<parameter name="path" required="true">
				<para>path to the element whose value will change (like "/path/to/element", 
				or "/path/to/element/3" to identify the element with index 3 in an array)</para>
			</parameter>
			<parameter name="value" required="true">
				<para>the new value to be set; must be of the same type as the current value. for 
				bool type elements any of the following 0, n, no, f, false or empty string (case 
				insensitive) are considered false, anything else is considered true</para>
			</parameter>
		</syntax>
		<description>
			<para>the first parameter is interpreted as a variable name, and its contents is 
			considered to be a json document. the json document is parsed and the path is followed 
			to the update point. the value of the element at the update point is changed to the given 
			value. note that the new value will be the same type as the replaced value: for example 
			if you try to replace a numeric value like 123 with a string value like abc, the 
			variable type will be preserved and you will end up with a 0 instead of abc. you may 
			change the value of boolean elements, numeric and string elements, but since the type is 
			preserved, you cannot change the value of null or array elements. (you may change the 
			value of an element insida an array though.) you may also change the value of an object 
			element by using a json-proper value, but you have to be very careful with escaping the 
			commas in this case... the contents of the json document variable is updated to reflect 
			the change.</para>
		</description>
		<see-also>
			<ref type="application">jsonadd</ref>
			<ref type="application">jsondelete</ref>
		</see-also>
	</application>
	<application name="jsondelete" language="en_US">
		<synopsis>
			removes an element from a json document
		</synopsis>
		<syntax>
			<parameter name="jsonvarname" required="true">
				<para>the name (not the contents!) of a variable that contains the json struct</para>
			</parameter>
			<parameter name="path" required="true">
				<para>path to the element will be added (like "/path/to/element", or 
				"/path/to/element/3" to identify the element with index 3 in an array); if empty, 
				the element becomes the root element</para>
			</parameter>
		</syntax>
		<description>
			<para>the first parameter is interpreted as a variable name, and its contents is 
			considered to be a json document. the json document is parsed and the path is followed 
			to the element that needs to be deleted. it is either pulled from the array, or dropped 
			from the list of subelements.the contents of the json document variable is updated to 
			reflect the change.</para>
		</description>
		<see-also>
			<ref type="application">jsonadd</ref>
			<ref type="application">jsonset</ref>
		</see-also>
	</application>
 ***/

static const char *app_jsonvariables = "jsonvariables";
static const char *app_jsonadd = "jsonadd";
static const char *app_jsonset = "jsonset";
static const char *app_jsondelete = "jsondelete";

#define MAX_ASTERISK_VARLEN    4096

#define ASTJSON_OK             0
#define ASTJSON_UNDECIDED      1
#define ASTJSON_ARG_NEEDED     2
#define ASTJSON_PARSE_ERROR    3
#define ASTJSON_NOTFOUND       4
#define ASTJSON_INVALID_TYPE   5
#define ASTJSON_ADD_FAILED     6
#define ASTJSON_SET_FAILED     7
#define ASTJSON_DELETE_FAILED  8

static void json_set_operation_result(struct ast_channel *chan, int result) {
	char *numresult;
	ast_asprintf(&numresult, "%d", result);
	pbx_builtin_setvar_helper(chan, "JSONRESULT", numresult);
}

static int jsonpretty_exec(struct ast_channel *chan, 
	const char *cmd, char *parse, char *buffer, size_t buflen
) {
// nicely format the contents of a varable that contains json

	buffer[0] = 0;

	// parse the function arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
	);
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "jsonpretty requires arguments (json)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	AST_STANDARD_APP_ARGS(args, parse);
	if (ast_strlen_zero(args.json)) {
		ast_log(LOG_WARNING, "a valid asterisk variable name is required\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	// parse json
	cJSON *doc = cJSON_Parse(pbx_builtin_getvar_helper(chan, args.json));
	if (!doc) {
		ast_log(LOG_WARNING, "source json parsing error\n");
		json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
		return 0;
	}
	char *pretty = cJSON_Print(doc);
	ast_copy_string(buffer, pretty, buflen);
	cJSON_Delete(doc);
	free(pretty);
	json_set_operation_result(chan, ASTJSON_OK);
	return 0;

}

static int jsoncompress_exec(struct ast_channel *chan, 
	const char *cmd, char *parse, char *buffer, size_t buflen
) {
// return a json string by stripping the unneeded characters (smallest footprint)

	buffer[0] = 0;

	// parse the function arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
	);
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "jsoncompress requires arguments (json)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	AST_STANDARD_APP_ARGS(args, parse);
	if (ast_strlen_zero(args.json)) {
		ast_log(LOG_WARNING, "a valid asterisk variable name is required\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	// parse json
	cJSON *doc = cJSON_Parse(pbx_builtin_getvar_helper(chan, args.json));
	if (!doc) {
		ast_log(LOG_WARNING, "source json parsing error\n");
		json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
		return 0;
	}
	char *unpretty = cJSON_Print(doc);
	ast_copy_string(buffer, unpretty, buflen);
	cJSON_Delete(doc);
	free(unpretty);
	json_set_operation_result(chan, ASTJSON_OK);
	return 0;

}

static int jsonelement_exec(struct ast_channel *chan, 
	const char *cmd, char *parse, char *buffer, size_t buflen
) {
// searches for a json element found based on a path (like "/path/to/element/3/value")  
//   and populates the element value and type with the contents of the element

	json_set_operation_result(chan, ASTJSON_UNDECIDED);
	buffer[0] = 0;

	// parse the function arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
		AST_APP_ARG(path);
	);
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "jsonelement requires arguments (json,path)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	AST_STANDARD_APP_ARGS(args, parse);
	if (ast_strlen_zero(args.json)) {
		ast_log(LOG_WARNING, "a valid asterisk variable name is required\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	if (ast_strlen_zero(args.path)) {
		ast_log(LOG_WARNING, "path is empty, returning full json\n");
		ast_copy_string(buffer, args.json, buflen);
		json_set_operation_result(chan, ASTJSON_OK);
		return 0;
	}
	// parse json
	cJSON *doc = cJSON_Parse(pbx_builtin_getvar_helper(chan, args.json));
	if (!doc) {
		ast_log(LOG_WARNING, "source json parsing error\n");
		json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
		return 0;
	}
	// go over the path (and eliminate heading and trailing slash in the process)
	char *thispath = ast_strdupa((char *)(args.path + ((args.path[0] == '/') ? 1 : 0)));
	if (thispath[strlen(thispath) - 1] == '/') thispath[strlen(thispath) - 1] = 0;
	cJSON *thisobject = doc; cJSON *nextobject = NULL;
	int ixarray;
	char *pathpiece = strsep(&thispath, "/");
	while (pathpiece) {
		// determine if we have an object with the given name or index
		if (sscanf(pathpiece, "%3d", &ixarray) == 1)
			nextobject = cJSON_GetArrayItem(thisobject, ixarray);
		else
			nextobject = cJSON_GetObjectItem(thisobject, pathpiece);
		if (!nextobject) {
			cJSON_Delete(doc);
			json_set_operation_result(chan, ASTJSON_NOTFOUND);
			return 0;
		}
		thisobject = nextobject;
		pathpiece = strsep(&thispath, "/");
	}
	// got to the end of our path, evaluate the object type and set the value
	char *type = NULL; char *value = NULL;
	int jtype = thisobject->type;
	switch (jtype) {
		case cJSON_False: type = ast_strdupa("bool"); ast_copy_string(buffer, "0", buflen); break;
		case cJSON_True: type = ast_strdupa("bool"); ast_copy_string(buffer, "1", buflen); break;
		case cJSON_NULL: type = ast_strdupa("null"); ast_copy_string(buffer, "", buflen); break;
		case cJSON_Number: 
			type = ast_strdupa("number"); 
			if (thisobject->valuedouble > thisobject->valueint)
				ast_asprintf(&value, "%f", thisobject->valuedouble); 
			else
				ast_asprintf(&value, "%d", thisobject->valueint); 
			ast_copy_string(buffer, value, buflen); 
			free(value);
			break;
		case cJSON_String: 
			type = ast_strdupa("string"); 
			ast_copy_string(buffer, thisobject->valuestring, buflen);
			break;
		case cJSON_Array: 
			type = ast_strdupa("array"); 
			ast_copy_string(buffer, cJSON_PrintUnformatted(thisobject), buflen); 
			break;
		case cJSON_Object: 
			type = ast_strdupa("object"); 
			ast_copy_string(buffer, cJSON_PrintUnformatted(thisobject), buflen); 
			break;
	}
	pbx_builtin_setvar_helper(chan, "JSONTYPE", type);
	json_set_operation_result(chan, ASTJSON_OK);
	cJSON_Delete(doc);
	return 0;

}

static int jsonvariables_exec(struct ast_channel *chan, const char *data) {
// considers that the json string offered as input is a list of key-value pairs; associates each
//   key with an asterisk variable name, and sets the value for it
// depending on the type of the json variable, the values are:
//   True, False: 1, 0
//   NULL: '' (resulting variable contains an empty string)
//   Number, String: the number or the string
//   Array: !array! (literal)
//   Object: string, the json representation of the underlying object
// note that this is mainly intended for simple key-value lists; if you have variables that are 
//   arrays or objects, things may get screwed up because of the separators and braces...

	json_set_operation_result(chan, ASTJSON_UNDECIDED);

	// parse the app arguments
	char *argcopy;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "jsonvariables requires arguments (jsonsource)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);
	if (ast_strlen_zero(args.json)) {
		ast_log(LOG_WARNING, "json string is empty\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	// parse json
	cJSON *doc = cJSON_Parse(args.json);
	if (!doc) {
		ast_log(LOG_WARNING, "source json parsing error\n");
		json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
		return 0;
	}
	// for each element
	cJSON *nvp = doc->child;
	char *num = NULL; char *eljson = NULL;
	while (nvp) {
		if (strlen(nvp->string)) {
			switch (nvp->type) {
				case cJSON_False: pbx_builtin_setvar_helper(chan, nvp->string, "0"); break;
				case cJSON_True: pbx_builtin_setvar_helper(chan, nvp->string, "1"); break;
				case cJSON_NULL: pbx_builtin_setvar_helper(chan, nvp->string, ""); break;
				case cJSON_Number:
					if (nvp->valuedouble > nvp->valueint)
						ast_asprintf(&num, "%f", nvp->valuedouble); 
					else
						ast_asprintf(&num, "%d", nvp->valueint); 
					pbx_builtin_setvar_helper(chan, nvp->string, num); 
					free(num);
					break;
				case cJSON_String: pbx_builtin_setvar_helper(chan, nvp->string, nvp->valuestring); break;
				case cJSON_Array: pbx_builtin_setvar_helper(chan, nvp->string, "!array!"); break;
				case cJSON_Object: 
					eljson = cJSON_PrintUnformatted(nvp);
					pbx_builtin_setvar_helper(chan, nvp->string, eljson); 
					free(eljson);
					break;
				default: 
					break;
			}
		}
		nvp = nvp->next;
	}
	cJSON_Delete(doc);
	json_set_operation_result(chan, ASTJSON_OK);
	return 0;

}

static int jsonadd_exec(struct ast_channel *chan, const char *data) {
// add an element of a certain type into a json structure, at the path indicated
// accepted types are bool, null, number, string or array
// the value parameter is ignored for null and array types; boolean false are represented by an 
//    empty string, 0, n, no, f or false (case insensitive) - anything else is considered true
// if the element at the path is an array, append to the array (in this case the name is ignored)
// rewrite the contents of the variable that contains the json source and set an error code variable

	json_set_operation_result(chan, ASTJSON_UNDECIDED);

	// parse the app arguments
	char *argcopy;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
		AST_APP_ARG(path);
		AST_APP_ARG(type);
		AST_APP_ARG(name);
		AST_APP_ARG(value);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "jsonadd requires arguments (jsonvarname,path,type,name,value)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);
	if (!ast_strlen_zero(args.json))
		ast_log(LOG_DEBUG, "getting json and setting result back into variable '%s'\n", args.json);
	else {
		ast_log(LOG_WARNING, "a valid dialplan variable name is needed as first argument\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	if (ast_strlen_zero(args.path))
		ast_log(LOG_WARNING, "path is empty, adding element to the root\n");

	// create the object to add
	cJSON *newobject = NULL;
	if (ast_strlen_zero(args.type)) {
		ast_log(LOG_WARNING, "an element type is needed (bool, null, number, string, array or object)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	} else {
		// get the element type
		if (strcasecmp(args.type, "bool") == 0)
			newobject = (
				(strlen(args.value) == 0) || (strcasecmp(args.value, "0") == 0) ||
				(strcasecmp(args.value, "no") == 0) || (strcasecmp(args.value, "n") == 0) ||
				(strcasecmp(args.value, "false") == 0) || (strcasecmp(args.value, "f") == 0) 
			) ? cJSON_CreateFalse() : cJSON_CreateTrue();
		else if (strcasecmp(args.type, "null") == 0)
			newobject = cJSON_CreateNull();
		else if (strcasecmp(args.type, "number") == 0)
			newobject = cJSON_CreateNumber((double)atof(args.value));
		else if (strcasecmp(args.type, "string") == 0)
			newobject = cJSON_CreateString(args.value);
		else if (strcasecmp(args.type, "array") == 0)
			newobject = cJSON_CreateArray();
		else {
			ast_log(LOG_WARNING, "invalid element type '%s'; need bool, null, number, string or array\n", args.type);
			json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
			return 0;
		}
	}
	// parse source
	cJSON *doc;
	const char *source = pbx_builtin_getvar_helper(chan, args.json);
	if (strlen(source)) {
		doc = cJSON_Parse(source);
		if (!doc) {
			ast_log(LOG_WARNING, "source json parsing error\n");
			cJSON_Delete(newobject);
			json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
			return 0;
		}
	} else {
		// empty source, which means the object we created is the json itself
		char *jsonresult = cJSON_PrintUnformatted(newobject);
		pbx_builtin_setvar_helper(chan, args.json, jsonresult);
		free(jsonresult);
		cJSON_Delete(newobject);
		json_set_operation_result(chan, ASTJSON_OK);
		return 0;
	}
	// go over the path
	int ret = ASTJSON_NOTFOUND;
	char *thispath = ast_strdupa((char *)(args.path + ((args.path[0] == '/') ? 1 : 0)));
	if (thispath[strlen(thispath) - 1] == '/') thispath[strlen(thispath) - 1] = 0;
	if (strlen(thispath) == 0) {
		// no path - add to the json root
		switch (doc->type) {
		case cJSON_Array:
			cJSON_AddItemToArray(doc, newobject);
			ret = ASTJSON_OK;
			break;
		case cJSON_Object:
			cJSON_AddItemToObject(doc, args.name, newobject);
			ret = ASTJSON_OK;
			break;
		default:
			ret = ASTJSON_ADD_FAILED;
			break;
		}
	} else {
		cJSON *thisobject = doc; cJSON *nextobject = NULL; 
		int ixarray;
		char *pathpiece = strsep(&thispath, "/");
		while (pathpiece) {
			// determine if we have an object with the given name or index
			if (sscanf(pathpiece, "%3d", &ixarray) == 1)
				nextobject = cJSON_GetArrayItem(thisobject, ixarray);
			else
				nextobject = cJSON_GetObjectItem(thisobject, pathpiece);
			if (nextobject == NULL) break; // path element not found
			pathpiece = strsep(&thispath, "/");
			if (pathpiece == NULL) {
				// done going down the path, add object here
				switch (nextobject->type) {
				case cJSON_Array:
					cJSON_AddItemToArray(nextobject, newobject);
					ret = ASTJSON_OK;
					break;
				case cJSON_Object:
					cJSON_AddItemToObject(nextobject, args.name, newobject);
					ret = ASTJSON_OK;
					break;
				default:
					ret = ASTJSON_ADD_FAILED;
					break;
				}
				break;
			} else
				thisobject = nextobject;
		}
	}
	// regenerate the source json
	char *jsonresult = cJSON_PrintUnformatted(doc);
	if (ret == ASTJSON_OK)
		pbx_builtin_setvar_helper(chan, args.json, jsonresult);
	// cleanup the mess and let's get outta here
	free(jsonresult);
	cJSON_Delete(doc);
	json_set_operation_result(chan, ret);
	return 0;

}

static int jsonset_exec(struct ast_channel *chan, const char *data) {
// sets the value of the element at the path indicated (like "/path/to/element/3/value") 
// the new value must be of the same type as the existing element. you cannot set the value of
//    existing null elements, or array elements: you can only delete or add them (then for the 
//    arrays you would need to add the elements with repeated "add" operations)
// regarding boolean values to be set, false is represented by an empty string, 0, n, no, f or false 
//    (case insensitive) - anything else is considered true
// rewrite the contents of the variable that contains the json source and set an error code variable

	json_set_operation_result(chan, ASTJSON_UNDECIDED);

	// parse the app arguments
	char *argcopy;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(json);
		AST_APP_ARG(path);
		AST_APP_ARG(value);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "jsonset requires arguments (jsonvarname,path,value)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);
	if (!ast_strlen_zero(args.json))
		ast_log(LOG_DEBUG, "getting json and setting result back into variable '%s'\n", args.json);
	else {
		ast_log(LOG_WARNING, "a valid dialplan variable name is needed as first argument\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	if (ast_strlen_zero(args.path))
		ast_log(LOG_WARNING, "path is empty, adding element to the root\n");

	// parse source
	cJSON *doc;
	const char *source = pbx_builtin_getvar_helper(chan, args.json);
	if (strlen(source) == 0) {
		ast_log(LOG_WARNING, "source json is empty\n");
		json_set_operation_result(chan, ASTJSON_INVALID_TYPE);
		return 0;
	}
	doc = cJSON_Parse(source);
	if (!doc) {
		ast_log(LOG_WARNING, "source json parsing error\n");
		json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
		return 0;
	}
	// go over the path
	int ret = ASTJSON_NOTFOUND;
	char *thispath = ast_strdupa((char *)(args.path + ((args.path[0] == '/') ? 1 : 0)));
	if (thispath[strlen(thispath) - 1] == '/') thispath[strlen(thispath) - 1] = 0;
	if (strlen(thispath) == 0) {
		ast_log(LOG_WARNING, "invalid path to the object we want to set\n");
		json_set_operation_result(chan, ASTJSON_NOTFOUND);
		return 0;
	}
	cJSON *thisobject = doc; cJSON *nextobject = NULL; cJSON *newobject = NULL; 
	int ixarray;
	char *pathpiece = strsep(&thispath, "/");
	while (pathpiece) {
		// determine if we have an object with the given name or index
		if (sscanf(pathpiece, "%3d", &ixarray) == 1)
			nextobject = cJSON_GetArrayItem(thisobject, ixarray);
		else
			nextobject = cJSON_GetObjectItem(thisobject, pathpiece);
		if (nextobject == NULL) break; // path element not found
		pathpiece = strsep(&thispath, "/");
		if (pathpiece == NULL) {
			newobject = NULL;
			// done going down the path, this is the object we want to change the value for
			switch (nextobject->type) {
			case cJSON_False:
			case cJSON_True:
				newobject = (
					(strlen(args.value) == 0) || (strcasecmp(args.value, "0") == 0) ||
					(strcasecmp(args.value, "no") == 0) || (strcasecmp(args.value, "n") == 0) ||
					(strcasecmp(args.value, "false") == 0) || (strcasecmp(args.value, "f") == 0) 
				) ? cJSON_CreateFalse() : cJSON_CreateTrue();
				break;
			case cJSON_NULL:
				break;
			case cJSON_Number:
				newobject = cJSON_CreateNumber((double)atof(args.value));
				break;
			case cJSON_String:
				newobject = cJSON_CreateString(args.value);
				break;
			case cJSON_Array:
				break;
			case cJSON_Object:
				newobject = cJSON_Parse(args.value);
				break;
			default:
				break;
			}
			if (newobject) {
				// replace in the parent object with what we've just created here
				ret = ASTJSON_OK;
				if (thisobject->type == cJSON_Array)
					cJSON_ReplaceItemInArray(thisobject, ixarray, newobject);
				else if (thisobject->type == cJSON_Object)
					cJSON_ReplaceItemInObject(thisobject, nextobject->string, newobject);
				else
					ret = ASTJSON_SET_FAILED;
			} else
				ret = ASTJSON_INVALID_TYPE;
			break;
		} else
			thisobject = nextobject;
	}
	// regenerate the source json
	char *jsonresult = cJSON_PrintUnformatted(doc);
	if (ret == ASTJSON_OK)
		pbx_builtin_setvar_helper(chan, args.json, jsonresult);
	// cleanup the mess and let's get outta here
	free(jsonresult);
	cJSON_Delete(doc);
	json_set_operation_result(chan, ret);
	return 0;

}

static int jsondelete_exec(struct ast_channel *chan, const char *data) {
// delete a json element in a path (like "/path/to/element/3/value")  
// rewrite the contents of the variable that contains the json source and set an error code variable

	json_set_operation_result(chan, ASTJSON_UNDECIDED);

	// parse the app arguments
	char *argcopy;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(jsonvarname);
		AST_APP_ARG(path);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "jsondelete requires arguments (jsonvarname,path)\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);
	if (!ast_strlen_zero(args.jsonvarname))
		ast_log(LOG_DEBUG, "setting result into variable '%s'\n", args.jsonvarname);
	else {
		ast_log(LOG_WARNING, "a valid dialplan variable name is needed as first argument\n");
		json_set_operation_result(chan, ASTJSON_ARG_NEEDED);
		return 0;
	}
	if (ast_strlen_zero(args.path)) {
		ast_log(LOG_WARNING, "path is empty, will not delete the whole doc\n");
		json_set_operation_result(chan, ASTJSON_OK);
		return 0;
	}
	// parse source
	cJSON *doc;
	const char *source = pbx_builtin_getvar_helper(chan, args.jsonvarname);
	if (strlen(source)) {
		doc = cJSON_Parse(source);
		if (!doc) {
			ast_log(LOG_WARNING, "source json parsing error\n");
			json_set_operation_result(chan, ASTJSON_PARSE_ERROR);
			return 0;
		}
	} else {
		ast_log(LOG_WARNING, "source json is 0-length, delete would have no effect\n");
		json_set_operation_result(chan, ASTJSON_NOTFOUND);
		return 0;
	}
	// go over the path
	char *thispath = ast_strdupa((char *)(args.path + ((args.path[0] == '/') ? 1 : 0)));
	if (thispath[strlen(thispath) - 1] == '/') thispath[strlen(thispath) - 1] = 0;
	cJSON *thisobject = doc; cJSON *nextobject;
	int ixarray;
	char *deleteitem = NULL;
	char *pathpiece = strsep(&thispath, "/");
	int ret = ASTJSON_NOTFOUND;
	while (pathpiece) {
		deleteitem = ast_strdupa(pathpiece);
		// determine if we have an object with the given name or index
		if (sscanf(pathpiece, "%3d", &ixarray) == 1)
			nextobject = cJSON_GetArrayItem(thisobject, ixarray);
		else
			nextobject = cJSON_GetObjectItem(thisobject, pathpiece);
		if (!nextobject) break;
		char *pathpiece = strsep(&thispath, "/");
		if (pathpiece)
			thisobject = nextobject;
		else
			// got to the end of our path, we need to delete 'nextobject' from 'thisobject'
			switch (thisobject->type) {
			case cJSON_Array:
				cJSON_DeleteItemFromArray(thisobject, ixarray);
				ret = ASTJSON_OK;
				break;
			case cJSON_Object:
				cJSON_DeleteItemFromObject(thisobject, deleteitem);
				ret = ASTJSON_OK;
				break;
			default:
				ret = ASTJSON_DELETE_FAILED;
				break;
			}
	}

	// regenerate the source json
	char *jsonresult = cJSON_PrintUnformatted(doc);
	if (ret == ASTJSON_OK)
		pbx_builtin_setvar_helper(chan, args.jsonvarname, jsonresult); 
	free(jsonresult);
	cJSON_Delete(doc);
	json_set_operation_result(chan, ret);
	return 0;

}

static struct ast_custom_function acf_jsonpretty = {
	.name = "JSONPRETTY",
	.read = jsonpretty_exec
};
static struct ast_custom_function acf_jsoncompress = {
	.name = "JSONCOMPRESS",
	.read = jsoncompress_exec
};
static struct ast_custom_function acf_jsonelement = {
	.name = "JSONELEMENT",
	.read = jsonelement_exec
};

static int load_module(void) {
	int ret = 0;
	ret |= ast_custom_function_register(&acf_jsonpretty);
	ret |= ast_custom_function_register(&acf_jsoncompress);
	ret |= ast_custom_function_register(&acf_jsonelement);
	ret |= ast_register_application_xml(app_jsonvariables, jsonvariables_exec);
	ret |= ast_register_application_xml(app_jsonadd, jsonadd_exec);
	ret |= ast_register_application_xml(app_jsonset, jsonset_exec);
	ret |= ast_register_application_xml(app_jsondelete, jsondelete_exec);
	return ret;
}

static int unload_module(void) {
	int ret = 0;
	ret |= ast_custom_function_unregister(&acf_jsonpretty);
	ret |= ast_custom_function_unregister(&acf_jsoncompress);
	ret |= ast_custom_function_unregister(&acf_jsonelement);
	ret |= ast_unregister_application(app_jsonvariables);
	ret |= ast_unregister_application(app_jsonadd);
	ret |= ast_unregister_application(app_jsonset);
	ret |= ast_unregister_application(app_jsondelete);
	return ret;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "json parser and builder functions");
