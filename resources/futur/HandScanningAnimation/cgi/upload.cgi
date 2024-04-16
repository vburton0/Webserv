#!/usr/bin/env python3

import cgi
import os

print("HTTP/1.1 200 OK")
print("Content-Type: text/html\n")

form = cgi.FieldStorage()

upload_path = "resources/futur/HandScanningAnimation/uploadedFiles"
if not os.path.exists(upload_path):
    os.makedirs(upload_path)

# Check if the form has been submitted
if "file" in form:
    fileitem = form["file"]
    # Check if the file has been uploaded
    if fileitem.filename:
        filepath = os.path.join(upload_path, fileitem.filename)
        # Open the file in binary write mode
        with open(filepath, 'wb') as fout:
            while True:
                chunk = fileitem.file.read(1024)  # Read in chunks of 1024 bytes
                if not chunk:
                    break  # Exit the loop when no more data is read
                fout.write(chunk)
        print("<p>The file '{}' has been uploaded successfully.</p>".format(fileitem.filename))
    else:
        print("<p>No file was uploaded.</p>")
else:
    print("<p>No file was uploaded.</p>")

# List all files in the upload directory
files = os.listdir(upload_path)
print("<h2>Available Files:</h2>")
print("<ul>")
for f in files:
    file_path = os.path.join("uploadedFiles/", f)
    print('<li><a href="{}">{}</a></li>'.format(file_path, f))
print("</ul>")
