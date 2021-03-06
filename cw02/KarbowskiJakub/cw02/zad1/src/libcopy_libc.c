#include "libcopy.h"

#ifdef IMPL_LIB

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int copy_file(const char *in_path, const char *out_path)
{
    if (!in_path || !out_path) return -1;

    int err = 0;
    long buf_size = 256;
    char *buf = NULL;
    FILE *fin = NULL;
    FILE *fout = NULL;

    do
    {
        buf = malloc(buf_size);
        if (!buf)
        {
            err = -1;
            break;
        }

        fin = fopen(in_path, "r");
        if (!fin)
        {
            err = -1;
            break;
        }

        fout = fopen(out_path, "w");
        if (!fout)
        {
            err = -1;
            break;
        }

        while (!feof(fin))
        {
            // remember start of line
            long line_start = ftell(fin);
            if (line_start < 0)
            {
                err = -1;
                break;
            }

            // should this line be discarded
            int ignore = 1;

            // find end of line
            for (;;)
            {
                int c = fgetc(fin);
                if (feof(fin)) break;
                if (ferror(fin))
                {
                    err = -1;
                    break;
                }

                // found char in line
                if (!isspace(c)) ignore = 0;

                // end of line
                if (c == '\n') break;
            }
            if (err) break;

            // discard line
            if (ignore) continue;

            // remember end of line
            long line_end = ftell(fin);
            if (line_end < 0)
            {
                err = -1;
                break;
            }

            long line_size = line_end - line_start;

            // go to beginning
            err = fseek(fin, line_start, SEEK_SET);
            if (err) break;

            // realloc buf if needed
            if (line_size > buf_size)
            {
                buf_size = line_size;
                buf = realloc(buf, line_size);
                if (!buf)
                {
                    err = -1;
                    break;
                }
            }

            // read entire line
            size_t n = fread(buf, 1, line_size, fin);
            if (n != line_size)
            {
                err = -1;
                break;
            }

            // write line to output
            n = fwrite(buf, 1, line_size, fout);
            if (n != line_size)
            {
                err = -1;
                break;
            }
        }
    } while (0);

    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);
    if (buf)
        free(buf);

    return err;
}

#endif // IMPL_LIB
