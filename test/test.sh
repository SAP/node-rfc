# skip async/await test (or transpile ...)

# sed -i 's/\r$//' test/test.sh

await=$(node -e "console.log(require('compare-versions')(process.version, '7.6.0'))")

if [ $await == "1" ]; then
    mocha --async-only test/await && mocha --async-only test/concurrency && mocha --async-only
else
    mocha test/concurrency && mocha 
fi
