#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace asio = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;          // from <boost/beast.hpp>
using tcp = asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

int main() {
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
