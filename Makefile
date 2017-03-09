
CC := /usr/local/arm_linux_4.8/bin/arm-linux-gcc
CPP := /usr/local/arm_linux_4.8/bin/arm-linux-g++
#CC := g++

TARGET := http-flv
#所有源文件
CSRCS = $(wildcard  *.c)
CPPSRCS = $(wildcard *.cpp)

COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)

LIBS := -lpthread

all:$(COBJS) $(CPPOBJS)
	$(CPP) -o $(TARGET) $(COBJS) $(CPPOBJS) $(LIBS)
	cp -rf $(TARGET) /ysg/nfs/http-flv/

$(COBJS) : %.o : %.c
	$(CC) -c $< -o $@ 
$(CPPOBJS) : %.o : %.cpp
	$(CPP) -c $< -o $@ 

clean:
	rm -rf http-flv
	rm -rf *.o
