whatta...?
----------
__res_json__ is a wrapper module around a json library, that gives you the capability to handle json 
documents inside an asterisk dialplan. if i already have you confused with terms like "asterisk 
dialplan" (http://www.asterisk.org) or "json" (http://www.json.org), i will not insist in getting 
you interested for now. but, if you ever had to use external agi scripts just to obtain the value of 
a variable from a rest api, read on - this may help you. and yeah, i will try to make it easy to 
understand.

basic functions
---------------
a lot of api's these days send back responses in a json format. calling the api and obtaining the 
response is the "easy" part, since other people worked hard to create asterisk's func_curl (whic is 
a wrapper around the curl library). now we're going to deal with the other part - parsing the json 
http response, and obtaining the values for the variables we need.

here's the basic idea:

    exten => s,n,set(json=${CURL(http://api.dataprovider.com/somefunction?param=value)})
    exten => s,n,set(myvariable=${JSONELEMENT(json,path/to/element)})

this is going to call an api that returns a json document, and set the value of an asterisk dialplan 
variable to something found inside that json document. but this is not all: you can use the apps and 
functions inside __res_json__ to modify, or even construct from scratch, json documents. you can 
then post them, again using curl, to other api's:

    exten => s,n,jsonset(json,path/to/element,${newvalue})
    exten => s,n,jsonadd(json,path/to,number,newelement,5)
    exten => s,n,set(response=${CURL(http://api.datareceiver.com/somefunction,${json})})

more detailed info below.

install
-------
__res_json__ needs to be built into asterisk. i'm working with the people that take care of the 
asterisk distribution so we can include this module in the main distribution. until that happens, 
you will need to compile asterisk from source and have it take care of the json library. therefore, 
step by step, this is what you have to do.

(1) obtain the asterisk source code, from https://www.asterisk.org/downloads. unzip and untar it, 
but dont proceed to building it yet. 

(2) cd into the directory where you unzipped / untarred asterisk (the "asterisk root"), and get the 
__res_json__ module (git must be installed on your machine): 
`git clone git://github.com/drivefast/asterisk-res_json.git`

(3) we now need to move the source files to their appropriate places in the asterisk directory. a 
shell script was provided for that, so run `./asterisk-res_json/install.sh`

(4) edit the file main/asterisk.exports.in and add the following line next to the similar ones:

`LINKER_SYMBOL_PREFIXcJSON_*;`

(5) only now proceed with building asterisk (`./configure; make menuconfig; make; make install`). 
if you already built asterisk from source in this directory, you may need to run `./bootstrap.sh` 
before running `./configure`.

(6) start asterisk, login to its console, and try `core show function JSONELEMENT`. you should get 
an usage description.

what'd you get
--------------
a bunch of apps and functions:

- `JSONELEMENT(doc,path)` (r/o function) - gets the value of an element at a given path in a json document
- `jsonvariables(doc)` (app) - reads a single level json document (dictionary) into dialplan variables
- `jsonadd(doc,path,elemtype,name,value)` (app) - adds an element to the json document at the given path
- `jsonset(doc,path,newvalue)` (app) - changes the value of an element in the json document
- `jsondelete(doc,path)` (app) - deletes an element in the json document
- `JSONPRETTY(doc)` (r/o function) - formats a json document for nice printing
- `JSONCOMPRESS(doc)` (r/o function) - formats a json document for minimum footprint

none of the functions or the apps above would fail in such a way that would terminate the call. if 
any of them would need to return an abnormal result, they would do so by setting the value of a 
dialplan variable called `JSONRESULT`. these values are:
* `ASTJSON_OK` (0) - the operation was successful
* `ASTJSON_UNDECIDED` (1) - the operation was aborted mid-way and the results are not guaranteed
* `ASTJSON_ARG_NEEDED` (2) - missing or invalid argument type
* `ASTJSON_PARSE_ERROR` (3) - the string that was supposed to be a json document could not be parsed
* `ASTJSON_NOTFOUND` (4) - the expected element could not be found at the given path
* `ASTJSON_INVALID_TYPE` (5) - invalid element type for a jsonadd or jsonset operation
* `ASTJSON_ADD_FAILED` (6) - the jsonadd operation failed
* `ASTJSON_SET_FAILED` (7) - the jsonset operation failed
* `ASTJSON_DELETE_FAILED` (8) - the jsondelete operation failed

__IMPORTANT NOTE__ all the functions and apps expect **the name** of a dialplan variable containing 
the json document, instead of the parseable string itself. for example, if the document is stored in 
the variable named `json`, we would call the function and execute an app using `json` as parameter:

    exten => s,n,set(el=${JSONELEMENT(json,path/to/elem)})  
    exten => s,n,jsonset(json,path/to/elem,123) 

and __!!NOT!!__ the contents of the json variable, like in:

   exten => s,n,set(el=${JSONELEMENT(${json},path/to/elem)})  ;; WRONG
   exten => s,n,jsonset(${json},path/to/elem,123)             ;; WRONG

the decision on this type of usage was made because, typically, the json representations contain a 
lot of commas. escaping the json content such that the arguments are parsed correctly becomes 
therefore pretty complicated.

apps and functions
------------------

- `JSONELEMENT(doc,path)` 

>returns the value of an element at the given path. the element type is set in the dialplan variable 
>`JSONTYPE`. depending on the type of the json variable, the values are:
>
>   True, False => returned values are `1` or `0` respectively; `JSONTYPE` is `bool`
>
>   NULL => returned value is an empty string; `JSONTYPE` is `null`
>
>   Number => returned value is a number; `JSONTYPE` is `number`
>
>   String => returned value is a number; `JSONTYPE` is `string`
>
>   Array => returned value is a json representation of an array; `JSONTYPE` is `array`
>
>   Object => returned value is a json representation of the underlying object; `JSONTYPE` is `object`
>
>parameters:
>
>   _doc_: the name (not the contents!) of a variable that contains the json document
>
>   _path_: path to the element we're looking for (like `/path/to/element`, or `/path/to/element/3` 
>to identify the element with index 3 in an array)

- `jsonvariables(doc)`

>reads a single level json document into dialplan variables with the same names. the json document 
>is considered to be the representation of a dictionary, or key-value pairs, containing scalar 
>values. for example `{"one":1,"two":"deuce","three":"III"}` will set the values of the dialplan 
>variables `one`, `two` and `three` to the values `1`, `deuce`, and `III` respectively. depending on 
>the type of each variable, their possible values are:
>
>   True, False => `1`, `0`
>
>   NULL => resulting asterisk variable will contain an empty string
>
>   number, string => the number or the string
>
>   array => the string `!array!` (array values cannot be returned into a single variable) 
>
>   object => string, the json representation of the underlying object parameters
>
>parameters:
>
>   _doc_: the name (not the contents!) of a variable that contains the json document


- `jsonadd(doc,path,elemtype,[name][,value])`

>adds an element to the json document at the given path. the value of the variable that contains the 
>json document is updated to reflect the change. the element to be added has a type (_elemtype_), a 
>_name_, and a _value_. _elemtype_ can be one of `bool`, `null`, `number`, `string` or `array`. a 
>`bool` "false" value is represented as either an empty string, `0`, `n`, `no`, `f` or `false` (case 
>insensitive); any other value for a `bool` _elemtype_ is interpreted as true. for a `null` _elemtype_, 
>the _value_ paramenter is ignored. the _value_ parameter is also ignored for an `array` _elemtype_: 
>in this case, and an empty array is created. further on, you may append elements to this array using 
>repeated calls to the `jsonadd` app. something like this:

    exten => s,n,jsonadd(json,path/there,array,vec)
    exten => s,n,jsonadd(json,path/there/vec,string,,abcd)
    exten => s,n,jsonadd(json,path/there/vec,number,,1234)
    exten => s,n,noop(${JSONELEMENT(json,path/there/vec/0)} & ${JSONELEMENT(json,path/there/vec/1)})

>the last line will display `abcd & 1234` to the console.   
>
>parameters:
>
>  _doc_: the name (not the contents!) of a variable that contains the json document
>
>  _path_: path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3` 
>to identify the element with index 3 in an array)
>
>  _elemtype_: element type, one of `bool`, `null`, `number`, `string` or `array`
>
>  _name_: the name of the element to be added (may be missing if adding elements to an array)
>
>  _value_: value to be set for the element we added

- `jsonset(doc,path,newvalue)`

>sets the value of the element in the json document at the given path. the value of the variable 
>that contains the json document (_doc_) is updated to reflect the change. the element that changes 
>the value preserves its name and its type, and must be a boolean, number, or string. the new value 
>is converted to the type of the existing document. that means, if you would try to set the value of 
>a number element to `abc`, its resulting value will be `0`, or if you try to set a boolean element 
>to `13`, you will end up with it being `true`. to set a "false" value to a `bool` element, use an 
>empty string, `0`, `n`, `no`, `f` or `false` (case insensitive); anything else is interpreted as 
>`true`. 
>
>parameters
>
>   _doc_: the name (not the contents!) of a variable that contains the json document
>
>   _path_: path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3`
>to identify the element with index 3 in an array)
>
>   _newvalue_: value to be set
   
- `jsonset(doc,path)`

>delete the element at the given path, from the given document. the value of the variable that 
>contains the json document (_doc_)is updated to reflect the change. you may delete any type of 
>element.
>
>parameters
>
>   _doc_: the name (not the contents!) of a variable that contains the json document
>
>   _path_: path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3` 
>      to identify the element with index 3 in an array)

- `JSONPRETTY(doc)`

>returns the nicely formatted form of a json document, suitable for printing and easy reading. the 
>function has cosmetic functionality only.
>
>parameters
>
>   _doc_: the name (not the contents!) of a variable that contains the json document. the value will 
      not change.

- `JSONCOMPRESS(doc)`

>returns the given json document formatted for a minimum footprint (eliminates all unnecessary 
>characters). the function has cosmetic functionality only.
>
>parameters
>  _doc_: the name (not the contents!) of a variable that contains the json document. the value will 
>      not change.

author, licensing and credits
-----------------------------
Radu Maierean
radu dot maierean at gmail

Copyright (c) 2010 Radu Maierean

the __res_json__ module is distributed under the GNU General Public License version 2. The GPL 
(version 2) is included in this source tree in the file COPYING.

the __res_json__ module is built on top of David Gamble's cJSON library. i used his code with very 
minor and insignificant changes, to make my picky compiler happy. the __res_json module__ is 
intended to be used with asterisk, so you will have to follow their usage and distribution policy. 
and i guess so do i. i'm no lawyer and i have to take the safe route, and this is why i go with the 
same level of license restriction as asterisk does. 
