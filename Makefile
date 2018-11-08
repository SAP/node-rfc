#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

define DEPS_DEV
@babel/core @types/node acorn aws-sdk compare-versions \
decimal.js eslint eslint-plugin-mocha mocha node-gyp \
prettier-eslint random-bytes should typescript
endef

define DEPS_PROD
@types/bluebird bluebird node-addon-api node-pre-gyp
endef

addon:
	node-pre-gyp build # --loglevel=silent

wrapper:
	npm run build

uninstall:
	npm uninstall --save $(DEPS_DEV) $(DEPS_PROD)

install-prod:
	npm install --save $(DEPS_PROD)

install-dev:
	npm install --save-dev $(DEPS_DEV)

install: install-dev install-prod

reinstall:	uninstall install

build: addon wrapper

debug:
	node-pre-gyp rebuild --debug

verbose:
	node-pre-gyp rebuild --loglevel=verbose

test:
	npm run test

check: test

.PHONY: test clean build
