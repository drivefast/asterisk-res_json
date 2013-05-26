if [ ! -s include/asterisk.h ] ; then
	echo "please cd into the directory where the asterisk source has been untarred\n"
	exit
fi
cp asterisk-res_json/res_json.c addons/.
cp asterisk-res_json/cJSON.h include/asterisk/.
cp asterisk-res_json/cJSON.c addons/.
echo "edit addons/Makefile: add res_json to the list of modules built, and\n"
echo "    res_json.so: cJSON.o res_json.o\n"
