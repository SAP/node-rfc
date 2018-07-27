# skip async/await test (or transpile ...)

# https://stackoverflow.com/questions/11616835/r-command-not-found-bashrc-bash-profile
# sed -i 's/\r$//' test/test.sh

await=$(node -e "console.log(require('compare-versions')(process.version, '7.6.0'))")

if [ $await == "1" ]; then
    mocha --async-only
else
    mocha --async-only --exclude test/await.spec.js
fi
