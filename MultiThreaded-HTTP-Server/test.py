import socket
import concurrent.futures
import requests

def make_request(host, port, method, path='/', body=None):

    if method == 'GET':
        request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n'
    elif method == 'POST':
        if body is None:
            raise ValueError('Body must be provided for POST requests')
        request = f'POST {path} HTTP/1.1\r\nHost: {host}\r\nContent-Length: {len(body)}\r\n\r\n{body}'

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

# def make_post_request(host, port, path, data):
#     url = f'http://{host}:{port}{path}'
#     response = requests.post(url, data=data.encode('utf-8'))

#     print("Response Status Code:", response.status_code)
#     print("Response Content:")
#     print(response.text)


if __name__ == '__main__':
    host = '127.0.0.1'
    port = 1025
    #// g++ -o Server Server.cpp -pthread
    #cd MultiThreaded-HTTP-Server
    # Sending a GET request
    # make_request(host, port, 'GET', '/index.html', None)

    # Sending a POST request
    make_request(host, port, 'POST', 'output.txt', 'Hello from Python client')
    # path = '/output.txt'    # Update with the path you want to POST to
    # data = 'Hello from Python POST request'

    # make_post_request(host, port, path, data)