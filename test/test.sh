# skip async/await test (or transpile ...)
[[ "$(node -v)" > "v7.6.0" ]] && mocha || mocha --exclude test/promise.async.js