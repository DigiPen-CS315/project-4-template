CXX=g++-7
VERSION=-std=c++17
CONFIG=debug
DEBUG_FLAGS=-O0 -g3
RELEASE_FLAGS=-O2 -DNDEBUG -g
CXX_FLAGS=-Wall -Wextra -Werror $(VERSION) -x c++ -lstdc++

PRG=project2
OBJECTS=$(wildcard $(PRG)/*.cpp)
OUT=out

ifeq (release,$(CONFIG))
CXX_FLAGS+=$(RELEASE_FLAGS)
else
CXX_FLAGS+=$(DEBUG_FLAGS)
endif

ifneq (,$(findstring g++,$(CXX)))
CXX_FLAGS+=-rdynamic
else
CXX_FLAGS+=-fshow-source-location
endif

check_defined = \
    $(strip $(foreach 1,$1, \
        $(call __check_defined,$1,$(strip $(value 2)))))
__check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))

$(call check_defined, CXX, no compiler set)

.PHONY: build test

build:| $(OUT)/ 
	$(CXX) -o $(OUT)/$(PRG) $(OBJECTS) $(CXX_FLAGS)
	objcopy --only-keep-debug $(OUT)/$(PRG) $(OUT)/$(PRG).debug
	strip -g $(OUT)/$(PRG)

msbuild:
	./build.ps1 Debug x64

g++:| $(OUT)/
	make CXX=g++-7
	make CXX=g++-8
	# make CXX=g++-7 CONFIG=release
	# make CXX=g++-8 CONFIG=release

clang:| $(OUT)/
	make CXX=clang-8
	make CXX=clang-9
	# make CXX=clang-8 CONFIG=release
	# make CXX=clang-9 CONFIG=release

all:| $(OUT)/ g++ clang

run0 run1 run2 run3 run4 run5 run6 run7 run8 run9 run10 run11 run12:
	objcopy --add-gnu-debuglink=$(OUT)/$(PRG).debug $(OUT)/$(PRG)
	./$(OUT)/$(PRG) $(subst run,,$@)

runall:
	./runall.sh $(OUT)/$(PRG)

dbg0 dbg1 dbg2 dbg3 dbg4 dbg5 dbg6 dbg7 dbg8 dbg9 dbg10 dbg11 dbg12:
	objcopy --add-gnu-debuglink=$(OUT)/$(PRG).debug $(OUT)/$(PRG)
	dbg $(PRG) $(subst dbg,,$@)

$(OUT)/:
	mkdir -p $(OUT)

clean:
	rm $(OUT)/*
