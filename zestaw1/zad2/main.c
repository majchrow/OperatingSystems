#include <stdio.h>
#include "library.h"

int main(){
	struct wrapped *arr = create(2);
	set_dir(arr, "/");
	set_file(arr, "ss");
	execute_search(arr);
	int x = add_block(arr);
	set_dir(arr, "/bin");
	set_file(arr, "echo");
	execute_search(arr);
	int y = add_block(arr);
	printf("%d\n%d\n%d\n",x,y,arr->indicator);
	printf("%s", arr->blocks[0]);
}
