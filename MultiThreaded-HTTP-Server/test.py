# ####################################text get##############################################
# import socket

# def make_request(host, port, method, path='/', body=None, content_type=None):
#     if method == 'GET':
#         request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
#     elif method == 'POST':
#         if body is None:
#             raise ValueError('Body must be provided for POST requests')
#         content_length = len(body)
#         request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\n\r\n{body}'
#     else:
#         raise ValueError(f'Unsupported HTTP method: {method}')

#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.connect((host, port))
#         s.sendall(request.encode())
#         response = b""
#         while True:
#             part = s.recv(4096)
#             if not part:
#                 break
#             response += part

#         # Print the received data as text
#         print('Received:', response.decode('utf-8'))

# if __name__ == '__main__':
#     host = '127.0.0.1'
#     port = 1025

#     # Making a GET request for the 'textfile.txt' path with plain text content
#     make_request(host, port, 'GET', '/output.txt')








############################################text post#######################################

import socket

def make_request(host, port, method, path='/', body=None, content_type=None):

    if method == 'GET':
        request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
    elif method == 'POST':
        if body is None:
            raise ValueError('Body must be provided for POST requests')
        content_length = len(body)
        request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\n\r\n{body}'

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        s.sendall(request.encode())
        response = b""
        while True:
            part = s.recv(4096)
            if not part:
                break
            response += part
        print('Received:', response.decode())

if __name__ == '__main__':
    host = '127.0.0.1'
    port = 1024

    # Sending a POST request with Content-Type set to 'text/plain'
    make_request(host, port, 'POST', 'output.txt', 'elhamdulilah', 'text/plain')
########################################html get#########################################
# import socket

# def make_request(host, port, method, path='/', body=None, content_type=None):
#     if method == 'GET':
#         request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
#     elif method == 'POST':
#         if body is None:
#             raise ValueError('Body must be provided for POST requests')
#         content_length = len(body)
#         request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\n\r\n{body}'
#     else:
#         raise ValueError(f'Unsupported HTTP method: {method}')

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
#     port = 1025

#     # Making a GET request for the 'index.html' path
#     make_request(host, port, 'GET', '/index.html')



 #############################################html post####################################################
# import socket

# def make_request(host, port, method, path='/', body=None, content_type=None):

#     if method == 'GET':
#         request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
#     elif method == 'POST':
#         if body is None:
#             raise ValueError('Body must be provided for POST requests')
#         content_length = len(body)
#         request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\n\r\n{body}'

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
#     port = 1025

#     # Sending a POST request with Content-Type set to 'text/html'
#     make_request(host, port, 'POST', 'output.html', '<html><body><h1>Hello, World!</h1></body></html>', 'text/html')

########################################images post################################################################
# import socket
# import base64

# def make_request(host, port, method, path='/', body=None, content_type=None):

#     if method == 'GET':
#         request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
#     elif method == 'POST':
#         if body is None:
#             raise ValueError('Body must be provided for POST requests')
#         content_length = len(body)
#         request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\n\r\n{body}'

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
#     port = 1025

#     # Load an example image (replace 'your_image_path.jpg' with the actual image path)
#     with open('photo.jpeg', 'rb') as image_file:
#         image_content = base64.b64encode(image_file.read()).decode()

#     # Sending a POST request with Content-Type set to 'image/jpeg'
#     # make_request(host, port, 'POST', 'output.jpg', image_content, 'image/jpeg')
#     make_request(host, port, 'GET', 'image.jpeg', image_content, 'image/jpeg')
######cd MultiThreaded-HTTP-Server
########gcc -o server server.c

#########################image get#############################################
# import socket

# def make_request(host, port, method, path='/'):
#     request = f'{method} {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'

#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.connect((host, port))
#         s.sendall(request.encode())
#         response = b""
#         while True:
#             part = s.recv(4096)
#             if not part:
#                 break
#             response += part

#     return response

# if __name__ == '__main__':
#     host = '127.0.0.1'
#     port = 1025

#     # Making a GET request for the 'image.jpeg' path
#     response = make_request(host, port, 'GET', 'image.jpeg')
#     with open('received_image.jpeg', 'wb') as received_file:
#         received_file.write(response)

#     print('Received image saved as "received_image.jpeg"')



