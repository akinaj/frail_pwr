/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "utils.h"

bool loadFile( const char* filename, uint8** out_buf_ptr, uint32* out_buf_size, bool text_file )
{
    FILE* file = fopen(filename, text_file ? "rt" : "rb");

    if (file == 0)
        return false;

    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    fseek(file, 0, SEEK_SET);

    *out_buf_size = (uint32)pos;
    *out_buf_ptr = new uint8[pos + 1];
    const size_t real_read = fread(*out_buf_ptr, 1, *out_buf_size, file);
    fclose(file);
    (*out_buf_ptr)[real_read] = '\0';

    return true;
}

bool fileExists( const char* filename )
{
    // TODO: replace this fopen with stat or smth
    FILE* file = fopen(filename, "rb");
    if (!file)
        return false;

    fclose(file);
    return true;
}

bool writeTextFile( const char* filename, const char* buffer, size_t len )
{
    FILE* file = fopen(filename, "wt");
    if (file == 0)
        return false;

    fwrite(buffer, 1, len, file);
    fclose(file);

    return true;
}
