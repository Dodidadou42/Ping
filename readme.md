# ft_ping

A high-performance C implementation of the standard ping utility, built from scratch using Raw Sockets and the ICMP protocol.

## Overview

ft_ping reproduces the core behavior of iputils, sending ICMP ECHO_REQUESTs and analyzing responses to monitor network latency and reliability. This project focuses on low-level network programming, precise timing, and robust packet parsing.

## Key Features

Raw ICMP Handling: Manually crafts and parses ICMP packets using SOCK_RAW.

Zero-Drift Loop: Implements an absolute deadline timing system (via pselect) to maintain a perfect 1-second interval, regardless of system or processing delays.

Smart Error Inspection: Parses deep ICMP error payloads (TTL Exceeded, Unreachable) and verifies them against the process PID to ensure accuracy.

Precise Statistics: Live calculation of RTT (min/avg/max) and Mean Deviation (mdev) using the root-mean-square method.

Full CLI: Includes support for -v (verbose) and --help options.

## Usage

_Note: Requires root privileges to open Raw Sockets._

```bash
make
sudo ./ft_pin
```
