CC = gcc
CFLAGS = -Wall
SRC1 = src/example_virtualTracker.c
SRC2 = src/example_razorAHRS.c
TARGET1 = bin/example_virtualTracker
TARGET2 = bin/example_razorAHRS


all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2)

start:
	./bin/example_virtualTracker /dev/pts/1 & 
	VIRTUAL_TRACKER_PID=$!
	# waiting one second until virtual tracker is started
	sleep 1	
	./bin/example_razorAHRS /dev/pts/2
	
	kill $(VIRTUAL_TRACKER_PID)
		
clean:
	$(RM) $(TARGET1)
	$(RM) $(TARGET2)


#run: example
#	./bin/example

#example : example.c
#	gcc $< -o $@
