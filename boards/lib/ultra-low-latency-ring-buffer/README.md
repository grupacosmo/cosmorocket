# SPSC Lock-Free Ring Buffer (HFT Optimized)

A high-performance **Single Producer Single Consumer (SPSC) lock-free ring buffer**, optimized for **low-latency** and **high-throughput** workloads such as **high-frequency trading (HFT), networking, and real-time systems**.

This project demonstrates:
- Lock-free communication between threads  
- Cache-aware memory layout  
- Memory prefaulting to avoid runtime page faults  
- Thread pinning & priority tuning for latency-sensitive workloads  
- Benchmark suite for latency and throughput analysis  

---

## ✨ Features
- **Lock-free SPSC ring buffer** with predictable performance  
- **Cache line–aligned data structures** to minimize false sharing  
- **Memory prefaulting** to eliminate first-touch page faults  
- **Thread affinity & priority tuning** for deterministic benchmarking  
- **Comprehensive benchmark suite** (latency, throughput, contention)  

---

## 🖥 Benchmark Results (Windows, 16 cores)

**System Information**  
- CPU Cores: 16  
- Available Memory: 4950 MB  
- Ring Buffer Size: ~1.5 MB (~384 pages)  
- Message Size: 24 bytes  
- Messages per cache line: 2  

### Single-Threaded Latency
| Metric      | Value     |
|-------------|-----------|
| Average     | 19.5 ns   |
| p95         | 100 ns    |
| p99         | 100 ns    |
| Max         | 505 μs    |

### Producer–Consumer Throughput
| Metric      | Value             |
|-------------|-------------------|
| Operations  | 1,000,000,000     |
| Duration    | 9376 ms           |
| Throughput  | **106.6 M ops/s** |
| Avg Latency | 9.4 ns            |

### Contention Test
- Impact: **-0.5% slower** under contention  
- Latency distribution remains stable up to p99.9  

---

## 📊 Performance Summary
- **Best case latency:** ~0 ns (cache hit / warm path)  
- **Average latency:** ~20 ns  
- **Max throughput:** **~106 million ops/sec**  
- **Stable under contention**  

---

## ⚙️ Technical Details
- Implemented in **C++17** (portable, no external deps)  
- Uses **memory barriers** and `std::atomic` for correctness  
- **Prefaults memory pages** on startup to ensure predictable runtime  
- Supports **thread pinning** for NUMA / CPU locality optimization  
- Benchmark suite uses **high-resolution timers**  

---
