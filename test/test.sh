# skip async/await test (or transpile ...)

# sed -i 's/\r$//' test/test.sh

await=$(node -e "console.log(require('compare-versions')(process.version, '7.6.0'))")

if [ $await == "1" ]; then
    mocha --timeout 5000 --async-only "test/concurrency/callback.spec.js" && \
    mocha --timeout 5000 --async-only "test/concurrency/promise.spec.js" && \
    mocha --timeout 5000 --async-only "test/concurrency/await.spec.js" && \
    mocha --timeout 5000 --async-only
else
    mocha --timeout 5000 --async-only "test/concurrency/callback.spec.js" && \
    mocha --timeout 5000 --async-only "test/concurrency/promise.spec.js" && \
    mocha --timeout 5000 --async-only
fi
