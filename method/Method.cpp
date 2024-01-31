#include "Method.hpp"
#include <iostream>

Method::Method() {}

Method::Method(const Method& other) {}

Method::~Method() {}

Method& Method::operator=(const Method& other) {
    if (this != &other) {
        
    }
    return *this;
}

void Method::sendErrorResponse(int clientSocket, int errorCode, const std::string& message) {
    std::string response = "HTTP/1.1 " + std::to_string(errorCode) + " " + message + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Connection: close\r\n\r\n";

    response += "<html><body><h1>" + std::to_string(errorCode) + " " + message + "</h1></body></html>";

    write(clientSocket, response.c_str(), response.length());
}

void Method::handleCgi(const std::string& uri, const std::string& requestBody, int clientSocket, const std::string& method) {
    Cgi cgi(uri, requestBody, clientSocket, method);  // Crée une instance de Cgi
    cgi.execute();  // Exécute le script CGI
}

// methode get 
void Method::sendFileResponse(const std::string& filePath, int clientSocket) {
    std::ifstream file(filePath.c_str(), std::ios::binary);
    
    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string fileContent = ss.str();
        
        std::string responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + Server::getMimeType(filePath) + "\r\nContent-Length: " + std::to_string(fileContent.length()) + "\r\n\r\n";
        write(clientSocket, responseHeader.c_str(), responseHeader.length());

        write(clientSocket, fileContent.c_str(), fileContent.length());
    } else {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(clientSocket, response.c_str(), response.length());
    }
}

// Méthode GET
void Method::get(const std::string& uri, int clientSocket) {
    std::cout << "GET request to: " << uri << std::endl;
	std::string targetUri = uri;
    if (uri == "/") {
        targetUri = "/index.html";
    }
	if (Server::isCgiRequest(targetUri)) {
        // Logique pour exécuter le script CGI
        handleCgi(targetUri, "", clientSocket, "GET");
	}
    else if (Server::isFileRequest(targetUri)) {
        std::string filePath = Server::getFilePath(targetUri);
        sendFileResponse(filePath, clientSocket);
    } 
	else {
        // Gérer les cas où ni un fichier ni un CGI ne correspondent à l'URI
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(clientSocket, response.c_str(), response.length());
    }
}

// Méthode POST
void Method::post(const std::string& uri, int clientSocket) {
	std::cout << "Content-Type: text/html\r\n\r\n";
	 char *content_length_str = getenv("CONTENT_LENGTH");
    if (content_length_str == nullptr) {
        sendErrorResponse(clientSocket, 400, "Bad Request");
        return;
    }

    int content_length;
    try {
        content_length = std::stoi(content_length_str);
    } catch (const std::exception& e) {
        sendErrorResponse(clientSocket, 400, "Bad Request");
        return;
    }

    if (content_length < 0) {
        sendErrorResponse(clientSocket, 400, "Bad Request");
        return;
    }

    char *post_data = new char[content_length + 1];
    std::cin.read(post_data, content_length);
    if (!std::cin) {
        delete[] post_data;
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
        return;
    }
    post_data[content_length] = '\0';
    std::string requestBody = post_data;
    delete[] post_data;

    // Si CGI requis
    if (Server::isCgiRequest(uri)) {
        // Logique d'exécution du CGI, gérer les erreurs potentielles
        try {
            handleCgi(uri, requestBody, clientSocket, "POST");
        } catch (const std::exception& e) {
            sendErrorResponse(clientSocket, 500, "Internal Server Error");
            return;
        }
    } else {
        // Autre logique POST
		sendErrorResponse(clientSocket, 404, "Not Found");
    }
}

// Méthode DELETE
void Method::deleteRequest(const std::string& uri, int clientSocket) {
    std::cout << "DELETE request to: " << uri << std::endl;

    std::string targetFilePath = Server::getFilePath(uri); // Obtenez le chemin du fichier basé sur l'URI

    if (std::filesystem::exists(targetFilePath)) {
        try {
            std::filesystem::remove(targetFilePath); // Tentative de suppression du fichier
            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
            write(clientSocket, response.c_str(), response.length());
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Erreur lors de la suppression du fichier : " << e.what() << '\n';
            sendErrorResponse(clientSocket, 500, "Internal Server Error");
        }
    } else {
        sendErrorResponse(clientSocket, 404, "Not Found"); // Fichier non trouvé
    }
}

