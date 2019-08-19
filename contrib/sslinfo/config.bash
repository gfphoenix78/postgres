# This bash script is internally used by sslinfo test
function sslinfo_prepare() {
echo "Enable SSL in postgresql.conf with master only..."

echo "#BEGIN SSLINFO CONF : BEGIN ANCHOR##" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "ssl=on" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "ssl_ciphers='HIGH:MEDIUM:+3DES:!aNULL'" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "ssl_cert_file='server.crt'" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "ssl_key_file='server.key'" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "ssl_ca_file='root.crt'" >> $PG_DATA_DIRECTORY/postgresql.conf
echo "#END SSLINFO CONF : END ANCHOR##" >> $PG_DATA_DIRECTORY/postgresql.conf

echo "preparing CRTs and KEYs"
cp -f data/root.crt   $PG_DATA_DIRECTORY/
cp -f data/server.crt $PG_DATA_DIRECTORY/
cp -f data/server.key $PG_DATA_DIRECTORY/
chmod 400 $PG_DATA_DIRECTORY/server.key
chmod 644 $PG_DATA_DIRECTORY/server.crt
chmod 644 $PG_DATA_DIRECTORY/root.crt

mkdir -p ~/.postgresql
cp -f data/root.crt         ~/.postgresql/
cp -f data/postgresql.crt   ~/.postgresql/
cp -f data/postgresql.key   ~/.postgresql/
chmod 400 ~/.postgresql/postgresql.key
chmod 644 ~/.postgresql/postgresql.crt
chmod 644 ~/.postgresql/root.crt
}

function sslinfo_clean() {
echo "restore SSL in postgresql.conf with master only"
sed -i '/#BEGIN SSLINFO CONF : BEGIN ANCHOR##/,/#END SSLINFO CONF : END ANCHOR##/d' $PG_DATA_DIRECTORY/postgresql.conf
}

case "$1" in
prepare)
#    sslinfo_prepare
echo "### BEGIN"
env
echo "### END"
    ;;
clean)
    sslinfo_clean
    ;;
*)
    echo "$0 { prepare | clean }"
    exit 1
esac
