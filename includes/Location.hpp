// Totalement copié collé ///////////////////////////////

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Webserv.hpp"
# include <algorithm>
# include <fstream>

class Location
{
	private:
		bool _auto_sighted;
		bool _line_sighted;
		bool _return_sighted;
		bool _body_sighted;

		void compareBlockInfo(std::string line);
		void checkSetDefault(void);

	public:
		Location(std::string line, std::ifstream & indata, std::string root);
		~Location(void);
		Location &operator=(const Location & other);

		std::string location;
		std::string root;
		std::list<std::string> index_files;
		std::vector<std::string> methods;
		std::string cgi;
		std::string _return;
		size_t body_size;
		bool auto_index;
		bool suffixed;

		void showLocationContent(void);
};

#endif