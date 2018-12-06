#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

addon:
	node-pre-gyp configure build # --loglevel=silent

wrapper:
	npm run build

build: addon wrapper

debug:
	node-pre-gyp rebuild --debug

verbose:
	node-pre-gyp rebuild --loglevel=verbose

test:
	npm run test

check: test

.PHONY: test clean build
