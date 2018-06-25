#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

modules:
	rm -rf node_modules
	npm install
	node-pre-gyp configure

addon: 
	node-pre-gyp build # --loglevel=silent

wrapper:
	npm run wrapper

all: addon wrapper

debug:
	node-pre-gyp rebuild --debug

verbose:
	node-pre-gyp rebuild --loglevel=verbose

rebuild: modules all

test:
	npm run test

check: test

.PHONY: test clean build
