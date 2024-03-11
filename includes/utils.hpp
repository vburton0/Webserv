#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <string.h>
#include <fstream>

bool hasExtension(const std::string &str, const std::string &extension);
std::string trimSpaceComments(const std::string str);
int isErrorCode(int code);
char *ftStrDup(std::string str);
void ftStrCat(std::string src, char *dst);
void displaySpecialCharacters(std::string str);
std::string readData(std::ifstream &indata);
char **mapToArray(std::map<std::string, std::string> env_map);
std::string getBody(std::string bufstr);


int checkHttpVersion(std::string bufstr);
int checkCorrectHost(std::string bufstr, std::list<std::string>);
int checkHTTPHeaderFormat(std::string bufstr);
std::string GetContentType(std::string file);
char **getExecveArgs(std::string file_path);
std::string getLastWord(std::string str);

#endif