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
void ft_strcat(std::string src, char *dst);
void display_special_characters(std::string str);
std::string read_data(std::ifstream &indata);
char **map_to_array(std::map<std::string, std::string> env_map);
std::string getBody(std::string bufstr);


int check_http_version(std::string bufstr);
int check_correct_host(std::string bufstr, std::list<std::string>);
int check_header_names(std::string bufstr);
std::string GET_content_type(std::string file);
char **getExecveArgs(std::string file_path);
std::string get_last_word(std::string str);

#endif