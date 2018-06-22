#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

install:
	rm -rf node_modules
	npm install --save node-pre-gyp node-addon-api
	npm install --save-dev typescript @types/node decimal.js eslint eslint-plugin-mocha mocha prettier-eslint should

addon: 
	node-pre-gyp build # --loglevel=silent

wrapper:
	npm run build

all: addon wrapper

debug:
	node-pre-gyp rebuild --debug

verbose:
	node-pre-gyp rebuild --loglevel=verbose

clean:
	@rm -rf ./build
	rm -rf lib/binding/
	rm -rf ./node_modules/

rebuild:
	@make clean
	@make

test:
	npm run test

check: test

.PHONY: test clean build
