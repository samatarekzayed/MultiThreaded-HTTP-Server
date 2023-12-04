from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, parse_qs

class SimpleHTTPServer(BaseHTTPRequestHandler):
    def log_request(self, code='-', size='-'):
        self.log_message('"%s" %s %s',
                         self.requestline, str(code), str(size))

    def log_connection(self):
        client_host, client_port = self.client_address
        print(f"Connection from: {client_host}:{client_port}")

    def do_GET(self):
        self.log_connection()  # Log details about the incoming connection
        parsed_path = urlparse(self.path)
        query_params = parse_qs(parsed_path.query)
        file_path = parsed_path.path

        # Log details of the incoming GET request
        print(f"Received GET request for: {file_path}")
        print(f"Query Parameters: {query_params}")
        print(f"Request Headers:\n{self.headers}")

        if file_path == '/draft':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            with open('text/html', 'rb') as file:
                self.wfile.write(file.read())
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'404 Not Found')

    def do_POST(self):
        self.log_connection()  # Log details about the incoming connection
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)

        self.send_response(200)
        self.send_header('Content-type', 'image/jpeg')
        self.end_headers()
        self.wfile.write(b'POST request received. Data: ' + post_data)

def run_server(server_class=HTTPServer, handler_class=SimpleHTTPServer, port=1025):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Server started on port {port}")
    httpd.serve_forever()

if __name__ == '__main__':
    run_server()
