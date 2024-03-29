#!/usr/bin/env python3
import cgi
import os

print("Content-Type: text/html\n")

form = cgi.FieldStorage()

# Vérifiez si le formulaire a été soumis
if "file" in form:
    fileitem = form["file"]

    # Vérifiez si le fichier a été uploadé
    if fileitem.filename:
        # Déterminez le chemin où vous voulez sauvegarder le fichier
        # Assurez-vous que ce dossier existe et est accessible en écriture par le serveur web
        upload_path = "/ressources"
        filepath = os.path.join(upload_path, fileitem.filename)

        # Ouvrez le fichier en écriture binaire
        with open(filepath, 'wb') as fout:
            fout.write(fileitem.file.read())
        
        message = f"Le fichier '{fileitem.filename}' a été uploadé avec succès."

    else:
        message = "Aucun fichier n'a été uploadé."
else:
    message = "Aucun fichier n'a été uploadé."

print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Upload de Fichier CGI</title>
</head>
<body>
    <p>{message}</p>
</body>
</html>
""")