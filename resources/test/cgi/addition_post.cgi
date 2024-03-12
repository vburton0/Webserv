#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11

#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11

import os
import sys

# En-tête HTTP pour indiquer que la réponse est au format HTML
print("Content-Type: text/html\n")

# Récupérer les données du requestBody depuis l'environnement
request_body = os.environ.get("REQUEST_BODY", "")

# Parsez le requestBody si nécessaire
# Vous devrez implémenter la logique de traitement du requestBody ici
# Par exemple, si le requestBody est au format JSON, vous pouvez le parser comme suit :
# import json
# request_data = json.loads(request_body)

# Votre logique de traitement du requestBody ici

# Exemple : Calculez la somme des nombres fournis dans le requestBody
try:
    # Remplacez cette logique par votre propre traitement du requestBody
    num1 = float(request_body.get("num1", ""))
    num2 = float(request_body.get("num2", ""))
    
    # Calculer la somme
    result = num1 + num2

    # Afficher le résultat
    print("<html><head><title>Résultat de l'addition</title></head><body>")
    print("<h1>Résultat de l'addition :</h1>")
    print(f"<p>{num1} + {num2} = {result}</p>")
    print("</body></html>")
except ValueError:
    # En cas d'erreur de conversion en nombre
    print("<html><head><title>Erreur</title></head><body>")
    print("<h1>Erreur :</h1>")
    print("<p>Les valeurs fournies ne sont pas des nombres valides.</p>")
    print("</body></html>")