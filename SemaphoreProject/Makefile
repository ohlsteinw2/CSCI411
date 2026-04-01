CC      = g++
TARGET  = producer_consumer
SRCS    = producer_consumer.cpp
CFLAGS  = -Wall -Wextra -std=c++11
LDFLAGS = -lpthread

all: $(TARGET)

$(TARGET): $(SRCS) buffer.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
