/*****************************************************************
 *                                                               *
 * (c) 2016 / QU Lab / T-Labs / TU Berlin                        *
 *                                                               *
 * --> more informations and changeable user settings            *
 *     in the razorAHRS.c file or on github                      *
 *     https://github.com/krueger-christian/razorAHRS            *
 *                                                               *
 ****************************************************************/

#ifndef RAZORCALIB_H
#define RAZORCALIB_H

void print_hint(int step){

	if(step == 0){
		printf("\r\n\n\n");
		printf("         ||           Point board down                 \r\n");
		printf("         ||           with connector                   \r\n");
		printf("         ||           holes (x-axis)                   \r\n");
		printf("         ||           and tilt slightly                \r\n");
		printf("      __|  |__                                         \r\n");
		printf("     |  |__|  |                                        \r\n");
		printf("     |  _||_  |                  +–––> y-axis          \r\n");
		printf("     | |||||| |                  |                     \r\n");
		printf("     | |||||| |                  v                     \r\n");
		printf("     | |||||| |                x-axis                  \r\n");
		printf("     | ...... |                                        \r\n");
		printf("                                                       \r\n");
		printf("                      PRESS K to keep min. x-value     \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 1){
		printf("\r\n\n\n");
		printf("     | ...... |       Point board up                   \r\n");
		printf("     | |||||| |       with connector                   \r\n");
		printf("     | |||||| |       holes (x-axis)                   \r\n");
		printf("     | |||||| |       and tilt slightly                \r\n");
		printf("     |   ||   |                                        \r\n");
		printf("     |__|  |__|                x-axis                  \r\n");
		printf("        |  |                     ^                     \r\n");
		printf("         ||                      |                     \r\n");
		printf("         ||            y-axis <––+                     \r\n");
		printf("         ||                                            \r\n");
		printf("         ||                                            \r\n");
		printf("                                                       \r\n");
		printf("                      PRESS K to keep max. x-value     \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 2){
		printf("\r\n\n\n");
		printf("            Point board to the right                   \r\n");
		printf("            with the reset button down                 \r\n");
		printf("            and tilt slightly		                   \r\n");
		printf("                                                       \r\n");
		printf("            _____________                              \r\n");
		printf("           |             |                             \r\n");
		printf("         __||| o     o  :|       +––> x-axis           \r\n");
		printf("   ======__|   o    ##  :|       |                     \r\n");
		printf("           |--      ##  :|       v                     \r\n");
		printf("           |___RST_______|     y-axis                  \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("            PRESS K to keep min. y-value               \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 3){
		printf("\r\n\n\n");
		printf("            Point board to the left                    \r\n");
		printf("            with the reset button up                   \r\n");
		printf("            and tilt slightly		                   \r\n");
		printf("                                                       \r\n");
		printf("            _____________                              \r\n");
		printf("           |             |     y-axis                  \r\n");
		printf("         __|___ ||||||| :|       ^                     \r\n");
		printf("   ======______=||||||| :|       |                     \r\n");
		printf("           |    ||||||| :|       +––> x-axis           \r\n");
		printf("           |_____________|                             \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("            PRESS K to keep max. y-value               \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 4){
		printf("\r\n\n\n");
		printf("            Point board down with                      \r\n");
		printf("            the usb-connector                          \r\n");
		printf("            and tilt slightly		                   \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("         ___==============       +––> x-axis           \r\n");
		printf("   ======_____|=--------         |                     \r\n");
		printf("                                 v                     \r\n");
		printf("                               z-axis                  \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("            PRESS K to keep min. z-value               \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 5){
		printf("\r\n\n\n");
		printf("            Point board up with                        \r\n");
		printf("            the usb-connector                          \r\n");
		printf("            and tilt slightly		                   \r\n");
		printf("                                                       \r\n");
		printf("                               z-axis                  \r\n");
		printf("         _____                   ^                     \r\n");
		printf("   ======_____|=--------         |                     \r\n");
		printf("            ==============       +––> x-axis           \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("            PRESS K to keep max. z-value               \r\n");
		printf("                                                       \r\n");
		printf("   ACCELEROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("  x min |  x max ||  y min |  y max ||  z min |  z max \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 6){
		printf("\r\n\n\n");
		printf("            Lay the board on a flat surface            \r\n");
		printf("            e.g. table (the reset button               \r\n");
		printf("            side should point up) and make 		       \r\n");
		printf("            sure it stays still                        \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("         ___==============       +––> x-axis           \r\n");
		printf("   ======_____|=--------         |                     \r\n");
		printf("  __________________________     v                     \r\n");
		printf("                               z-axis                  \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("            PRESS K to keep values                     \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
		printf("   GYROMETER\r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
		printf("    x   |  x (ø) ||    y   |  y (ø) ||    z   |  z (ø) \r\n");
		printf("--------+--------++--------+--------++--------+--------\r\n");
	}
	else if(step == 7){
		printf("\r\n\n                                                   \r\n");
		printf("                    _______                            \r\n");
		printf("                    \\  .  /                            \r\n");
		printf("                     \\:::/                             \r\n");
		printf("                      \\:/                              \r\n");
		printf("                      /:\\                              \r\n");
		printf("                     / . \\                             \r\n");
		printf("                    /.....\\                            \r\n");
		printf("                 -------------                         \r\n");
		printf("                                                       \r\n");
		printf("                                                       \r\n");
	}
	else if(step == 8){
		printf("\r\n\n                                                   \r\n");
		printf("                    _______                            \r\n");
		printf("                    \\     /                            \r\n");
		printf("                     \\   /                             \r\n");
		printf("                      \\ /                              \r\n");
		printf("                      / \\                              \r\n");
		printf("                     /...\\                             \r\n");
		printf("                    /:::::\\                            \r\n");
		printf("                 -------------                         \r\n");
		printf("                                                       \r\n");
		printf("                 DONE.                                 \r\n");
	}
}

void print_result(struct calibData *data){
		printf("\n\n");
		printf("                 Copy the following lines              \r\n");
		printf("                 and paste it into your                \r\n");
		printf("                 arduino code Razor_AHRS.ino           \r\n");
		printf("                                                       \r\n");
		printf("#define ACCEL_X_MIN ((float) %f)\r\n", data->acc_xmin);
		printf("#define ACCEL_X_MAX ((float) %f)\r\n", data->acc_xmax);
		printf("#define ACCEL_Y_MIN ((float) %f)\r\n", data->acc_ymin);
		printf("#define ACCEL_Y_MAX ((float) %f)\r\n", data->acc_ymax);
		printf("#define ACCEL_Z_MIN ((float) %f)\r\n", data->acc_zmin);
		printf("#define ACCEL_Z_MAX ((float) %f)\r\n", data->acc_zmax);
		printf("\r\n");
		printf("#define GYRO_AVERAGE_OFFSET_X ((float) %f)\r\n", data->gyr_x);
		printf("#define GYRO_AVERAGE_OFFSET_Y ((float) %f)\r\n", data->gyr_y);
		printf("#define GYRO_AVERAGE_OFFSET_Z ((float) %f)\r\n", data->gyr_z);
		printf("\r\n");
		printf("\r\n");
}


#endif
