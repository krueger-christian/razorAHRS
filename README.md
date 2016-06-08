Parser for headtracker razorAHRS
===

Program to read binary data stream of the [RazorAHRS](https://github.com/ptrbrtz/razor-9dof-ahrs) headtracker, written in C.

Contains a parser and a virtual headtracker.

Build
---

	$ cmake . && make


Virtual Tracker
---

If you want to start the reader razorAHRS connected to a virtual tracker: $ sh start.sh

It will:

1. Create a _virtual_ serial port,
2. Start a virtual tracker, and
3. Start the reader and print out the data until spacebar or enter is pressed.