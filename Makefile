NODE = /usr/bin/env node
NPM = /usr/bin/env npm
GYP = /usr/bin/env node-gyp
VALGRIND = /usr/bin/env valgrind
MODULES = ./node_modules/

default: install
.PHONY: build

install: clean
	$(NPM) install

build:
	$(GYP) rebuild --debug

clean:
	@rm -rf $(MODULES)

valgrind:
	$(VALGRIND) --trace-children=yes node ./test.js
