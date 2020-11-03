#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>

// key bindings - key presses are gonna be notated in hexadecimal
unsigned char kb_exit=0x71;	// exit			q
unsigned char kb_del=0x50;	// delete		d

unsigned char kb_nextfile=0x6A;	// next file		j
unsigned char kb_prevfile=0x6B;	// previous file	k
unsigned char kb_nextpage=0x4A;	// next page		J (Shift + j)
unsigned char kb_prevpage=0x4B;	// previous page	K (Shift + k)

unsigned char kb_parent_d=0x68;	// go to parent dir	h
unsigned char kb_child_d=0x6C;	// go to child dir	l

int cycle = 0;

/*
void get_user();
*/
char user_name[4096];

/* void get_time(); */
time_t now;
char current_time[64];

struct winsize w;
int ROWS, COLS;

int ch;

int lod_length = 0;

static char *list_of_directory[32768];
char current_directory[4096];

unsigned int selected_file;
unsigned int max_selection;
unsigned int current_page;
unsigned int pages;

unsigned int directory_changed = 1;

void get_scr_siz() {
	ioctl(0, TIOCGWINSZ, &w);
	ROWS = w.ws_row;
	COLS = w.ws_col;
	//printf("terminal size: %03i %03i", ROWS, COLS);	
}

void clear_scr() {
	printf("\e[1;1H\e[2J");
}

// Get username function
void get_user() {
	char *p = getenv("USER");
	if(p==NULL) printf(" wierd");
	
	//user_name = p;
	//printf("user: '%s'",p);
	strcpy(user_name, p);
}

// Print time function.
void get_time() {
	time(&now);
	strcpy(current_time, ctime(&now));
	//printf("%s", ctime(&now));
}

// Key press detection function
int mygetch ( void ) {
	int ch;
	struct termios oldt, newt;

	tcgetattr ( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );

	return ch;
}



int read_directory(char directory[]) {
	// clear the existing list_of_directory
	lod_length = 0;
	for (int k = 0; k < lod_length; k++) {
		//list_of_directory[k] = "";
		free(list_of_directory[k]);
	}

	DIR *d;
	struct dirent *dir;
	d = opendir(directory);

	int i = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			//printf("debug: reading.. '%s' \n", dir->d_name);
			//list_of_directory[i] = dir->d_name;

			list_of_directory[i] = malloc(strlen(dir->d_name) + 1);
			strcpy(list_of_directory[i], dir->d_name);

			i++;
			lod_length++;
		}
		closedir(d);
	}
	
}

/*
 * This function is too slow goddamnit. 
void sort_lod() {
	char t[4096];
	for (int s = 0; s < lod_length - 1; s++) {
		//printf("%i ", s);
		for (int d = 0; d < lod_length - 1; d++) {
			//printf("%i ", d);
			if (strcmp(list_of_directory[d], list_of_directory[d + 1]) > 0) {
				//printf("d1\n");
				strcpy(t, list_of_directory[d]);
				
				//free(list_of_directory[d]);
				list_of_directory[d] = realloc(list_of_directory[d], strlen(list_of_directory[d + 1]) + 1);
				strcpy(list_of_directory[d], list_of_directory[d + 1]);
				
				//free(list_of_directory[d + 1]);
				list_of_directory[d + 1] = realloc(list_of_directory[d], strlen(t) + 1);
				strcpy(list_of_directory[d + 1], t);
			}
		}
	}

}
*/


int comparefunc (const void* p1, const void* p2) {
	return strcmp (* (const char **) p1, * (const char **) p2);
}


void pp(char text[4096], int offset) {
	// short for 'pretty print'.
	// this function exists to handle the terminal size and to prevent the
	// screen overflow that happens with smaller terminal sizes
	for (int j = 0; j < COLS + 1 - offset; j++) {
		if (strlen(text) >= j) {
			printf("%c", text[j]);
		} else {
			printf(" ");
		}
	}
	printf("\n");
}



void get_current_dir() {
	char buff_dir_name[4096];
	getcwd(buff_dir_name, 4096);

	//current_directory = buff_dir_name;
	strcpy(current_directory, buff_dir_name);
}



/* C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }
	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;
	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );
	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

int is_directory(char name[4096]) {
	DIR* directory = opendir(name);
	
	if(directory != NULL) {
		closedir(directory);
		return 0;
	}

	if (errno == ENOTDIR) {
		return 1;
	}

	return -1;
}

/*
void get_term_colors() {
	char *t = getenv("LS_COLORS");
	//if(t==NULL) printf(" wierd");
	
	
}
*/

void pp_colored(char buf[4096], int offset, char filename[4096]) {
	/* directory --> */

	struct stat sb;
	stat(filename, &sb);

	
	if (lstat(filename, &sb) == 0 && sb.st_mode & S_IXUSR) {
		printf("\033[01;32m");
	}
	
	switch (sb.st_mode & S_IFMT) {
	case S_IFDIR:  printf("\033[01;34m");break;		// dir
        case S_IFBLK:  printf("\033[40;33;01m");break;		// block dev
        case S_IFCHR:  printf("\033[40;33;01m");break;		// character dev
        case S_IFIFO:  printf("\033[40;33m");break;		// FIFO/pipe
        case S_IFLNK:  printf("\033[01;36m");break;		// symlink
        case S_IFSOCK: printf("\033[01;35m");break;		// socket
        //case S_IFDIR:  printf("\033[01;34m");break;		// dir
        //case S_IFREG:  printf("\033[0m");break;		// regular file
        //default:       printf("\033[0m");break;		// unknown
	}
	

	pp(buf, offset);

	printf("\033[0m");
}

/*
char *readlink_malloc (const char *filename)
{
	printf("debug-- readlink_malloc 1");
	int size = 128;
	char *buffer = NULL;

	while (1) {
		buffer = (char *) realloc (buffer, size);
		int nchars = readlink (filename, buffer, size);
		
		if (nchars < 0) {
			free (buffer);
			return NULL;
		}
		if (nchars < size)
			return buffer;
		size *= 2;
	}
	printf("debug--readlink_malloc executed");
}
*/

/*
int is_symlink(char filename[4096]) {
	struct stat sb;
	char f[4096];
	
	strcpy(f, current_directory);
	if (current_directory != "/") strcat(f, "/");
	strcat(f, filename);
	
	stat(f, &sb);

	switch (sb.st_mode & S_IFMT) {
        case S_IFLNK:  return 1; break;		// symlink
        //default:       return 0; break;		// default
	}
	return 0;


}
*/

int main(int argc, char **argv)
{
	//chdir("~/");
	
	//Alternate screen buffer.
	printf("\033[?1049h\033[H");

	// user_name variable holds the username
	get_user();

	// ROWS and COLS variables hold the terminal size
	get_scr_siz();

	// current_time variable holds the time
	get_time();


	get_current_dir();


	// Main loop.
	while (1)
	{
		// Disable cursor
		printf("\e[?25l");


		get_scr_siz();
		get_time();


		// navigation in ui
		max_selection = lod_length - 1;

		
		// current_directory variable holds the current directory name
		// list_of_directory variable holds the list of current directory
		
		//get_current_dir();
		//read_directory(current_directory);
		//read_directory("/usr");

		// listing files in the current directory, handling pages

		int lod_length_to_print;
		if (lod_length > ROWS - 4) {
			lod_length_to_print = ROWS - 4;
		} else {
			lod_length_to_print = lod_length;
		}

		// handling pages
		// current_page, pages
		if (lod_length <= lod_length_to_print) {
			pages = 1;
		} else {
			pages = floor(lod_length / lod_length_to_print) + 1;
		}
		
	
		//--- assigning actions to key bindings
		
		if ( ch == kb_exit ) {
			clear_scr();

			for ( int i = 0; i < lod_length; i++ ) {
				free(list_of_directory[i]);
			}

			// Return to the previous screen buffer
			printf("\033[?1049l");

			// Enable cursor
			printf("\e[?25h");
			
			exit(0);
		}
		else if ( ch == kb_prevfile ) {
			if (selected_file > 0) {
				selected_file--;
			}
		}
		else if ( ch == kb_nextfile ) {
			if (selected_file < max_selection) {
				selected_file++;
			}
		}
		else if ( ch == kb_prevpage ) {
			if (selected_file < lod_length_to_print) selected_file = 0;
			else selected_file = selected_file - lod_length_to_print;
		}
		else if ( ch == kb_nextpage ) {
			if (selected_file + lod_length_to_print > max_selection) selected_file = max_selection;
			else selected_file = selected_file + lod_length_to_print;
		}
		
		else if ( ch == kb_parent_d ) {
			if (current_directory != "/") {
				
				chdir("..");
				selected_file = 0;

				directory_changed = 1;
			} else {
				chdir(".");
			}
		}
		else if ( ch == kb_child_d ) {
			//chtochild();
			if (is_directory(list_of_directory[selected_file]) == 0) {
				chdir(list_of_directory[selected_file]);
				selected_file = 0;

				directory_changed = 1;
			}
		}

	
		// navigation in ui
		max_selection = lod_length - 1;

		
		get_current_dir();

		
		if (directory_changed == 1) {
			read_directory(current_directory);
			//sort_lod();
			qsort(list_of_directory, lod_length, sizeof(char **), comparefunc);
		}
		


		// generating first line to be printed
		char rows_str[8];
		char cols_str[8];
		itoa(ROWS, rows_str, 10);
		itoa(COLS, cols_str, 10);

		char first_line_buffer[4096];
		strcpy(first_line_buffer, current_directory);
		strcat(first_line_buffer, " | ");
		strcat(first_line_buffer, "terminal size: ");
		strcat(first_line_buffer, rows_str);
		strcat(first_line_buffer, " ");
		strcat(first_line_buffer, cols_str);
		strcat(first_line_buffer, " | ");
		strcat(first_line_buffer, user_name);


		//printing first line
		printf("\e[0;30m\e[47m");
		pp(first_line_buffer, 1);
		printf("\e[0m");
		printf("\n");
	



		// listing files in the current directory, handling pages

		//int lod_length_to_print;
		if (lod_length > ROWS - 4) {
			lod_length_to_print = ROWS - 4;
		} else {
			lod_length_to_print = lod_length;
		}

		// handling pages
		// current_page, pages
		if (lod_length <= lod_length_to_print) {
			pages = 1;
		} else {
			pages = floor(lod_length / lod_length_to_print) + 1;
		}
	


		get_current_dir();

		if (directory_changed == 1) {
			
			read_directory(current_directory);
			//sort_lod();
			qsort(list_of_directory, lod_length, sizeof(char **), comparefunc);
			

			directory_changed = 0;
			
		}

		
		current_page = floor(selected_file / lod_length_to_print) + 1;

		// listing files in directory
		for (int d = 0 + lod_length_to_print*(current_page - 1); d < lod_length_to_print * current_page; d++) {
			char b[4096];
			char space[3] = "  ";
			char space_selected[3] = "* ";
			char space_d[3] = "*D";

			if (d < lod_length) {
				
				//check if selected path is a directory
				char selected_path[4096];
				strcpy(selected_path, current_directory);
				strcat(selected_path, "/");
				strcat(selected_path, list_of_directory[d]);

				if (d == selected_file) {
					/*
					if (is_directory(selected_path) == 0) strcat(b, "dir");
					else strcat(b, "---");
					*/
					strcat(b, space_selected);
					strcat(b, list_of_directory[d]);

				} else {
					/*
					if (is_directory(selected_path) == 0) strcat(b, "dir");
					else strcat(b, "---");
					*/
					strcat(b, space);
					strcat(b, list_of_directory[d]);

				}
				// print the buffer
				//pp(b, 2);
				pp_colored(b, 2, list_of_directory[d]);
			} else {
				printf("\n");
			}
			strcpy(b, "");

		}
		for (int f = 0; f < ROWS - lod_length_to_print - 4; f++) {
			printf("\n");
		}
		
		// printing last line
		printf("character :%X", ch);

		//check if selected path is a directory
		char selected_path[4096];
		strcpy(selected_path, current_directory);
		strcat(selected_path, "/");
		strcat(selected_path, list_of_directory[selected_file]);

		printf(" %i", is_directory(selected_path));
		
		printf(" | selected file index: %i | %i/%i page(s)", selected_file, current_page, pages);
		ch = mygetch();


		// Flush and clear the screen
		fflush(stdout);
		clear_scr();

		// Print program cycle for debug purposes
		//printf("%i", cycle);
		cycle++;

		
	}

	// end screen buffer
	printf("\033[?1049l");
 
	return 0;
}

