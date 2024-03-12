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
    num1 = float(parsed_body.get("num1", [""])[0])
    num2 = float(parsed_body.get("num2", [""])[0])
    
    # Calculate the sum
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
