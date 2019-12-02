AR = ar
CC = gcc
CFLAGS = -Wall -O3
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LDFLAGS = `pkg-config --libs gtk+-3.0`
CPPFLAGS = `pkg-config --cflags gtk+-3.0`

all : gtkclip

SYSLIB = \
	-lpthread \
	-lsqlite3 \
	-ldl \
	-lrt \
	-lssl \
	-luuid \
	-lcrypto

gtkclip : gtkclip.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) gtkclip.cpp -o $@ $(LDFLAGS)

$(OUT)/%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUT)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
