all: tests

build:
	meson build

compile: build FORCE
	ninja -C ./build

tests: compile FORCE
	./build/tests/fsm_test

clean: FORCE
	rm -Rf ./build

FORCE:
