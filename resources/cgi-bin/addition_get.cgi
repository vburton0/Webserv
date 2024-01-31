
#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11

#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11
import os

# Récupérer les paramètres de requête GET depuis l'environnement CGI
query_string = os.environ.get("QUERY_STRING", "")
query_params = query_string.split("&")

# Initialiser des dictionnaires pour stocker les paramètres GET
get_params = {}
for param in query_params:
    param_parts = param.split("=")
    if len(param_parts) == 2:
        param_name, param_value = param_parts
        get_params[param_name] = param_value

# Maintenant, vous avez les paramètres GET dans le dictionnaire get_params

