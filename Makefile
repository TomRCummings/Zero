CXX = g++
CXXFLAGS = -g
SRCDIR = src
SRCS = $(SRCDIR)/zero.cpp $(SRCDIR)/gofer.cpp
OBJDIR = bin
TARGET = zero

ifeq ($(OS), Windows_NT)
	SRCS += $(SRCDIR)/Socket_win.cpp
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		SRCS += $(SRCDIR)/Socket_unix.cpp
	endif
endif

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CXX) -o $@ $^

debug: $(TARGET)-debug
$(TARGET)-debug: $(SRCS)
	$(CXX) -g -o $@ $^

clean:
	rm -rf $(TARGET) $(TARGET)-debug

.PHONY: all clean

