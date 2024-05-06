
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++17 -Wall

all: correctness persistence

correctness: vLog.o sstindex.o sstheader.o sstable.o memtable.o kvstore.o correctness.o

persistence: vLog.o sstindex.o sstheader.o sstable.o memtable.o kvstore.o persistence.o

clean:
	-rm -f correctness persistence *.o
