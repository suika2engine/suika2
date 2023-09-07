#!/bin/python3

import http.server
import socketserver
import webbrowser

PORT = 8000
Handler = http.server.SimpleHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print("http://localhost:" + str(PORT) + "/")
    webbrowser.open("http://localhost:" + str(PORT) + "/", new=0, autoraise=True)
    httpd.serve_forever()
