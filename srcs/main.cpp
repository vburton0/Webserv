#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <filesystem>
#include "includes/httpserver.hpp"

namespace asio = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;          // from <boost/beast.hpp>
using tcp = asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

int main(int ac, char **av) {
    
    //Config File and args handler
    std::string config_file;
    if (ac == 1){
        config_file = "config/default.conf";
    } else if (ac == 2){
        config_file = av[1];
    } else {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }

    // Check if config_file extension is ".conf"
    if (std::filesystem::path(config_file).extension() != ".conf") {
        throw std::runtime_error("Invalid config file extension. Must be '.conf'.");
    }

    

    try {
        // Créer une io_context pour gérer les opérations d'E/S
        asio::io_context io_context;

        // Créer un objet de type endpoint pour représenter le serveur
        tcp::endpoint endpoint(tcp::v4(), 8080);

        // Créer un objet acceptor pour accepter les connexions entrantes
        tcp::acceptor acceptor(io_context, endpoint);

        // Boucle infinie pour accepter les connexions
        while (true) {
            // Créer un socket pour la nouvelle connexion
            tcp::socket socket(io_context);

            // Attendre et accepter une nouvelle connexion
            acceptor.accept(socket);

            // Lire la requête HTTP
            beast::flat_buffer buffer;
            beast::http::request<beast::http::string_body> request;
            beast::http::read(socket, buffer, request);

            // Traiter la requête et construire une réponse
            std::string message = "Hello, World!";
            beast::http::response<beast::http::string_body> response{
                beast::http::status::ok, request.version()};
            response.set(beast::http::field::server, "MyServer");
            response.set(beast::http::field::content_type, "text/plain");
            response.body() = std::move(message);
            response.prepare_payload();

            // Envoyer la réponse
            beast::http::write(socket, response);
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
    }

    return 0;
}
