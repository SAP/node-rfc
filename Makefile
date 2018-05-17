#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

all: build

./node_modules:
	npm install --build-from-source

build: ./node_modules
	node-pre-gyp build # --loglevel=silent

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
	@make build && npm run test

check: test

.PHONY: test clean build
