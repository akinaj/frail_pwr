/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

bool loadFile(const char* filename, uint8** out_buf_ptr, uint32* out_buf_size, bool text_file = false);
bool fileExists(const char* filename);
bool writeTextFile(const char* filename, const char* buffer, size_t len);
