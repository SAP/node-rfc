# skip async/await test (or transpile ...)
[[ "$(node -v)" > "v7.6.0" ]] && mocha -b || mocha -b --exclude test/promise.async.js