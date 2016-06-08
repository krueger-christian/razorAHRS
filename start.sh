#socat /dev/pts/3 /dev/pts/4 &
#socat -d -d pty,raw,echo=0 pty,raw,echo=0 &
#SOCAT_PID = $!

./bin/example_razorAHRSvirtual "/dev/pts/1" &
VIRTUAL_TRACKER_PID = $!

./bin/example_razorAHRS "/dev/pts/2" && echo "Cool" || echo "Failed"

#kill $SOCAT_PID
kill $VIRTUAL_TRACKER_PID
