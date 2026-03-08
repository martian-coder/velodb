# VeloDB: High-Performance Asynchronous Storage Engine
# Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
# 
# This software and the associated Nexus Protocol (NXP) are the 
# intellectual property of Amit Nilajkar.
# Licensed under the MIT License. See LICENSE file for details.

import http.server
import socketserver
import subprocess
import json
import sys
import os

# Configuration
VELODB_CLIENT = os.path.join(os.path.dirname(__file__), "..", "build", "Release", "velodb-client.exe")
VELODB_PORT = 9001
EXPORT_PORT = 8000

class MetricsHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/metrics":
            try:
                # Query STATS from VeloDB using the client
                result = subprocess.run(
                    [VELODB_CLIENT, "--port", str(VELODB_PORT), "STATS"],
                    capture_output=True, text=True, check=True
                )
                stats = json.loads(result.stdout.strip())
                
                # Convert to Prometheus format
                output = []
                for key, val in stats.items():
                    if isinstance(val, (int, float)):
                        output.append(f"velodb_{key} {val}")
                    elif isinstance(val, bool):
                        output.append(f"velodb_{key} {1 if val else 0}")
                
                self.send_response(200)
                self.send_header("Content-type", "text/plain")
                self.end_headers()
                self.wfile.write("\n".join(output).encode())
            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(str(e).encode())
        else:
            self.send_response(404)
            self.end_headers()

def main():
    print(f"VeloDB Metrics Exporter starting on port {EXPORT_PORT}...")
    print(f"Forwarding to VeloDB on port {VELODB_PORT}")
    with socketserver.TCPServer(("", EXPORT_PORT), MetricsHandler) as httpd:
        httpd.serve_forever()

if __name__ == "__main__":
    main()

