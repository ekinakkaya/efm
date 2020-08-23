#include <stdio.h>
#include <string.h>

char *list_of_dir[16];
int lod_length = 0;

char deneme[] = "yArra";

int main() {
	list_of_dir[0] = ("asddsdsdsd %s asdasd", deneme);
	list_of_dir[1] = "zzzzz";

	printf("%s %s", list_of_dir[0], list_of_dir[1]);

	return 0;
}
