#include "../includes/httpserver.hpp"

int essaie1() {
    // Création du socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return 1;
    }

    // Configuration de l'adresse du serveur
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Lier le socket à l'adresse du serveur
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Erreur lors de la liaison du socket à l'adresse." << std::endl;
        return 1;
    }

    // Écoute des connexions entrantes
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Erreur lors de l'écoute des connexions entrantes." << std::endl;
        return 1;
    }

    std::cout << "Serveur HTTP en attente de connexions sur le port 8080..." << std::endl;

    while (true) {
        // Accepter une connexion entrante
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Erreur lors de l'acceptation de la connexion entrante." << std::endl;
            return 1;
        }

        // Lire la requête du client
        char buffer[1024];
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            std::cerr << "Erreur lors de la lecture de la requête du client." << std::endl;
            return 1;
        }

        // Traiter la requête
        std::string request(buffer, bytesRead);
        std::cout << "Requête du client : " << request << std::endl;

        // Envoyer une réponse au client
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
        ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
        if (bytesSent < 0) {
            std::cerr << "Erreur lors de l'envoi de la réponse au client." << std::endl;
            return 1;
        }

        // Fermer la connexion avec le client
        close(clientSocket);
    }

    // Fermer le socket du serveur
    close(serverSocket);

    return 0;
}
