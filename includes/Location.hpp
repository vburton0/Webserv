// Totalement copié collé ///////////////////////////////

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Webserv.hpp"
# include <algorithm>
# include <fstream>

class Location
{
	private:
		bool _autoSighted;
		bool _lineSighted;
		bool _returnSighted;
		bool _bodySighted;

		void compareBlockInfo(std::string line);
		void checkSetDefault(void);

	public:
		Location(std::string line, std::ifstream & indata, std::string root);
		~Location(void);
		Location &operator=(const Location & other);

		std::string location;
		std::string root;
		std::list<std::string> indexFiles;
		std::vector<std::string> methods;
		std::string cgi;
		std::string _return;
		size_t bodySize;
		bool autoIndex;
		bool suffixed;

		void showLocationContent(void);
};

#endif