#include <fstream>
#include <algorithm>
#include <filesystem>
#include "filetools.h"

using namespace std;

const string SPLIT_EQ = " = ";
const string SPLIT_USCORE = "_";
const string SPLIT_LINEEND = "\r\n";
const vector<char> ADJ_CHAR = { '*','@','+','~' };

//Characters used in settings files for upgrade adjustments
//TODO: move to settings files
const vector<char>& amtGetAdjChar() {
    return ADJ_CHAR;
}

//Return proper case string (first letter capitalised only is better for parsing settings)
string strProperCase(string str) {
    transform(str.begin(), str.begin() + 1, str.begin(), [](unsigned char c) { return std::toupper(c); });
    //transform(str.begin() + 1, str.end(), str.begin() + 1, [](unsigned char c) { return std::tolower(c); });
    return str;
}

//Check if a character is an adjustment
bool isAdjChar(const char& ch) {
    for (char adj : ADJ_CHAR) {
        if (adj == ch)
            return true;
    }
    return false;
}

//Check if a character is a newline
bool isNewlineChar(const char& ch) {
    return (ch == '\n') || (ch == '\r');
}

int amtGetFilesize(string filename) {//add error handling
    ifstream file;
    file.open(filename, ios::binary);
    if (!file) {
        //throw FileException("StdIO Error: Could not open binary file to get size.");
    }
    file.ignore(numeric_limits<int>::max());
    int length = file.gcount();
    file.close();
    return length;
}

unsigned char* amtLoadBinaryFile(string filename, int filesize) {
    unsigned char* buffer = (unsigned char*)malloc(filesize * sizeof(unsigned char));
    if (!buffer) {
        //throw FileException("Malloc Error: Failed to allocate memory to load a file.");
    }
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        //throw FileException("StdIO Error: Could not open binary file to load.");
    }
    if (!fread(buffer, sizeof(unsigned char), filesize, file)) {
        //throw FileException("StdIO Error: Failed to read from opened binary file.");
    }
    fclose(file);
    return buffer;
}

void saveBinaryFile(string filename, unsigned char* data, int filesize) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        //throw FileException("StdIO Error: Could not open binary file to save.");
    }
    if (!fwrite(data, sizeof(unsigned char), filesize, file)) {
        //throw FileException("StdIO Error: Failed to write to opened binary file.");
    }
    fclose(file);
}

//same as above for now, this is used in patching exe only
void amtUpdateBinaryFile(string filename, int filesize, unsigned char* data) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        perror("Could not open binary file to save\n");
    }
    fwrite(data, sizeof(unsigned char), filesize, file);
    fclose(file);
}

//exe patching only, used to write the trampoline
void amtUpdateBinaryFile(string filename, int filesize, unsigned char* data, int appendSize, unsigned char* appendix) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        perror("Could not open binary file to save\n");
    }
    fwrite(data, sizeof(unsigned char), filesize, file);
    fwrite(appendix, sizeof(unsigned char), appendSize, file);
    fclose(file);
}