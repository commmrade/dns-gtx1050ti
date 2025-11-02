# DNS Request Tool

A learning project to explore and understand the DNS protocol by implementing DNS queries from scratch in C.

## 🎯 Purpose

This project is a hands-on exploration of the Domain Name System (DNS) protocol. Instead of using high-level APIs like `getaddrinfo()`, this implementation manually constructs DNS query packets, sends them over UDP, and parses the responses - providing deep insight into how DNS actually works under the hood.

## ✨ Features

- 🔧 **Manual DNS packet construction** - Build DNS queries byte-by-byte
- 📡 **UDP socket communication** - Direct communication with DNS servers
- 🌐 **IPv4 (A record) lookups** - Query and parse IPv4 addresses
- 🌍 **IPv6 (AAAA record) lookups** - Query and parse IPv6 addresses
- 📝 **DNS response parsing** - Decode DNS response packets manually
- 🎓 **Educational code** - Clear implementation for learning purposes

## 🚀 Building

This project uses CMake as its build system.

### Prerequisites

- CMake 3.30 or higher
- GCC or Clang compiler
- Linux/Unix-like operating system

### Build Steps

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Or simply use make
make
```

The executable will be created as `dns` in the build directory.

## 📖 Usage

```bash
./dns <v4|v6> <dns-server-ip> <domain-name>
```

### Arguments

- `<v4|v6>` - Specify record type: `v4` for A records (IPv4), `v6` for AAAA records (IPv6)
- `<dns-server-ip>` - IP address of the DNS server to query (e.g., `8.8.8.8` for Google DNS)
- `<domain-name>` - The domain name to resolve (e.g., `google.com`)

### Examples

Query Google DNS for the IPv4 address of google.com:
```bash
./dns v4 8.8.8.8 google.com
```

Query Cloudflare DNS for the IPv6 address of google.com:
```bash
./dns v6 1.1.1.1 google.com
```

Query custom DNS server:
```bash
./dns v4 9.9.9.9 example.com
```

### Sample Output

```
Address V4: 142.250.185.78
```

## 📁 Project Structure

```
dns_req/
├── src/
│   ├── main.c      # Entry point, socket setup, and command-line interface
│   ├── dns.c       # DNS packet construction and parsing logic
│   ├── dns.h       # DNS structures and constants
│   ├── utils.c     # Helper functions (domain name encoding)
│   └── utils.h     # Utility function declarations
├── CMakeLists.txt  # Build configuration
├── LICENSE.md      # GPL v3 license
└── README.md       # This file
```

## 🔬 Technical Details

### DNS Packet Structure

This implementation handles the following DNS packet components:

1. **DNS Header** (12 bytes)
   - Transaction ID
   - Flags (QR, Opcode, AA, TC, RD, RA, RCODE)
   - Question count, Answer count, Authority count, Additional count

2. **Question Section**
   - QNAME (encoded domain name)
   - QTYPE (A for IPv4, AAAA for IPv6)
   - QCLASS (always 1 for Internet)

3. **Answer Section** (in responses)
   - NAME (domain name)
   - TYPE (resource record type)
   - CLASS (typically 1 for Internet)
   - TTL (time to live)
   - RDLENGTH (length of resource data)
   - RDATA (the actual IP address or other data)

### Domain Name Encoding

Domain names are encoded using DNS label format:
- Each label is prefixed with its length
- Labels are separated by their length bytes
- Terminated with a zero-length byte

Example: `google.com` → `\x06google\x03com\x00`

### Supported Record Types

- **Type 1 (A)**: IPv4 address records
- **Type 28 (AAAA)**: IPv6 address records

## ⚠️ Current Limitations

- ❌ **DNS compression not supported** - Compressed domain names in responses will cause errors
- ❌ **Single question only** - Only sends one question per query
- ❌ **Limited record types** - Only A and AAAA records are parsed
- ❌ **No DNSSEC** - No validation of DNS responses
- ❌ **No retry logic** - If a packet is lost, the query fails
- ❌ **IPv4 DNS servers only** - Uses AF_INET for the server connection

## 📚 Learning Resources

This project was built using knowledge from:

- [DNS Primer - Duke University](https://courses.cs.duke.edu/fall16/compsci356/DNS/DNS-primer.pdf)
- [DNS Message Format - GeeksforGeeks](https://www.geeksforgeeks.org/computer-networks/dns-message-format/)
- RFC 1035 - Domain Names - Implementation and Specification

## 🛠️ Future Improvements

Potential enhancements for learning:

- [ ] Add DNS compression support
- [ ] Implement more record types (MX, TXT, CNAME, etc.)
- [ ] Add IPv6 DNS server support
- [ ] Implement iterative DNS resolution
- [ ] Add caching mechanism
- [ ] Better error handling and diagnostics
- [ ] Support for multiple questions in one query

## 📝 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE.md](LICENSE.md) file for details.

## 🎓 Educational Note

This is a learning project created to understand the DNS protocol at a low level. It is **not** intended for production use. For real applications, use the standard library functions like `getaddrinfo()` or robust DNS libraries.
