#!/usr/bin/env python3

import cgi
import os

# HTTP header to indicate the response is HTML
print("HTTP/1.1 200 OK")
print("Content-Type: text/html\n")

form = cgi.FieldStorage()

# Vérifiez si le formulaire a été soumis
if "file" in form:
    fileitem = form["file"]

    # Vérifiez si le fichier a été upswwwloadé
    if fileitem.filename:
        # Déterminez le chemin où vous voulez sauvegarder le fichier
        # Assurez-vous que ce dossier existe et est accessible en écriture par le serveur web
        upload_path = "resources/futur/HandScanningAnimation/uploadedFile"
        filepath = os.path.join(upload_path, fileitem.filename)

        # Ouvrez le fichier en écriture binaire
        with open(filepath, 'wb') as fout:
            while True:
                chunk = fileitem.file.read(1024)  # Read in chunks of 1024 bytes
                if not chunk:
                    break  # Exit the loop when no more data is read
                fout.write(chunk)

        with open("resources/futur/HandScanningAnimation/uploadGood.html", 'r') as fichier_html:
            print(fichier_html.read())

    else:
       with open("resources/futur/HandScanningAnimation/uploadBad.html", 'r') as fichier_html:
            print(fichier_html.read())
else:
   with open("resources/futur/HandScanningAnimation/uploadBad.html", 'r') as fichier_html:
            print(fichier_html.read())
