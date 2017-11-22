CC=gcc
XX=g++
IFLAGS= -I .
CFLAGS= -g -Wall -Wextra $(IFLAGS)
CXXFLAGS= -std=c++11 -g -Wall -Wextra $(IFLAGS)


STORAGE= applications/storage/
COM_AP= common/application/

all: $(STORAGE)storage_client $(COM_AP)configreader.o

$(COM_AP)configreader.o: $(COM_AP)configreader.cpp $(COM_AP)configreader.h
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(STORAGE)storage_client: $(STORAGE)storage_client.cpp $(COM_AP)configreader.o
	$(XX) -o $@ $< $(CXXFLAGS) $(COM_AP)configreader.o -lm -lpthread
 