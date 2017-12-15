CC=gcc
XX=clang++
IFLAGS= -I .
CFLAGS= -g -Wall -Wextra $(IFLAGS)
CXXFLAGS= -std=c++14 -g -Wall -Wextra $(IFLAGS)


STORAGE= applications/storage/
COM_AP=  common/application/
COM_DS=  common/dstage/

all: $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader \
  $(COM_DS)job.o \
  $(COM_DS)duplicatestage.o \
  $(COM_DS)dispatcher.o
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
$(COM_DS)job.o: $(COM_DS)job.cpp \
		$(COM_DS)job.h $(COM_DS)priority.h
	$(XX) -c -o $@ $< $(CXXFLAGS)

$(COM_DS)duplicatestage.o: $(COM_DS)duplicatestage.cpp \
		$(COM_DS)duplicatestage.h $(COM_DS)dstage.h $(COM_DS)job.o
	$(XX) -c -o $@ $< $(CXXFLAGS)

$(COM_DS)dispatcher.o: $(COM_DS)dispatcher.cpp \
		$(COM_DS)dispatcher.h $(COM_DS)dstage.h $(COM_DS)duplicatestage.o
	$(XX) -c -o $@ $< $(CXXFLAGS)

# $(COM_DS)requesthandlerdata.o: $(COM_DS)requesthandlerdata.cpp \
# 		$(COM_DS)requesthandlerdata.h $(COM_DS)requestdata.h
# 	$(XX) -c -o $@ $< $(CXXFLAGS)

# $(COM_DS)test_requesthandlerdata: $(COM_DS)test_requesthandlerdata.cpp \
# 		$(COM_DS)requesthandlerdata.o
# 	$(XX) -o $@ $< $(CXXFLAGS) $(COM_DS)requesthandlerdata.o

clean:
	rm $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader \
  $(COM_DS)requesthandlerdata.o $(COM_DS)test_requesthandlerdata \
  $(COM_DS)job.o $(COM_DS)duplicatestage.o $(COM_DS)dispatcher.o


 