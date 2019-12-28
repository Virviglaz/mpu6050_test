#include <stdio.h>
#include <stdlib.h>
#include "list.h"

static void handler(void *entry)
{
	printf("Handler: %s\n", entry);
}

int main(void)
{
	int i;
	char *p;
	struct list_t *list = create_list(malloc, free);
	printf("List created at: %p\n", list);

	for (i = 0; i != 10; i++) {
		p = malloc(10);
		sprintf(p, "i = %u", i);
		add_to_list(list, p);
	}
	printf("List size: %zu\n", get_list_size(list));

	remove_entry_by_num(list, 0);

	p = foreach_list(list);
	printf("First element: %s\n", p);

	do {
		p = foreach_list(NULL);
		printf("Next element: %s\n", p);
	} while (p);

	execute_foreach(list, handler);

	release_list(list);
}
