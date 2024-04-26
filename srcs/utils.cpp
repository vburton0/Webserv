#include "../includes/utils.hpp"
#include <string.h>


bool hasExtension(const std::string& filename, const std::string& extension) {
    size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    return filename.substr(dotPos) == extension;
}

std::string trimSpaceComments(const std::string str) {
    std::string result;
    size_t pos = 0;
    while (std::isspace(str[pos])) pos++;
    while (str[pos]){
        if (str[pos] == '#') {
            break;
        }
        else if (str[pos] == '\t')
			result += ' ';
		else
			result += str[pos];
		if (str[pos] == ' ' || str[pos] == '\t')
		{
			while (str[pos] == ' ' || str[pos] == '\t')
				pos++;
			pos--;
		}
		pos++;
	}
    if (result.size() > 0 && result[result.size() - 1] == ' ')
        result.erase(result.size() - 1, 1);
    return result;
}

int isErrorCode(int code)
{
	return ((code >= 400 && code <= 418) || (code >= 421 && code <= 426) || (code >= 428 && code <= 431) || code == 451
		|| (code >= 500 && code <= 508) || (code >= 510 && code <= 511));
}

char *ftStrDup(std::string str)
{
    char *dup = new char[str.size() + 1];
    strcpy(dup, str.c_str());
    return dup;
}


void ftStrCat(std::string src, char *dst)
{
	size_t cpyndex = 0;
	for (; dst[cpyndex]; cpyndex++);
	for (size_t index = 0; src[index]; index++, cpyndex++)
		dst[cpyndex] = src[index];
	dst[cpyndex] = '\0';
}

void displaySpecialCharacters(std::string str)
{
	for (size_t index = 0; index < str.size(); ++index)
	{
		char c = str[index];

		switch (c)
		{
			case '\\':
				std::cout << "\\";
				break;
			case '\n':
				std::cout << "\\n\n";
				break;
			case '\r':
				std::cout << "\\r";
				break;
			case '\t':
				std::cout << "\\t";
				break;
			default:
				if (isprint(c))
				{
					std::cout << c;
				}
				else
				{
					std::cout << '|' << c + '0' << '|';
				}
				break;
		}
	}
	std::cout << "EOF" << std::endl;
}

std::string readData(std::ifstream &indata)
{
	std::string res;
	std::string line;
	while (!indata.eof()) {
		std::getline( indata, line );
		res += line;
		if (!indata.eof())
			res += '\n';
	}
	indata.close();
	return (res);
}

char **mapToArray(std::map<std::string, std::string> env_map)
{
	char **ret = new char *[env_map.size() + 1];

	std::map<std::string, std::string>::iterator it = env_map.begin();
	std::map<std::string, std::string>::iterator ite = env_map.end();

	size_t index = 0;
	for (; it != ite; it++, index++)
	{
		ret[index] = new char[it->first.size() + 1 + it->second.size() + 1];
		ret[index][0] = '\0';
		ftStrCat(it->first, ret[index]);
		ftStrCat("=", ret[index]);
		ftStrCat(it->second, ret[index]);
	}
	ret[index] = NULL;
	return (ret);
}


std::string getBody(std::string bufstr)
{
	size_t index = bufstr.find("\r\n\r\n");
	if (index == std::string::npos)
		return ("");
	return (bufstr.substr(index + 4));
}


int checkHttpVersion(std::string bufstr)
{
	size_t index = bufstr.find("\r\n");
	if (index == std::string::npos || index < 8)
		return (0);
	return (bufstr.compare(index - 8, 8, "HTTP/1.1"));
}

int checkCorrectHost(std::string bufstr, std::list<std::string> server_names)
{
	size_t index = bufstr.find("Host: ");
	if (index == std::string::npos)
		return (1);
	
	std::list<std::string>::iterator it = server_names.begin();
	std::list<std::string>::iterator ite = server_names.end();
	for (; it != ite; it++)
	{
		if (!bufstr.compare(index + 6, (*it).size(), *it))
			return (0);
	}
	return (bufstr.compare(index + 6, 9, "localhost") && bufstr.compare(index + 6, 9, "127.0.0.1"));
}

int checkHTTPHeaderFormat(std::string bufstr)
{
	size_t index = bufstr.find("\r\n");
	while (index != std::string::npos)
	{
		index += 2;
		size_t endl_index = bufstr.find("\r\n", index);
		if (endl_index != std::string::npos && endl_index == index)
			return (0);
		size_t end_line = endl_index;
		if (endl_index == std::string::npos)
			end_line = bufstr.size();
		bool has_content = false;
		for (size_t curr_index = index; curr_index < end_line; ++curr_index)
		{
			if (bufstr[curr_index] == ':' && !has_content)
				return (1);
			has_content = true;
			if (bufstr[curr_index] == ':')
				break ;
			if (bufstr[curr_index] == ' ')
				return (1);
		}
		index = endl_index;
	}
	return (0);
}

std::string getLastWord(std::string str)
{
    size_t index = str.rfind('/');
    if (index == std::string::npos)
        return "/";
    else
        return str.substr(index + 1);
}

std::string GetContentType(std::string file)
{
    size_t index = file.rfind('.');
    if (index == std::string::npos)
        return ("text/plain");
    if (!file.compare(index, 4, ".css"))
        return ("text/css");
    if (!file.compare(index, 4, ".csv"))
        return ("text/csv");
    if (!file.compare(index, 4, ".htm") || !file.compare(index, 5, ".html"))
        return ("text/html");
    if (!file.compare(index, 4, ".ics"))
        return ("text/calendar");
    if (!file.compare(index, 4, ".js"))
        return ("text/javascript");
    if (!file.compare(index, 4, ".txt"))
        return ("text/plain");
    if (!file.compare(index, 4, ".xml"))
        return ("text/xml");
    if (!file.compare(index, 4, ".bmp"))
        return ("image/bmp");
    if (!file.compare(index, 4, ".gif"))
        return ("image/gif");
    if (!file.compare(index, 4, ".jpeg") || !file.compare(index, 4, ".jpg"))
        return ("image/jpeg");
    if (!file.compare(index, 4, ".png"))
        return ("image/png");
    if (!file.compare(index, 4, ".tiff"))
        return ("image/tiff");
    if (!file.compare(index, 4, ".ico"))
        return ("image/x-icon");
    if (!file.compare(index, 4, ".svg"))
        return ("image/svg+xml");
    if (!file.compare(index, 4, ".webp"))
        return ("image/webp");
    if (!file.compare(index, 4, ".mp3"))
        return ("audio/mpeg");
    if (!file.compare(index, 4, ".wav"))
        return ("audio/wav");
    if (!file.compare(index, 4, ".mp4"))
        return ("video/mp4");
    if (!file.compare(index, 4, ".mpeg"))
        return ("video/mpeg");
    if (!file.compare(index, 4, ".webm"))
        return ("video/webm");
    if (!file.compare(index, 4, ".json"))
        return ("application/json");
    if (!file.compare(index, 4, ".pdf"))
        return ("application/pdf");
    if (!file.compare(index, 4, ".zip"))
        return ("application/zip");
    if (!file.compare(index, 4, ".tar"))
        return ("application/x-tar");
    if (!file.compare(index, 4, ".gz"))
        return ("application/gzip");
    if (!file.compare(index, 4, ".rar"))
        return ("application/vnd.rar");
    if (!file.compare(index, 4, ".7z"))
        return ("application/x-7z-compressed");
    if (!file.compare(index, 4, ".doc"))   
        return ("application/msword");
    if (!file.compare(index, 4, ".docx"))
        return ("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    if (!file.compare(index, 4, ".ppt"))
        return ("application/vnd.ms-powerpoint");
    if (!file.compare(index, 4, ".pptx"))
        return ("application/vnd.openxmlformats-officedocument.presentationml.presentation");
    return "text/plain";
}

