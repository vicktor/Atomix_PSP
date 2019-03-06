#include <stdio.h>
#include <stdlib.h>

#include "funciones.h"

int split_string(char *str, char **flds, char delim)
{
	char	*p, *q;			
	int		nflds;			

	nflds = 0;				

	if ((p = str) == NULL) return nflds;

	while (*p != '\0')
	{
		for (q = p; *p != '\0' && *p != delim; p++);
		flds[nflds++] = q;
		if (*p != '\0')	*p++ = '\0';
		if (*p == '\0')	flds[nflds++] = p;
	}
	return --nflds;
}
