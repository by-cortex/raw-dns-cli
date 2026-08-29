import socket
import time

LISTEN_IP = "127.0.0.1"
LISTEN_PORT = 5353

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((LISTEN_IP, LISTEN_PORT))

print(f"DNS Mock Server running on {LISTEN_IP}:{LISTEN_PORT}...")

mode = "delay"

while True:
    data, addr = sock.recvfrom(512)
    print(f"\n[+] Received {len(data)} bytes from {addr}")
    print(f"HEX:", data.hex())

    if mode == "timeout":
        print("[!] Ignoring packet to trigger client timeout...")
        continue

    elif mode == "delay":
        print("[!] Delaying response for 3 seconds...")
        time.sleep(3)

    responce = bytearray(data)
    responce[2] |= 0x80  # QR

    sock.sendto(responce, addr)
