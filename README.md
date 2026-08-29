# raw-dns-cli

A minimal DNS CLI tool for educational purposes. Look up DNS records from the command line with a clean table output, hex dump mode, and customizable DNS servers.

![C Language](https://img.shields.io/badge/C-11-ff69b4?style=for-the-badge&logo=c)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)

## Features

- 🔍 **A/AAAA/CNAME record resolution** – Look up IPv4, IPv6, and alias records
- 📦 **Hex dump mode** (`-x`) – View raw DNS packets in hexadecimal format
- 🌐 **Custom DNS server** – specify any resolver (default: 1.1.1.1)
- 🎨 **Color-coded output** – Green for A records, blue for AAAA, yellow for CNAME
- 📋 **Table formatting** – Pretty-printed results with unicode box-drawing characters
- 🛠️ **Simple CLI** – `dns_cli domain [dns-server-ip]`

## Installation

### Standard

```bash
# Clone the repository
git clone https://github.com/by-cortex/raw-dns-cli.git
cd raw-dns-cli

# Build with make
make

# Or compile manually
gcc -Wall -Wextra -Werror -pedantic -std=c11 -Iinclude src/*.c -o dns_cli
```

### If you use Nix:

```bash
nix-shell
make
```

## Usage

```bash
# Basic A record lookup
./dns_cli google.com

# With hex dump
./dns_cli -x google.com

# Custom DNS server with hex dump
./dns_cli -x google.com 8.8.8.8

# Show help
./dns_cli -h
```
## Testing

The project includes a Python mock DNS server (`tests/mock_server.py`) to test network timeouts, retry logic, and edge cases locally without external network dependencies.

### 1. Run the mock server (Terminal 1)

```bash
# Standard mock server (listens on 127.0.0.1:5353)
make mock-server

# Or run with specific test modes
python3 tests/mock_server.py --mode drop-twice --port 5353
# Or:
make mock-server ARGS="--mode drop-twice"

```

Available mock modes:

* `normal` – Returns a valid DNS answer (`1.2.3.4`)
* `drop-twice` – Drops the first 2 incoming packets to test retry/timeout logic
* `bad-id` – Corrupts the Transaction ID to test packet validation
* `nxdomain` – Returns an `NXDOMAIN` (RCODE 3) error response

### 2. Test the CLI (Terminal 2)

```bash
# Query the local mock server
./dns_cli -p 5353 example.com 127.0.0.1

```

## Screenshots

![Basic output](screenshots/basic.jpg)

*Standard A record resolution for google.com*

![Hex dump](screenshots/hex.jpg)

*Raw DNS packet hex dump with -x flag*

![Help output](screenshots/help.jpg)

*Usage information with -h flag*

## Project Structure

```
raw-dns-cli/
├── include/          # Header files (args.h, domain.h, network.h, table_print.h)
├── src/              # Source code (args.c, domain.c, main.c, network.c, table_print.c)
├── tests/            # Python DNS mock server for local testing
├── screenshots/      # Example output images for documentation
├── Makefile          # Build automation
├── shell.nix         # Nix development environment
├── LICENSE           # MIT License
└── README.md         # Project documentation
```

## How it works

1. **Query construction** (`main.c` + `network.c`): Builds a standard DNS query with the domain name encoded in wire format, transaction ID, and query type/class
2. **UDP send** (`send_query`): Sends the query via SOCK_DGRAM to the specified DNS server on port 53
3. **Response parsing** (`parse_recv`): Parses the DNS response, extracts answer records, and handles name compression pointers
4. **Display** (`table_print`): Renders results in a color-coded table with proper formatting

## License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

## TODO / Future improvements

- [x] Basic A record resolution
- [x] AAAA/CNAME record types
- [x] Hex dump mode
- [x] Color-coded output
- [x] Socket timeout & retries (-t / --timeout)
- [ ] EDNS support
- [ ] TCP fallback for large responses
- [ ] Async/non-blocking I/O
- [ ] Additional record types (MX, TXT, SRV)
- [ ] JSON output option (--json)
- [ ] Reverse DNS lookup (PTR records)
