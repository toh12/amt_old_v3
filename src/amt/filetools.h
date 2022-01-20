/*
* Filetools from older versions of AMT, using some C-style code.
* Loads and saves file data as binary data; due to the need to decrypt the files.
* TODO: Move into APCL library.
*/

#pragma once

#include <map>
#include <string>
#include <vector>
#include <exception>

const std::vector<char>& amtGetAdjChar();
int amtGetFilesize(std::string filename);
unsigned char* amtLoadBinaryFile(std::string filename, int filesize);
void amtUpdateBinaryFile(std::string filename, int filesize, unsigned char* data);
void amtUpdateBinaryFile(std::string filename, int filesize, unsigned char* data, int appendSize, unsigned char* appendix);