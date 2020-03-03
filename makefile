CXX=g++-7
VERSION=-std=c++11
DEBUG_FLAGS=-O0
RELEASE_FLAGS=-O2 -DNDEBUG
CXX_FLAGS=-Wall -Wextra -Werror $(VERSION) -x c++ -lstdc++ -g $(DEBUG_FLAGS)

PRG=project2
SRC=Project2
OBJECTS=$(SRC)/tests.cpp $(SRC)/MemoryDebugger.cpp $(SRC)/main.cpp
OUT=out

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

.PHONY: debug release test

$(OUT)/:
	mkdir -p $(OUT)

debug release:| $(OUT)/ 
	$(CXX) -o $(OUT)/$(PRG) $(OBJECTS) $(CXX_FLAGS)
	objcopy --only-keep-debug $(OUT)/$(PRG) $(OUT)/$(PRG).debug
	strip -g $(OUT)/$(PRG)

run0 run1 run2 run3 run4 run5 run6 run7 run8 run9 run10 run11 run12:
	./$(OUT)/$(PRG) $(subst run,,$@)

runall: debug
	./runall.sh $(OUT)/$(PRG)

dbg0 dbg1 dbg2 dbg3 dbg4 dbg5 dbg6 dbg7 dbg8 dbg9 dbg10 dbg11 dbg12: debug
	objcopy --add-gnu-debuglink=$(OUT)/$(PRG).debug $(OUT)/$(PRG)
	dbg $(PRG) $(subst dbg,,$@)

clean:
	rm $(OUT)/*
