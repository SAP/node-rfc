# skip async/await test (or transpile ...)
[[ "$(node -v)" > "v7.6.0" ]] && mocha --napi-modules --async-only || mocha -napi-modules --async-only --exclude test/promise.async.js