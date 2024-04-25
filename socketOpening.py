import socket
import sys

HOST = "127.0.0.1"
PORT = int( sys.argv[1] )

while True:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind( (HOST, PORT) )
            print(f"Servidor escuchando en: {HOST} - {PORT}")
            s.listen()
            while True:
                conn, addr = s.accept()
                with conn:
                    print(f"Conexion activa: {addr}")
                    data = conn.recv(1024)
                    if not data:
                        break
                    print(f"Data recibida: { data }")
    except ConnectionResetError:
        print (f"Connection reset by peer")
        continue

    except Exception as e:
        print("Error:", e)
        break
