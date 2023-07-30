import socket
import time

def connect_and_send(host, port, message):
    connected = False
    
    while not connected:
        try:
            # Create a socket object
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            
            # Connect to the server
            sock.connect((host, port))
            print(f"Connected to {host}:{port}")
            connected = True
            
            # Send the message
            sock.sendall(message.encode())
            print("Message sent:", message)
            
            # Receive and print the response
            response = sock.recv(1024).decode()
            print("Response received:", response)
            
        except ConnectionRefusedError:
            print(".")
            time.sleep(.1)
            
        finally:
            # Close the socket
            sock.close()

# Usage example
connect_and_send("localhost", 5002, "Hello, server!")