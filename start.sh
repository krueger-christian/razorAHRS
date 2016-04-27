socat /dev/pts/3 /dev/pts/4 & 
SOCAT_PID = $!

./bin/example_virtualTracker "/dev/pts/3" &
VIRTUAL_TRACKER_PID = $!

./bin/example_razorAHRS "/dev/pts/4" && echo "Cool" || echo "Failed"

kill $SOCAT_PID
kill $VIRTUAL_TRACKER_PID
