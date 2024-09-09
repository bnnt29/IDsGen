#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    char *filepath = "./";
    filepath = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));
    strcat(filepath, argv[1]);
    FILE *file = fopen(filepath, "r");

    char line[256 * 4];
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    char *b = strstr(line, argv[2]);

    int e = b - line;

    if (e < 0 || e > (int)strlen(line))
    {
        printf("0\n");
        return 0;
    }

    int i = 0;
    for (int j = 0; j < e; j++)
    {
        if (line[j] == ';')
        {
            i++;
        }
    }
    if (argc >= 6)
    {
        printf("#%s\n", argv[3]);
        for (int j = 4; j <= 6  ; j++)
        {
            printf("%f\n", (strtod(argv[j], NULL) / 255));
        }
    }
    else
    {
        printf("#Bürger\n");
        printf("0.34\n");
        printf("0.6\n");
        printf("0.14\n");
    }

    while (fgets(line, sizeof(line), file))
    {
        int o = 0;
        int z = 0;
        for (int j = 0; j < (int)strlen(line) && z < i; j++)
        {
            if (line[j] == ';')
            {
                z++;
            }
            o++;
        }
        char *name = malloc(256 * sizeof(char));
        z = 0;
        int x = 0;
        for (int j = o - 1; j < (int)strlen(line); j++)
        {
            if (line[j] == ';' && line[j + 1] == '"')
            {
                x++;
                j += 2;
            }
            if (line[j] == '"' && line[j + 1] == ';' && x >= 1)
            {
                break;
            }
            name[z] = line[j];
            z++;
        }
        printf("%s\n", name);
    }
    printf("/\n");
}
