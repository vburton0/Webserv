#include "CGI.hpp"

# include ".h"


Cgi::Cgi(const std::string& uri, const std::string& requestBody, int clientSocket, const std::string& method)
: _method(method), _uri(uri), _requestBody(requestBody), _clientSocket(clientSocket) {}

Cgi::~Cgi() {}

void Cgi::setupEnvironment() {
    // Configurer les variables d'environnement pour le script CGI
    _environment["REQUEST_METHOD"] = _method;
    _environment["SCRIPT_FILENAME"] = findScriptPath(_uri);
    // Ajoutez d'autres variables d'environnement nécessaires ici
	if (_method == "POST") {
        _environment["REQUEST_BODY"] = _requestBody;
    }
	else if (_method == "GET") {
		size_t queryStart = _uri.find("?");
        if (queryStart != std::string::npos) {
            std::string queryString = _uri.substr(queryStart + 1);

            // Séparer les paramètres de la requête
            size_t currentPosition = 0;
            while (currentPosition < queryString.length()) {
                // Trouver le prochain '&' à partir de la position actuelle
                size_t nextAmpersand = queryString.find('&', currentPosition);
                if (nextAmpersand == std::string::npos) {
                    nextAmpersand = queryString.length();
                }

                // Extraire le paramètre entre la position actuelle et le prochain '&'
                std::string paramSegment = queryString.substr(currentPosition, nextAmpersand - currentPosition);

                // Rechercher le signe '=' pour séparer le nom et la valeur du paramètre
                size_t equalSignPos = paramSegment.find("=");
                if (equalSignPos != std::string::npos) {
                    std::string paramName = paramSegment.substr(0, equalSignPos);
                    std::string paramValue = paramSegment.substr(equalSignPos + 1);

                    // Stocker les paramètres dans l'environnement CGI
                    _environment["QUERY_STRING"] = queryString;
                    _environment[paramName] = paramValue;
                }

                // Passer à la position suivante après le prochain '&'
                currentPosition = nextAmpersand + 1;
            }
        }
	}
	
}
std::string Cgi::findScriptPath(const std::string& uri) {
    // Implémentez la logique pour trouver le chemin du script basé sur l'URI
    return "../ressources/cgi-bin/addition.cgi"; // Exemple de chemin
}

void Cgi::executeScript() {
	pid_t pid = fork(); // Crée un processus enfant
    if (pid == 0) {
        // Processus enfant : Exécutez le script CGI ici
        char* argv[] = {nullptr}; // Arguments pour le script, si nécessaire

        // Convertissez _environment en char**
        char** envp = new char*[_environment.size() + 1];
        int i = 0;
        for (const auto& entry : _environment) {
            std::string env_entry = entry.first + "=" + entry.second;
            envp[i] = strdup(env_entry.c_str());
            i++;
        }
        envp[i] = nullptr; // Marquez la fin de la liste des variables d'environnement

        execve(_environment["SCRIPT_FILENAME"].c_str(), argv, envp); // Exécute le script CGI
        exit(1); // Sortie en cas d'erreur lors de l'exécution du script
    } else if (pid > 0) {
        // Processus parent : attendez la fin du script CGI
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            // Le script CGI s'est terminé, récupérez et envoyez la réponse
        }
    } else {
        // Erreur lors du fork (échec de la création du processus enfant)
    }
}

void Cgi::execute() {
    setupEnvironment();
    executeScript();
}