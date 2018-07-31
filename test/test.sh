# skip async/await test (or transpile ...)

# sed -i 's/\r$//' test/test.sh

await=$(node -e "console.log(require('compare-versions')(process.version, '7.6.0'))")

if [ $await == "1" ]; then
    mocha --async-only 
else
    mocha --async-only --exclude test/concurrency.await.spec.js
fi
