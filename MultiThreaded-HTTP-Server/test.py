# import socket
# import concurrent.futures
# import requests

# def make_request(host, port, method, path='/', body=None):

#     if method == 'GET':
#         request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
#     elif method == 'POST':
#         if body is None:
#             raise ValueError('Body must be provided for POST requests')
#         request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Length: {len(body)}\r\n\r\n{body}'

#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.connect((host, port))
#         s.sendall(request.encode())
#         response = b""
#         while True:
#             part = s.recv(4096)
#             if not part:
#                 break
#             response += part
#         print('Received:', response.decode())





# if __name__ == '__main__':
#     host = '127.0.0.1'
#     port = 1024
#     #// g++ -o Server Server.cpp -pthread
#     #cd MultiThreaded-HTTP-Server
#     # # Sending a GET request
#     # make_request(host, port, 'GET', '/index.html', None)

#     # Sending a POST request
#     make_request(host, port, 'POST', 'input.txt', 'Hello from Python client')
#     # path = '/output.txt'    # Update with the path you want to POST to
#     # data = 'Hello from Python POST request'

#     # make_post_request(host, port, path, data)


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

        # Receive and print the server's response
        response = client_socket.recv(4096)
        print(response.decode())

if __name__ == "__main__":
    send_post_request()

