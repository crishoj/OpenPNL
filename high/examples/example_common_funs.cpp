#include <stdio.h>
#include <string.h>

int CompareFiles(const char *name1, const char *name2)
{
    FILE *fp1 = fopen(name1, "r");
    FILE *fp2 = fopen(name2, "r");
    int result = 0;

    if(fp1 && fp2)
    {
	char buf[8192];
	int nByte;

	for(;;)
	{
	    nByte = fread(buf, 1, sizeof(buf)/2, fp1);
	    if(fread(buf + sizeof(buf)/2, 1, sizeof(buf)/2, fp2) != size_t(nByte))
	    {
		result = 2;//printf("(%s and %s): file differs by sizes", name1, name2);
		break;
	    }
	    if(nByte <= 0)
	    {
		break;
	    }
	    if(memcmp(buf, buf + sizeof(buf)/2, nByte))
	    {
		result = 3;//printf("(%s and %s): file differs by content", name1, name2);
		break;
	    }
	}
    }
    else
    {
        result = 1;// printf("Can't open one of the files: '%s', '%s'", name1, name2);
    }

    if(fp1)
    {
	fclose(fp1);
    }
    if(fp2)
    {
	fclose(fp2);
    }

    return result;
}
