# skip async/await test (or transpile ...)

# sed -i 's/\r$//' test/test.sh

await=$(node -e "console.log(require('compare-versions')(process.version, '7.6.0'))")

if [ $await == "1" ]; then
    mocha --timeout 5000 "test/concurrency/await.spec"
fi
mocha --timeout 5000 "test/concurrency/callback.spec"
mocha --timeout 5000 "test/concurrency/promise.spec"
mocha --timeout 5000 "test/connection.spec"
mocha --timeout 5000 "test/connection.promise.spec"
mocha --timeout 5000 "test/datatypes.spec"
mocha --timeout 5000 "test/errors.spec"
mocha --timeout 5000 "test/errors.promise.spec"
mocha --timeout 5000 "test/options.spec"
mocha --timeout 5000 "test/options.promise.spec"
mocha --timeout 5000 "test/pool.spec"
mocha --timeout 5000 "test/performance.spec"

