/**
 * replace.c
 *         Convert .csv file into data.txt, which is required by sch.
 *
 * Copyright (C) Muthu Subramanian K 2008 Dec
 */

#include <stdio.h>

#define FIND_CHAR   ','
#define REPLACE_CHAR '\n'
#define REPLACE_SPACE ' '
#define MULTIPLE 1

void replace(char *filename, FILE *fp2, char replace_char)
{
    FILE *fp1;
    int ch;
    char repl=0;
    fp1=fopen(filename,"rt");
    if(!fp1)
    {
        printf("Error: Unable to open: %s\n",filename);
        return;
    }
    while((ch=fgetc(fp1))!=EOF)
    {
        if(ch == FIND_CHAR || ch == replace_char)
        {
            if(!repl)
            {
                fputc(replace_char,fp2);
                if(MULTIPLE)
                    repl=1;
            }
        }
        else
        {
            repl=0;
            fputc(ch, fp2);
        }
    }
    fclose(fp1);
}

/* Params: <data.txt> <header> <company> <rooms> <students> <trailer> */
int main(int dummy, char *argc[])
{
    FILE *fp;
    if(dummy < 7)
        return 0;
    fp=fopen(argc[1],"wt");
    if(!fp) return 0;

    replace(argc[2],fp,FIND_CHAR); /* Headers */
    replace(argc[3],fp,REPLACE_CHAR); /* Companies */
    replace(argc[4],fp,REPLACE_SPACE); /* Rooms */
    replace(argc[5],fp,REPLACE_CHAR); /* Students */
    replace(argc[6],fp,REPLACE_SPACE); /* Trailers */
    
    fclose(fp);
    return 0;
}
