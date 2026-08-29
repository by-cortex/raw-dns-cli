import argparse
import socket
import struct


def run_mock_server(port: int, mode: str) -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("127.0.0.1", port))
    print(f"[MOCK] Server running on 127.0.0.1:{port} with {mode} mode...")

    request_count = 0

    while True:
        data, addr = sock.recvfrom(512)
        request_count += 1

        print(f"[MOCK] Got req #{request_count} ({len(data)} bytes) from {addr}")

        if mode == "drop-twice" and request_count <= 2:
            print(f"[MOCK] Dropping request #{request_count} (simulating loss)...")
            continue

        if mode == "bad-id":
            responce = bytearray(data)
            responce[0] ^= 0xFF
            sock.sendto(responce, addr)
            continue

        if mode == "nxdomain":
            responce = bytearray(data)
            responce[2] |= 0x80  # QR bit = 1 (Responce)
            responce[3] |= 0x03  # RCODE 3 (NXDOMAIN)
            sock.sendto(responce, addr)
            continue

        responce = create_dns_responce(data)
        sock.sendto(responce, addr)

def create_dns_responce(data: bytes, ip_address: str = "1.2.3.4") -> bytes:
    tx_id = data[:2]
    header = tx_id + struct.pack("!HHHHH", 0x8180, 1, 1, 0, 0)
    question = data[12:]
    answer_meta = struct.pack("!HHHIH", 0xC00C, 1, 1, 60, 4)
    ip_bytes = socket.inet_aton(ip_address)
    return header + question + answer_meta + ip_bytes


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", type=int, default=5353)
    parser.add_argument(
        "--mode",
        choices=["normal", "drop-twice", "bad-id", "nxdomain"],
        default="normal",
    )
    args = parser.parse_args()

    run_mock_server(args.port, args.mode)
