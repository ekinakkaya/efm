#include <stdio.h>
#include <stdlib.h>

char *
readlink_malloc (const char *filename)
{
  int size = 100;
  char *buffer = NULL;

  while (1)
    {
      buffer = (char *) realloc (buffer, size);
      int nchars = readlink (filename, buffer, size);
      if (nchars < 0)
        {
          free (buffer);
          return NULL;
        }
      if (nchars < size)
        return buffer;
      size *= 2;
    }
}


int is_symlink(char filename[4096]) {
	struct stat sb;
	char f[4096];
	
	//strcpy(f, current_directory);
	//if (current_directory != "/") strcat(f, "/");
	strcat(f, filename);
	
	stat(f, &sb);

	switch (sb.st_mode & S_IFMT) {
        case S_IFLNK:  return 1; break;		// symlink
        //default:       return 0; break;		// default
	}
	return 0;


}



int main() {
	char *buff = readlink_malloc("/bin");
	printf("%s\n", buff);
}
