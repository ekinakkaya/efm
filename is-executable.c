#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc > 1) {
        struct stat sb;
        printf("%s is%s executable.\n", argv[1], stat(argv[1], &sb) == 0 && sb.st_mode & S_IXUSR ? "" : " not");
    }
    return 0;
}
