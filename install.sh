if [ ! -s include/asterisk.h ] ; then
	echo "please cd into the directory where the asterisk source has been untarred\n"
	exit
fi
cp asterisk-res_json/res_json.c res/.
cp asterisk-res_json/cJSON.h include/asterisk/.
cp asterisk-res_json/cJSON.c main/.
