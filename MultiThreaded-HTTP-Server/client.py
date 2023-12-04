import socket

def send_post_request():
    host = "127.0.0.1"  # Update with your server's IP address
    port = 1024  # Update with your server's port

    data = "This is a test POST request data."

    # Create a socket connection to the server
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((host, port))

        # Craft the HTTP POST request
        request = f"POST / HTTP/1.1\r\nHost: {host}:{port}\r\nContent-Length: {len(data)}\r\n\r\n{data}"

        # Send the POST request to the server
        client_socket.sendall(request.encode())

        print(request.encode())

        # Receive and print the server's response
        response = client_socket.recv(4096)
        print(response.decode())

if __name__ == "_main_":
    send_post_request()