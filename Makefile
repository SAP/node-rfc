#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

modules:
	rm -rf node_modules
	npm install
	node-pre-gyp configure

addon:
	node-pre-gyp build # --loglevel=silent

wrapper:
	npm run build

install-clean:
	rm -rf node_modules && cp test/package.json package.json

install-prod:
	yarn add @types/bluebird bluebird node-addon-api node-pre-gyp

install-dev:
	yarn add --dev @babel/core @types/node acorn aws-sdk compare-versions \
		decimal.js eslint eslint-plugin-mocha mocha node-gyp \
		prettier-eslint random-bytes should typescript

install: install-clean install-prod install-dev

build: addon wrapper

debug:
	node-pre-gyp rebuild --debug

verbose:
	node-pre-gyp rebuild --loglevel=verbose

test:
	npm run test

check: test

.PHONY: test clean build
