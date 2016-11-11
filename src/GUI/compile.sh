gcc razor_calibration_gui.c -Wall -D_REENTRANT -lpthread -o calibration_assistant `pkg-config --libs --cflags gtk+-2.0`
