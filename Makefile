GLOG= /usr/local/lib/libglog.a
GFLAGS= /usr/lib/x86_64-linux-gnu/libgflags.a

CC=gcc
# newest clang build on Tufts servers which really just won't work unless a
# 	a newer version is built.
# XX=/usr/sup/llvm-5.0.0/bin/clang++.sh

# Use this compiler for Ubuntu
XX=clang++
# Use this compiler for Tufts serves with redhat
# XX=g++
IFLAGS= -I . -I /usr/local/include -I /usr/include
CFLAGS= -g -Wall -Wextra $(IFLAGS)
# If compiling with g++ then we want -fira-algorithm=priority for colored debug
# 	output
#CXXFLAGS= -std=c++14 -fira-algorithm=priority -g -Wall -Wextra $(IFLAGS)
CXXFLAGS= -std=c++14 -g -Wall -Wextra $(IFLAGS)
LFLAGS= -lpthread $(GLOG) $(GFLAGS)

STORAGE= applications/storage/
COM_AP=  common/application/
COM_DS=  common/dstage/

all: $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader \
  $(COM_DS)test_job \
  $(COM_DS)multiqueue.o $(COM_DS)test_multiqueue \
  $(COM_DS)dispatcher.o $(COM_DS)test_dispatcher \
  $(COM_DS)scheduler.o $(COM_DS)test_scheduler \
  $(COM_DS)dstage.o $(COM_DS)test_dstage
  

# *********************** TARGETS ********************
# common\applications
$(STORAGE)storage_client: $(STORAGE)storage_client.cpp $(COM_AP)configreader.o
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_AP)configreader.o

# common\applications
$(COM_AP)configreader.o: $(COM_AP)configreader.cpp $(COM_AP)configreader.h
	$(XX) -c -o $@ $< \
		$(CXXFLAGS)

$(COM_AP)test_configreader: $(COM_AP)test_configreader.cpp \
		$(COM_AP)configreader.o
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_AP)configreader.o


# common\dstage
$(COM_DS)test_job: $(COM_DS)test_job.cpp $(COM_DS)job.h $(COM_DS)priority.h
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS)

$(COM_DS)multiqueue.o: $(COM_DS)multiqueue.cpp $(COM_DS)multiqueue.h \
		 $(COM_DS)basemultiqueue.h $(COM_DS)priority.h \
		 $(COM_DS)job.h
	$(XX) -c -o $@ $< \
		$(CXXFLAGS)

$(COM_DS)test_multiqueue: $(COM_DS)test_multiqueue.cpp $(COM_DS)job.h \
		$(COM_DS)multiqueue.o $(COM_DS)priority.h
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_DS)multiqueue.o

$(COM_DS)dispatcher.o: $(COM_DS)dispatcher.cpp \
		$(COM_DS)dispatcher.h $(COM_DS)basedispatcher.h \
		$(COM_DS)multiqueue.h $(COM_DS)job.h $(COM_DS)priority.h
	$(XX) -c -o $@ $< \
		$(CXXFLAGS)

$(COM_DS)test_dispatcher: $(COM_DS)test_dispatcher.cpp \
		$(COM_DS)dispatcher.o \
		$(COM_DS)job.h $(COM_DS)multiqueue.o $(COM_DS)priority.h
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_DS)dispatcher.o $(COM_DS)multiqueue.o

$(COM_DS)scheduler.o: $(COM_DS)scheduler.cpp \
		$(COM_DS)scheduler.h $(COM_DS)basescheduler.h \
		$(COM_DS)basemultiqueue.h $(COM_DS)job.h $(COM_DS)priority.h
	$(XX) -c -o $@ $< \
		$(CXXFLAGS)

$(COM_DS)test_scheduler: $(COM_DS)test_scheduler.cpp \
		$(COM_DS)scheduler.o \
		$(COM_DS)job.h $(COM_DS)multiqueue.o $(COM_DS)priority.h
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_DS)scheduler.o $(COM_DS)multiqueue.o

$(COM_DS)dstage.o: $(COM_DS)dstage.cpp \
		$(COM_DS)dstage.h $(COM_DS)basedstage.h $(COM_DS)priority.h \
		$(COM_DS)job.h $(COM_DS)multiqueue.h $(COM_DS)basedispatcher.h \
		$(COM_DS)scheduler.h
	$(XX) -c -o $@ $< \
		$(CXXFLAGS)

$(COM_DS)test_dstage: $(COM_DS)test_dstage.cpp $(COM_DS)job.h \
		$(COM_DS)multiqueue.o  $(COM_DS)dispatcher.o \
		$(COM_DS)scheduler.o \
		$(COM_DS)priority.h $(COM_DS)basedstage.h $(COM_DS)dstage.o
	$(XX) -o $@ $< \
		$(CXXFLAGS) \
		$(LFLAGS) \
		$(COM_DS)multiqueue.o  $(COM_DS)dispatcher.o \
		$(COM_DS)scheduler.o $(COM_DS)dstage.o

format:
	clang-format -i  `find . -type f | command grep  '\.h\|\.cpp'`

clean:
	rm $(STORAGE)storage_client \
  $(COM_AP)configreader.o $(COM_AP)test_configreader \
  $(COM_DS)test_job \
  $(COM_DS)multiqueue.o $(COM_DS)test_multiqueue \
  $(COM_DS)dispatcher.o $(COM_DS)test_dispatcher \
  $(COM_DS)scheduler.o $(COM_DS)test_scheduler \
  $(COM_DS)dstage.o $(COM_DS)test_dstage


 