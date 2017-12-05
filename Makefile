CC=gcc
XX=clang++
IFLAGS= -I .
CFLAGS= -g -Wall -Wextra $(IFLAGS)
CXXFLAGS= -std=c++14 -g -Wall -Wextra $(IFLAGS)


STORAGE= applications/storage/
COM_AP=  common/application/
COM_DS=  common/dstage/

all: $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader 
  #$(COM_DS)requesthandlerdata.o $(COM_DS)test_requesthandlerdata

# *********************** TARGETS ********************
# common\applications
$(STORAGE)storage_client: $(STORAGE)storage_client.cpp $(COM_AP)configreader.o
	$(XX) -o $@ $< $(CXXFLAGS) $(COM_AP)configreader.o -lm -lpthread


# common\applications
$(COM_AP)configreader.o: $(COM_AP)configreader.cpp $(COM_AP)configreader.h
	$(XX) -c -o $@ $< $(CXXFLAGS)

$(COM_AP)test_configreader: $(COM_AP)test_configreader.cpp \
		$(COM_AP)configreader.o
	$(XX) -o $@ $< $(CXXFLAGS) $(COM_AP)configreader.o


# common\dstage
# $(COM_DS)requesthandlerdata.o: $(COM_DS)requesthandlerdata.cpp \
# 		$(COM_DS)requesthandlerdata.h $(COM_DS)requestdata.h
# 	$(XX) -c -o $@ $< $(CXXFLAGS)

# $(COM_DS)test_requesthandlerdata: $(COM_DS)test_requesthandlerdata.cpp \
# 		$(COM_DS)requesthandlerdata.o
# 	$(XX) -o $@ $< $(CXXFLAGS) $(COM_DS)requesthandlerdata.o

clean:
	rm $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader \
  $(COM_DS)requesthandlerdata.o $(COM_DS)test_requesthandlerdata


 