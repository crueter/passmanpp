CXX		:= g++
LIBS	:= -lbotan-2 -lsqlite3 -lstdc++fs
CXXFLAGS:= -O2 -I "/usr/include/botan-2/" -fpermissive

SOURCE	:= main.cpp db.cpp sql.cpp string.cpp
OBJ		:= db.o sql.o string.o main.o
DEPS	:= db.h sql.h string.h

.SUFFIXES: .cpp.o

.cpp.o: %.o : %.h$(DEPS)
	$(CXX) $(CXXFLAGS) -c $< $(LIBS)

all: passman clean

clean:
	rm -f $(OBJ)

passman: $(OBJ)
	$(CXX) -o $@ $^ $(OBJS) $(CXXFLAGS) $(LIBS)

install:
	install -Dm755 passman /usr/bin/passman
