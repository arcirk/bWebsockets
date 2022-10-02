export CERT_DOMAIN=arcirk.local
export CERT_IP=192.168.43.4

CERT_NAME=$CERT_DOMAIN
FILE_CERT_CONFIG=cert.conf

[ -f $CERT_NAME.key ] && mv -f $CERT_NAME.key{,.prev}
[ -f $CERT_NAME.crt ] && mv -f $CERT_NAME.crt{,.prev}

openssl genrsa -out $CERT_NAME.key 2048
openssl req -new -out $CERT_NAME.csr -key $CERT_NAME.key -config $FILE_CERT_CONFIG
openssl x509 -req -days 3650 -extensions req_ext -extfile $FILE_CERT_CONFIG \
             -in $CERT_NAME.csr -signkey $CERT_NAME.key -out $CERT_NAME.crt

rm -f $CERT_NAME.csr

echo
echo Certificate details:
echo
openssl x509 -text -in $CERT_NAME.crt