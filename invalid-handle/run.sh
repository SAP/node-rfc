
# server
pkill -f node
RFC_INI=./sapnwrfc.ini rm *.trc; node index.js > trace/log.txt &

sleep 2

# run clients, pass client request id
for i in {1..40}
do
    # wget -bq localhost:3456/doc/function/$i
    curl localhost:3456/doc/function/$i
done
