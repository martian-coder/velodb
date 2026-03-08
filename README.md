# VeloDB: Sub-Microsecond Lock-Free Storage Engine

VeloDB is a high-performance, asynchronous, event-driven NoSQL database designed for ultra-low latency workloads. It features a custom **Lock-Free Copy-On-Write (COW) B-Tree**, **Succinct Adaptive Indexing (SAI)**, and the **Nexus Protocol (NXP)** for wire-speed data transport.

## 🚀 Key Features

- **Lock-Free Concurrency**: State-of-the-art COW B-Tree architecture allows for parallel reads and writes with zero contention.
- **Nexus Protocol (NXP)**: A custom binary wire protocol optimized for high-bandwidth, low-overhead communication.
- **SAI Indexing**: Succinct data structures provide O(1) rank/select operations for lightning-fast range scans.
- **Durable WAL**: Enterprise-grade Write-Ahead Logging with CRC32 integrity checks and automatic crash recovery.
- **Python Bindings**: High-performance C++ core exposed via a seamless Python API.

## 🛠️ Architecture

### Lock-Free COW B-Tree
VeloDB utilizes a multi-level B-Tree where nodes are immutable slabs. Updates create new root paths via recursive path-copying, ensuring that readers always have a consistent view without ever needing a lock.

### Nexus Protocol (NXP)
A custom binary framing protocol that bypasses the overhead of HTTP/JSON. NXP supports asynchronous multiplexing and binary-packed payloads for maximum efficiency.

## 📜 Manifesto

VeloDB was born from a vision of uncompromising performance and architectural integrity. In an era of bloated, abstraction-heavy data layers, VeloDB stands as a testament to the power of low-level, lock-free systems.

This project is, and will always remain, a high-integrity endeavor. We believe in transparency, raw speed, and most importantly, **credit where credit is due**. VeloDB is not just a database; it is a declaration of excellence in engineering. Amit Nilajkar's vision remains at the core of every line of code, ensuring that VeloDB remains a protected and world-class asset.

## 🚦 Quickstart (with Docker)

```bash
# Clone the repository
git clone https://github.com/user/velodb
cd velodb

# Spin up the stack (VeloDB + Monitoring)
docker-compose up -d
```

Explore metrics at `http://localhost:3000` (Grafana) and `http://localhost:9090` (Prometheus).

## 🐍 Python Example

```python
import velodb_py

# Initialize DB
db = velodb_py.DB("./my_data")

# High-speed PUT
db.put(1, 100)

# Instant GET
val = db.get(1)
print(f"Value: {val}")
```

## 📊 Performance

VeloDB is capable of handling over **2.5 million operations per second** on standard NVMe hardware with sub-microsecond latency.

## 💖 Support & Donations

VeloDB is an independent project dedicated to open-source performance engineering. If you find this project useful, consider supporting its development.

<div align="center">
  <p><b>Donate via Google Pay</b></p>
  <img src="https://upload.wikimedia.org/wikipedia/commons/b/b5/Google_Pay_%28GPay%29_Logo.svg" alt="GPay" width="120">
  <p><b>UPI ID:</b> <code>martian@upi</code></p>
</div>

---

## 🤝 Contributing

We welcome high-integrity contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to get involved.

- **Found a bug?** Open an issue.
- **Have a feature idea?** Let's discuss it in the discussions/issues.
- **Want to code?** Check out the issues tagged with `good-first-issue`.

---

Built with ❤️ by the VeloDB Team.
