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

// SET BOTH TO ZERO TO DISABLE COLOR
#define USE_LS_COLORS 1 // set to zero for no ls colors
#define USE_MANUAL_COLORS 0 // set to zero for no manual colors
#define MANUAL_LS_COLORS "rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.zst=01;31:*.tzst=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.wim=01;31:*.swm=01;31:*.dwm=01;31:*.esd=01;31:*.jpg=01;35:*.jpeg=01;35:*.mjpg=01;35:*.mjpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.m4a=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.oga=00;36:*.opus=00;36:*.spx=00;36:*.xspf=00;36:"

#define LOD_INIT_SIZE 256


/*
struct efmdiritem
{
	char *i_name;
	char *i_type;
	char *i_ext;
	size_t *i_size;
};
*/

/* Key bindings
 * Use hexadecimal values to assign and change key bindings  */
unsigned char kb_exit=0x71;	// exit	                q
unsigned char kb_del=0x50;	// delete               d

unsigned char kb_nextfile=0x6A;	// next file        j
unsigned char kb_prevfile=0x6B;	// previous file    k
unsigned char kb_nextpage=0x4A;	// next page        J (Shift + j)
unsigned char kb_prevpage=0x4B;	// previous page    K (Shift + k)

unsigned char kb_parent_d=0x68;	// go to parent dir h
unsigned char kb_child_d=0x6C;	// go to child dir  l

//unsigned char kb_go_to_d=0x67;	// go to typed dir  g
//unsigned char kb_go_to_d=0x3A;	// type command     :

unsigned char kb_create_dir=0x43; // create dir     C (Shift + c)
unsigned char kb_file_yank=0x79; // yank file       y
unsigned char kb_file_cut=0x63; // cut file         c
unsigned char kb_file_paste=0x70; // paste file     p
unsigned char kb_rename=0x72; // rename         r




/* GLOBAL VARIABLES */

unsigned int cycle = 0;
char user_name[4096];

/* void get_time(); */
time_t now;
char current_time[64];

struct winsize w;
int ROWS, COLS;
int ROWS_PREV, COLS_PREV;

char ch;

int lod_length = 0;
unsigned int LOD_SIZE = LOD_INIT_SIZE;
static char **list_of_directory;

char current_directory[4096];
char parent_directory[4096];


unsigned int lod_length_to_print;

unsigned int selected_file;
unsigned int max_selection;
unsigned int current_page;
unsigned int pages;

unsigned int directory_changed = 1;
unsigned int prev_page_number = 1;


unsigned int LS_COLORS_SIZE;
static char LS_COLORS[1024][16];

void
get_scr_siz()
{
	ioctl(0, TIOCGWINSZ, &w);
	ROWS = w.ws_row;
	COLS = w.ws_col;
}


void
clear_scr()
{
	printf("\e[1;1H");
}


void
line_wrapping_enable()
{
	printf("\e[?7h");
}


void
line_wrapping_disable()
{
	printf("\e[?7l");
}


void
get_user()
{
	char *p = getenv("USER");
	if(p==NULL) printf(" wierd");
	strcpy(user_name, p);
}


void
get_time()
{
	time(&now);
	strcpy(current_time, ctime(&now));
}


struct termios oldt, newt;

void
set_canonical_mode(int state)
{
	switch (state)
	{
	case 0: tcsetattr ( STDIN_FILENO, TCSANOW, &oldt ); break; //off
	case 1:
		tcgetattr ( STDIN_FILENO, &oldt );
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		//newt.c_cc[VTIME] = 0.01;
		tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
		break;
	default: break;
	}
}


int
set_user_echo(int state)
{
	struct termios termInfo, save;
    int c;
	c = tcgetattr(0,&termInfo);

	/* error */
	if (c == -1) {
		return -1;
	}

	switch (state)
	{
		case 0: termInfo.c_lflag &= ~ECHO; /* turn off ECHO */ break;
		case 1: termInfo.c_lflag |= ECHO;  /* turn on ECHO */ break;
		default: return 0; break;
	}
}


int
read_directory(char *directory)
{
	/* clear the existing list_of_directory */
	
	//for (int k = 0; k < lod_length; k++) {
	for (int k = 0; k < lod_length; k++) {
		free(list_of_directory[k]);
	}
	lod_length = 0;
	LOD_SIZE = LOD_INIT_SIZE;

	DIR *d;
	struct dirent *dir;
	d = opendir(directory);

	int i = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (lod_length > LOD_SIZE) {
				LOD_SIZE *= 2;
				list_of_directory = realloc(list_of_directory, LOD_SIZE * sizeof(char*));
			}

			list_of_directory[i] = malloc(strlen(dir->d_name) + 1);
			strcpy(list_of_directory[i], dir->d_name);

			i++;
			lod_length++;
		}
		closedir(d);
	}
	
}

/*
int
yank_from_to(char from[], char to[])
{

}
*/

/* Used to compare and sort the list of directory */
int
comparefunc(const void* p1, const void* p2)
{
	return strcmp (* (const char **) p1, * (const char **) p2);
}


/*
void
pp(char text[4096], int offset)
{
	for (int j = 0; j < COLS + 1 - offset; j++) {
		if (strlen(text) >= j) {
			printf("%c", text[j]);
		} else {
			printf(" ");
		}
	}
	printf("\n");
}*/


void
get_current_dir()
{
	char buff_dir_name[4096];
	getcwd(buff_dir_name, 4096);
	strcpy(current_directory, buff_dir_name);
}



/* C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char
*itoa(int value, char* result, int base)
{
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


int
is_directory(char name[4096])
{
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


int
check_file_exists(const char *filename)
{
    struct stat buffer;
    int exist = stat(filename, &buffer);
    if (exist == 0)
        return 0;
    else
        return -1; // error
}


int
get_ls_colors()
{
	char ls[4096];
	if (USE_MANUAL_COLORS == 0) strcpy(ls, getenv("LS_COLORS"));

	if (ls == NULL || USE_MANUAL_COLORS == 1) {
		strcpy(ls, MANUAL_LS_COLORS);
	}

	int i = 0;
	int ii = 0;
	int index = 0;
	while (ls[i] != '\0') {
		if (ls[i] != ':') {
			LS_COLORS[index][ii] = ls[i];
		} else {
			LS_COLORS[index][ii+1] = '\0';
			index++;
			ii = -1;
		}
		i++;
		ii++;
	}

	LS_COLORS_SIZE = index;
	return 0;
}


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot;
}


void
print_ls_color(char *filename)
{
	char t[1024];
	char ext_t[1024];
	char *ext; // ex.: "*.out" OR "di", "pi" etc..
	char col_t[1024];
	char *col; // ex.: 01;35
	
	for (int i = 0; i < LS_COLORS_SIZE; i++) {
		strcpy(ext_t, LS_COLORS[i]);
		strcpy(t, LS_COLORS[i]);
		strcpy(col_t, LS_COLORS[i]);
		
		// if filename is actually a filename, 
		ext = strtok(t, ".");
		if (strcmp(ext, ext_t) != 0) {
			ext = strtok(ext_t, "*");
			ext = strtok(ext, "=");
		} else {
			ext = strtok(ext_t, "=");
		}
		col = strrchr(col_t, '=');
		col = strtok(col, "=");
		if (strcmp(filename, ext) == 0 || strcmp(get_filename_ext(filename), ext) == 0) {
			//printf("\e[%sm|%s, %s|%s, %s|", col, ext_t, LS_COLORS[i], ext, col);
			printf("\e[%sm", col);
			break;
		}
	}
}

int
pp_colored(char filename[4096])
{
	int flag = 0;
	struct stat sb;

	lstat(filename, &sb);
	
	//if (sb.st_mode & S_ISUID) { printf("\033[01;32m"); }
	if (sb.st_mode & S_ISUID) { print_ls_color("su"); }
	//if (sb.st_mode & S_ISGID) { printf("\033[01;32m"); }
	else if (sb.st_mode & S_ISGID) { print_ls_color("sg"); }
	

	// dir that is sticky and other writable (+t, o+w)
	if (sb.st_mode & S_IFDIR && sb.st_mode & S_ISVTX && sb.st_mode & S_IWOTH) { print_ls_color("tw"); flag = 1;}
	// dir that is other writable and not sticky (o+w)
	else if (sb.st_mode & S_IFDIR && sb.st_mode & S_IWOTH) { print_ls_color("ow"); flag = 1;}
	// dir that is sticky but not other writable (+t)
	else if (sb.st_mode & S_IFDIR && sb.st_mode & S_ISVTX && sb.st_mode & S_IWOTH) { print_ls_color("st"); flag = 1;}
	else {
		switch (sb.st_mode & S_IFMT) {
			//case S_IFDIR:  printf("\033[01;34m"); flag = 1; break;      // dir
			case S_IFDIR:  print_ls_color("di"); flag = 1; break;       // dir
    	    //case S_IFBLK:  printf("\033[40;33;01m"); flag = 1; break;   // block dev
    	    case S_IFBLK:  print_ls_color("bd"); flag = 1; break;       // block dev
    	    //case S_IFCHR:  printf("\033[40;33;01m"); flag = 1; break;   // character dev
    	    case S_IFCHR:  print_ls_color("cd"); flag = 1; break;   // character dev
    	    //case S_IFIFO:  printf("\033[40;33m"); flag = 1; break;      // FIFO/pipe
    	    case S_IFIFO:  print_ls_color("pi"); flag = 1; break;      // FIFO/pipe
    	    //case S_IFLNK:  printf("\033[01;36m"); flag = 1; break;      // symlink
    	    case S_IFLNK:  print_ls_color("ln"); flag = 1; break;      // symlink
    	    //case S_IFSOCK: printf("\033[01;35m"); flag = 1; break;      // socket
    	    case S_IFSOCK: print_ls_color("so"); flag = 1; break;      // socket
		}
	}
		//if (sb.st_mode & S_IXUSR) { printf("\033[01;32m"); }
	if (sb.st_mode & S_IXUSR && flag == 0) { print_ls_color("ex"); }

	if (flag == 0 && !(sb.st_mode & S_IFDIR)) {
		/*
		char ext_t[1024];
		char *ext; // ex.: *.out
		char col_t[1024];
		char *col; // ex.: 01;35
		
		for (int i = 0; i < LS_COLORS_SIZE; i++) {
			strcpy(ext_t, LS_COLORS[i]);
			strcpy(col_t, LS_COLORS[i]);

			ext = strtok(ext_t, "*");
			ext = strtok(ext, "=");

			col = strrchr(col_t, '=');
			col = strtok(col, "=");

			if (strcmp(get_filename_ext(filename), ext) == 0) {
				//printf("\e[%sm|%s, %s|%s, %s|", col, ext_t, LS_COLORS[i], ext, col);
				printf("\e[%sm", col);
				break;
			}
		}
		*/
		print_ls_color(filename);
	}
	printf("%s\e[0m", filename);
}

int
create_dir(char *dirname)
{
	int check = mkdir(dirname, 0777);
	if (!check) {
		return 0;
	} else {
		return -1;
	}
	
}


int
get_string(char **string_addr) {
	int bytes_read;
	size_t size = 10;

	char *string;
	string = (char *) malloc (size);
	bytes_read = getdelim(&string, &size, '\n', stdin);

	for (int i = 0; i < bytes_read; i++) {
		if (string[i] == '\n') string[i] = '\0';
	}

	if (bytes_read == -1) {
		return -1; // error
	} else {
		*string_addr = string;
		return 0;
	}
}



int
get_string_input(int mode, char *content, char **string_addr)
{
	char *input_str;

	newt.c_lflag |= ( ICANON | ECHO ); // change the mode of canonical mode
	                                   // and user echo to off
	tcsetattr ( STDIN_FILENO, TCSANOW, &newt ); // apply changes
	printf("\e[?25h"); // enable cursor

	switch (mode) {
		// clear screen and get a string
		case 0:
			printf("\e[2J%s", content);

			get_string(&input_str);
			*string_addr = input_str;

			break;

		// get a string from the first line
		case 1: 
			printf("\e[1;1f\033[K%s", content); // move the cursor to (1, 1), clear line

			get_string(&input_str);
			*string_addr = input_str;

			printf("\e[2J\e[1;1f"); // clear screen, move the cursor to (1, 1)

			break;
	}
	printf("\e[?25l"); // disable cursor
	newt.c_lflag &= ~( ICANON | ECHO ); // change the mode of canonical mode
	                                   // and user echo to on

	tcsetattr( STDIN_FILENO, TCSANOW, &newt );// apply changes
	clear_scr();
}


void
handle_ui_dirlist()
{
		/* listing files in the current directory, handling pages */

		lod_length_to_print = (ROWS - 4) * (lod_length > ROWS - 4) + lod_length * (lod_length <= ROWS - 4);


		/* handling pages
		 * current_page, pages*/
		if (lod_length <= lod_length_to_print) {
			pages = 1;
		} else {
			pages = floor(lod_length / lod_length_to_print) + 1;
		}
}


int
main(int argc, char **argv)
{
	printf("\033[?1049h\033[H"); /* Alternate screen buffer, disable line wrapping*/
	line_wrapping_disable();
	set_user_echo(0); /* set canonical mode and user echo */
	set_canonical_mode(1);
	printf("\e[?25l"); /* Disable cursor */

	get_user(); /* user_name variable holds the username */
	get_scr_siz(); /* ROWS and COLS variables hold the terminal size */
	get_time();/* current_time variable holds the time */

	if (USE_LS_COLORS == 1 || USE_MANUAL_COLORS == 1) get_ls_colors();

	get_current_dir();

	
	list_of_directory = malloc(LOD_INIT_SIZE * sizeof(char*));

	while (1) {
		get_scr_siz();
		get_time();

		/* navigation in ui */
		max_selection = lod_length - 1;
		handle_ui_dirlist();

		
		strcpy(parent_directory, current_directory);
		strcat(parent_directory, "/..");

		/* assigning actions to key bindings */
		if ( ch == kb_exit ) {
			for ( int i = 0; i < lod_length; i++ ) {
				free(list_of_directory[i]);
			}

			printf("\033[?1049l"); // Return to the previous screen buffer
			printf("\e[?25h"); // Enable cursor
			set_canonical_mode(0);
			line_wrapping_enable(); // Reenable line wrapping
			set_user_echo(1);
			
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
				while (is_directory(parent_directory) < 0) {
					strcat(parent_directory, "/..");
				}
				
				chdir(parent_directory);
				selected_file = 0;

				directory_changed = 1;
			} else {
				// go to parent directory, if valid: cd, else: go to parent directory again (make this a loop)
				// same thing must be done in the else if block below
				// else if ( ch == kb_child_d )

				chdir(".");
			}
		}
		else if ( ch == kb_child_d ) {
			//chtochild();
			if (is_directory(list_of_directory[selected_file]) == 0) {
				chdir(list_of_directory[selected_file]);
				selected_file = 0;
				directory_changed = 1;
			} else if (check_file_exists(list_of_directory[selected_file]) < 0) {
				directory_changed = 1; //if file can't be read, it's probably deleted or modified. refresh.

				// prevents a bug. selected_file can't be larger than max_selection.
				if(selected_file >= max_selection) selected_file = max_selection;
				selected_file--; // decrement by one, just in case...
			}
		}
		else if ( ch == kb_file_yank ) {
			// yank
			printf("yank");
		}
		else if ( ch == kb_file_cut ) {
			// cut
			//printf("");
		}
		else if ( ch == kb_file_paste ) {
			// paste
			//printf("");
		}
		else if ( ch == kb_create_dir ) {

			char *dirname;
			get_string_input(1, "\e[0;30m\e[47mType directory name:\e[0m ", &dirname);
			clear_scr();

			if (create_dir(dirname) < 0) {
				// error
			} else {
				// directory isn't actually changed, but the content did. So this is good enough, aight?..
				directory_changed = 1; 
			}
		}
		else if ( ch == kb_rename ) {
			char *new_filename;
			get_string_input(1, "\e[0;30m\e[47mRename:\e[0m ", &new_filename);
			//rename(list_of_directory[selected_file], new_filename);
			if (rename(list_of_directory[selected_file], new_filename) < 0) {
				// error
			} else {
				// directory isn't actually changed, but the content did. So this is good enough, aight?..
				directory_changed = 1; 
			}
		}


	


		/* check if the directory is changed, and if so, read the current directory. */
		get_current_dir();
		if (directory_changed == 1) {
			read_directory(current_directory);
			qsort(list_of_directory, lod_length, sizeof(char **), comparefunc); /* quick sort the list */
			directory_changed = 0;
			printf("\e[2J"); /* clear the screen */
		}
		
		
		// navigation in ui
		max_selection = lod_length - 1;
		handle_ui_dirlist();
		
		current_page = floor(selected_file / lod_length_to_print) + 1;

		/* if the page is changed OR the size of the terminal window changed, hard clear the screen */
		if ((prev_page_number != current_page) || (ROWS_PREV != ROWS || COLS_PREV != COLS)) {
			printf("\e[1J\e[2J");
		}

		/* update the previous size values of the terminal. which is ROWS_PREV and COLS_PREV */
		ROWS_PREV = ROWS;
		COLS_PREV = COLS;

		
		/* PRINT FIRST LINE */
		printf("\e[0;30m\e[47m%s | terminal size: %3i %3i | %s\e[0m\n", current_directory, ROWS, COLS, user_name);

		/* listing files in directory */

		char space[3] = "  ";
		char space_selected[3] = "* ";
		char space_d[3] = "*D";

		int d = 0 + lod_length_to_print*(current_page - 1);
		int dd = lod_length_to_print * current_page;

		// TODO: add a loop that gets the colors that will be 
		//       in the page. add to a list once. get the colors
		//       again only if the page or the directory is changed

		for (d; d < dd; d++) {
			if (d < lod_length)
			{
				if ( d == selected_file ) {
					printf("[ ] * ");
				} else {
					printf("[ ]   ");
				}

				if (USE_LS_COLORS == 1 || USE_MANUAL_COLORS == 1) pp_colored(list_of_directory[d]);
				else printf("%s", list_of_directory[d]);
				//printf("\033[0m");
				printf("\n");
			}
			else
			{
				printf("\n");
			}
		}
		for (int f = 0; f < ROWS - lod_length_to_print - 3; f++)
		{
			printf("\n");
		}
		
		
		handle_ui_dirlist();



		// printing last line
		printf(" %s | character :%X | selected file index: %i | %i/%i page(s)    \n\033[K%i"
		        , LS_COLORS[100], ch, selected_file, current_page, pages, LOD_SIZE);

		/* TODO: print what is the current operation, print instruction
		 * EX: copying /file/path/ex to ? | select directory or press v and type destination path */
		//printf("%s", );
		
		prev_page_number = current_page;

		/* check if the directory is changed, and if so, read the current directory. again. */
		get_current_dir();
		if (directory_changed == 1) {
			read_directory(current_directory);
			qsort(list_of_directory, lod_length, sizeof(char **), comparefunc); /* quick sort the list */
			directory_changed = 0;
			printf("\e[2J"); /* clear the screen */
		}

		

		//ch = mygetch();
		/* wait for user input and get the pressed key */
		ch = getchar();


		// Flush and clear the screen
		fflush(stdout);
		clear_scr();
	}

	/* end screen buffer, enable cursor, reenable line wrapping */
	printf("\e[?25h\e[?7h\033[?1049l");
	/* Set the scroll region to its default value. */
	// printf("\e[;r");
 
	return 0;
}
