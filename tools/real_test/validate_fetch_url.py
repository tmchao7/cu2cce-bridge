#!/usr/bin/env python3
"""Security validator for the external fetch URL host of cu2cce-bridge.

Used by fetch_external_cuda_samples.sh before any server-side network fetch.
Only http/https URLs are allowed (scheme/authority parsing happens in the
shell); this tool validates the host component and rejects loopback, private,
link-local, multicast, reserved, and other special-use addresses, including
hostnames that resolve to such addresses.

Exit code contract:
  0  host is forbidden (reject the URL)
  1  host is allowed
"""
from __future__ import annotations

import ipaddress
import signal
import socket
import sys

# IPv4 special-use ranges (RFC 6890 plus documentation/benchmark networks) as
# (mask, base). The <= comparison form keeps the table readable.
_IPV4_BLOCKED: list[tuple[int, int]] = [
    (0xFF000000, 0x00000000),   # 0.0.0.0/8 "this network"
    (0xFF000000, 0x0A000000),   # 10.0.0.0/8 private
    (0xFFC00000, 0x64400000),   # 100.64.0.0/10 carrier-grade NAT
    (0xFF000000, 0x7F000000),   # 127.0.0.0/8 loopback
    (0xFFFF0000, 0xA9FE0000),   # 169.254.0.0/16 link-local
    (0xFFF00000, 0xAC100000),   # 172.16.0.0/12 private
    (0xFFFFFF00, 0xC0000000),   # 192.0.0.0/24 special use
    (0xFFFFFF00, 0xC0000200),   # 192.0.2.0/24 TEST-NET-1 documentation
    (0xFFFFFF00, 0xC0586300),   # 192.88.99.0/24 6to4 relay anycast
    (0xFFFF0000, 0xC0A80000),   # 192.168.0.0/16 private
    (0xFFFE0000, 0xC6120000),   # 198.18.0.0/15 benchmarking
    (0xFFFFFF00, 0xC6336400),   # 198.51.100.0/24 TEST-NET-2 documentation
    (0xFFFFFF00, 0xCB007100),   # 203.0.113.0/24 TEST-NET-3 documentation
    (0xF0000000, 0xE0000000),   # 224.0.0.0/4 multicast
    (0xF0000000, 0xF0000000),   # 240.0.0.0/4 reserved
]

# Hostnames that are special-use by construction and must never be fetched.
_SPECIAL_USE_HOST_SUFFIXES = (
    ".localhost",
    ".local",
    ".internal",
    ".home.arpa",
    ".lan",
    ".localdomain",
    ".onion",
)


def _v4_blocked(n: int) -> bool:
    if n == 0xFFFFFFFF:  # limited broadcast
        return True
    return any((n & mask) == base for mask, base in _IPV4_BLOCKED)


def _v6_blocked(ip: ipaddress.IPv6Address) -> bool:
    n = int(ip)
    if ip.ipv4_mapped is not None:
        return _v4_blocked(int(ip.ipv4_mapped))
    # fc00::/7 unique-local addressing (covering fc00:.. and fd00:..).
    if (n >> 121) == 0x7E:
        return True
    # 2001:db8::/32 documentation.
    if (n >> 96) == 0x20010DB8:
        return True
    # 64:ff9b::/96 well-known NAT64 prefix.
    if (n >> 32) == 0x64FF9B00:
        return True
    return False


def forbidden(host: str) -> bool:
    """Return True when *host* must not be fetched."""
    host = host.strip().rstrip(".").lower()
    if not host:
        return True
    try:
        address = ipaddress.ip_address(host)
    except ValueError:
        address = None
    if address is not None:
        if address.is_loopback or address.is_link_local or address.is_multicast \
                or address.is_reserved or address.is_unspecified or address.is_private:
            return True
        if address.version == 4:
            return _v4_blocked(int(address))
        return _v6_blocked(address)

    # Hostname: special-use suffixes are unconditionally forbidden.
    if host == "localhost" or host.endswith(_SPECIAL_USE_HOST_SUFFIXES):
        return True

    # Best-effort DNS check: reject only on a positive match. Resolution
    # failure is not a rejection reason (offline/proxy environments would
    # otherwise break the fetch for legitimate hosts).
    infos: list[tuple] = []
    signal.signal(signal.SIGALRM, _dns_timeout)
    signal.alarm(5)
    try:
        infos = socket.getaddrinfo(host, None, type=socket.SOCK_STREAM)
    except OSError:
        infos = []
    finally:
        signal.alarm(0)
    for info in infos:
        try:
            candidate = ipaddress.ip_address(info[4][0])
        except ValueError:
            continue
        if forbidden(str(candidate)):
            return True
    return False


def _dns_timeout(signum: int, frame: object) -> None:  # noqa: ARG001
    raise TimeoutError("host resolution timed out")


def main(argv: list[str]) -> int:
    if len(argv) != 1:
        print("usage: validate_fetch_url.py HOST", file=sys.stderr)
        return 2
    return 0 if forbidden(argv[0]) else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))