CC = g++

CFLAGS = -march=native -pipe -std=gnu++11 -pedantic -Wall -Wextra -Werror
LDLIBS = -pthread

INCLUDE_PATH = ./inc
LIB_PATH     = ./lib

TARGET       = YOLO_crop
FILEXT       = cpp

SRCDIR       = src
OBJDIR       = obj
BINDIR       = bin

SOURCES     := $(wildcard $(SRCDIR)/*.$(FILEXT))
INCLUDES    := $(wildcard $(INCLUDE_PATH)/*.h)
LIBS        := $(wildcard $(LIB_PATH)/*.h) $(wildcard $(LIB_PATH)/*.hpp)
OBJECTS     := $(SOURCES:$(SRCDIR)/%.$(FILEXT)=$(OBJDIR)/%.o)

PATH_TO_EXE  = $(BINDIR)/$(TARGET)
LAUNCH_CMD   = $(PATH_TO_EXE) -i in -o out -e .jpg -s 30,60,64

all : debug

debug: CFLAGS += -Og -DDEBUG -g -ggdb
debug: $(PATH_TO_EXE)
	@echo "\033[93mRunning in debug mode!\033[0m"

release: CFLAGS += -Ofast
release: $(PATH_TO_EXE)
	@echo "\033[96mRunning in release mode!\033[0m"

run:
ifneq ("$(wildcard $(PATH_TO_EXE))", "")
	./$(LAUNCH_CMD)
else
	@echo "\033[91mNo executable found!\033[0m"
endif

run-release: release
	./$(LAUNCH_CMD)

run-debug: debug
	valgrind --leak-check=full --show-leak-kinds=all --vgdb=full -s ./$(LAUNCH_CMD)

$(PATH_TO_EXE): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)
	@echo "\033[92mLinking complete!\033[0m"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.$(FILEXT) $(INCLUDES)
	mkdir -p $(OBJDIR)
	$(CC) -o $@ -c $< $(CFLAGS) -isystem$(INCLUDE_PATH) -isystem$(LIB_PATH)


.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/*.gcda
	rm -f $(OBJDIR)/*.gcno
	rm -f $(PATH_TO_EXE)
