//  compile with: gcc testgui.c -o testgui `pkg-config --libs --cflags gtk+-2.0` 

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "../razorAHRS.c"

GtkTextBuffer *buffer;
GtkTextBuffer *advise_buffer;
int *request ;
char *hint = " <path to store calibration data>";
char **hint_pics;

char *advise;
char **advise_paths;
#define ADVISE_BUFFER_SIZE 100

int step = 0;
int window_height = 540;
int window_width  = 540;
int img_height = 400;
int img_width = 400;


GtkWidget *window;
GtkWidget *table;
GtkWidget *title;
GtkWidget *txt;
GtkWidget *hintField;
GtkWidget *image;
GdkPixbuf *pixbuf;


void next_advise(){

	FILE *fp;
	memset(advise,' ', ADVISE_BUFFER_SIZE);

	fp = fopen(advise_paths[step], "r");
	fgets(advise, ADVISE_BUFFER_SIZE, (FILE*)fp);
	printf("%s\n", advise );
	gtk_text_buffer_set_text(advise_buffer, advise, strlen(advise));  
   
	fclose(fp);
}

void weiter(GtkButton *button, struct thread_parameter *parameter){

	printf("Razor Thread: %d\r\n", parameter->thread_id);

	razorAHRS_request(parameter);

	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter( buffer, &start);
	gtk_text_buffer_get_end_iter( buffer, &end);
	printf("weiter\r\n");
	printf("%s\r\n", gtk_text_buffer_get_text (buffer, &start, &end, TRUE));
	if(step < 5) step++;

	gtk_image_clear(GTK_IMAGE(image));

	pixbuf = gdk_pixbuf_new_from_file(hint_pics[step], NULL);
	pixbuf = gdk_pixbuf_scale_simple (pixbuf, img_width, img_height, GDK_INTERP_BILINEAR);
	//image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_image_set_from_pixbuf(GTK_IMAGE(image),pixbuf);

	next_advise();
}

void reset(GtkButton *button, struct thread_parameter *parameter)
{
	step = 0;
	printf("reset\r\n");

	razorAHRS_calibration_reset(parameter);

	gtk_image_clear(GTK_IMAGE(image));
	pixbuf = gdk_pixbuf_new_from_file(hint_pics[step], NULL);
	pixbuf = gdk_pixbuf_scale_simple (pixbuf, img_width, img_height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(GTK_IMAGE(image),pixbuf);

	next_advise();
}

int main(int argc, char *argv[])
{

	request = malloc(sizeof(int));
	*request = 0;

	struct thread_parameter* parameter = razorAHRS(B57600, argv[1], STREAMINGMODE_CALIBRATION, STREAMINGFORMAT_BINARY_FLOAT);
	razorAHRS_calibration (parameter, NULL);

	advise = (char*) calloc(ADVISE_BUFFER_SIZE, sizeof(char));

	hint_pics = (char**)calloc(6, sizeof(char*));
	for(int i = 0; i < 6; i++) hint_pics[i] = calloc(10, sizeof(char));
	sprintf(hint_pics[0], "x_max.png");
	sprintf(hint_pics[1], "x_min.png");
	sprintf(hint_pics[2], "y_max.png");
	sprintf(hint_pics[3], "y_min.png");
	sprintf(hint_pics[4], "z_max.png");
	sprintf(hint_pics[5], "z_min.png");

	advise_paths = (char**)calloc(6, sizeof(char*));
	for(int i = 0; i < 6; i++) advise_paths[i] = calloc(10, sizeof(char));
	sprintf(advise_paths[0], "x_max.txt");
	sprintf(advise_paths[1], "x_min.txt");
	sprintf(advise_paths[2], "y_max.txt");
	sprintf(advise_paths[3], "y_min.txt");
	sprintf(advise_paths[4], "z_max.txt");
	sprintf(advise_paths[5], "z_min.txt");

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, hint, strlen(hint));  

	advise_buffer = gtk_text_buffer_new(NULL);
	
	GtkWidget *actBtn;
	GtkWidget *fwdBtn;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request (window, window_width, window_height);
	gtk_window_set_title(GTK_WINDOW(window), "Razor AHRS Calibration Tool");
	GdkColor window_color;
	gdk_color_parse("#FFFFFF", &window_color);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &window_color);

	gtk_container_set_border_width(GTK_CONTAINER(window), 20);

	table = gtk_table_new(10, 10, TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_table_set_row_spacing(GTK_TABLE(table), 0, 5);

	// text view showing path to store
	txt = gtk_text_view_new_with_buffer(buffer);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(txt), TRUE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(txt), TRUE);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(txt), 10);
	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(txt), 12);
	GdkColor txt_color_bg;
	GdkColor txt_color_fg;
	gdk_color_parse("#FFFDA9", &(txt_color_bg));
	gdk_color_parse("#0000FF", &(txt_color_fg));
	gtk_widget_modify_text(txt, GTK_STATE_NORMAL, &txt_color_fg);
	gtk_widget_modify_base(txt, GTK_STATE_NORMAL, &txt_color_bg);
	gtk_table_attach(GTK_TABLE(table), txt, 0, 10, 0, 1, 
      GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);

	next_advise();

	// text view showing hints for user
	hintField = gtk_text_view_new_with_buffer(advise_buffer);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(hintField), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(hintField), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(hintField), GTK_WRAP_WORD);
	gtk_table_attach(GTK_TABLE(table), hintField, 0, 7, 9, 10, 
      GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);

	img_height = ((window_height-40) / 10) * 7;
	img_width  = ((window_width-40)  / 10) * 7;

	// some illustrations for user guidance
	pixbuf = gdk_pixbuf_new_from_file(hint_pics[0], NULL);
	pixbuf = gdk_pixbuf_scale_simple (pixbuf, img_width, img_height, GDK_INTERP_BILINEAR);
	pixbuf = gdk_pixbuf_new_from_file_at_size ("x_max.png", img_width, img_height, NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_table_attach(GTK_TABLE(table), image, 0, 7, 1, 8, 
		GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);

	int button_height =  window_height / 15;
	int button_width  =  window_width  / 10;

	// reset button
	actBtn = gtk_button_new_with_label("reset");
	gtk_widget_set_size_request(actBtn, button_width, button_height);
	g_signal_connect(G_OBJECT(actBtn), "clicked", G_CALLBACK(reset), parameter);
	gtk_table_attach(GTK_TABLE(table), actBtn, 8, 10, 8, 9, 
      GTK_FILL, GTK_SHRINK, 1, 1);

	// go on button
	fwdBtn = gtk_button_new_with_label("go on");
	gtk_widget_set_size_request(fwdBtn, button_width, button_height);
	g_signal_connect(G_OBJECT(fwdBtn), "clicked", G_CALLBACK(weiter), parameter);
	gtk_table_attach(GTK_TABLE(table), fwdBtn, 8, 10, 9, 10, 
      GTK_FILL, GTK_SHRINK, 1, 1);

	gtk_container_add(GTK_CONTAINER(window), table);

	g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), G_OBJECT(window));

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
