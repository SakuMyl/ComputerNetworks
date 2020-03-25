#include <stdio.h>
#include "template.h"


int parse_str(const char *str, struct numbers *n)
{
	int ret;
	ret = sscanf(str, "%hhu %u %hhu %hu %u", &n->a, &n->b, &n->c, &n->d, &n->e);
	return ret;
}

void output_str(char *str, size_t len, const struct numbers *n)
{
	snprintf(str, len, "%u %u %u %u %u\n", n->a, n->b, n->c, n->d, n->e);
}
