#!/usr/bin/env python3

import os
import urllib.parse
import sys

# HTTP header to indicate the response is HTML
print("HTTP/1.1 200 OK")
print("Content-Type: text/html\n")

# Retrieve the request body from the environment
#request_body = os.environ.get("REQUEST_BODY", "")
request_body = sys.stdin.read()

# Parse the request body into a dictionary
parsed_body = urllib.parse.parse_qs(request_body)

# Extract num1 and num2, converting them to float
# The values in parsed_body are lists, so we take the first item
try:
    query_string = os.environ.get("QUERY_STRING", "")
    query_params = query_string.split("&")
    get_params = {}
    for param in query_params:
        param_parts = param.split("=")
        if len(param_parts) == 2:
            param_name, param_value = param_parts
            get_params[param_name] = param_value
    
    
    # Calculate the sum
    num1 = float(get_params["num1"])
    num2 = float(get_params["num2"])
    result = num1 + num2

    # Display the result
    print("<html><head><title>Addition Result</title></head><body>")
    print("<h1>Addition Result:</h1>")
    print(f"<p>{num1} + {num2} = {result}</p>")
    print("</body></html>")
except ValueError:
    # In case of conversion error to number
    print("<html><head><title>Error</title></head><body>")
    print("<h1>Error:</h1>")
    print("<p>The provided values are not valid numbers.</p>")
    print("</body></html>")


#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11

#!/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11
# import os

# # Récupérer les paramètres de requête GET depuis l'environnement CGI
# query_string = os.environ.get("QUERY_STRING", "")
# query_params = query_string.split("&")

# # Initialiser des dictionnaires pour stocker les paramètres GET
# get_params = {}
# for param in query_params:
#     param_parts = param.split("=")
#     if len(param_parts) == 2:
#         param_name, param_value = param_parts
#         get_params[param_name] = param_value

# # Maintenant, vous avez les paramètres GET dans le dictionnaire get_params