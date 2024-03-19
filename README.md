# Webserv

## Il y a 5 methode pour parler avec le serveur. 

- GET : [Serveur => Client] Cette méthode est utilisée pour demander des données à partir d'une ressource spécifiée. Par exemple, lorsqu'un navigateur web demande une page web, il utilise la méthode GET.

- POST : [Client => Serveur] Utilisée pour soumettre des données pour traitement à une ressource identifiée. Typiquement utilisée pour soumettre des formulaires en ligne ou envoyer des données au serveur.

- PUT : Utilisée pour mettre à jour une ressource existante ou créer une nouvelle ressource si elle n'existe pas déjà à l'emplacement spécifié.

- DELETE : Utilisée pour demander au serveur de supprimer une ressource spécifiée.

- PATCH : Utilisée pour appliquer des modifications partielles à une ressource. Cela permet de mettre à jour seulement certaines parties de la ressource sans avoir à renvoyer l'ensemble des données.


## Configuration file.

## Probleme rencontré

- Possibilité de seulement upload des fichier en dessous de +- 65000 bytes parce que les Pipes ont un buffer size limité. 
   - Solution possible : Chunk writting, pour pouvoir écrire tout le body de la request et donc tout le file que l'on veut upload. Il faut aussi faire que le Fd soit non blockant pour qu'il n'y ai pas de probleme.




# Sources 

- Serveur http
   - https://feel5ny.github.io/2019/08/03/HTTP_001/
- Methode http
   - https://javaplant.tistory.com/18
- Http header
   - https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers
