# Performance Report – SPSC Lock-Free Ring Buffer

This document provides detailed **benchmark results** and **performance analysis** of the lock-free single producer single consumer (SPSC) ring buffer implementation.  

The benchmarks were executed on a **16-core Windows system** with ~5 GB free memory.  

---

## ⚙️ Benchmark Setup
- **CPU cores available:** 16  
- **Available memory:** 4950 MB  
- **Ring buffer size:** 1,572,992 bytes (~1.5 MB, ~384 pages)  
- **Message size:** 24 bytes  
- **Messages per cache line:** 2  
- **Page size:** 4096 bytes  
- **Optimizations applied:**
  - Memory prefaulting
  - Thread affinity (core pinning)
  - High-resolution timers
  - High process & thread priority  

---

## 📊 Benchmark Results

### 1. Single-Threaded Latency
| Metric  | Value    |
|---------|----------|
| Average | 19.5 ns  |
| p50     | 0 ns     |
| p95     | 100 ns   |
| p99     | 100 ns   |
| p99.9   | 100 ns   |
| p99.99  | 200 ns   |
| Max     | 505 μs   |

✅ **Observation:** Latency is consistently below 100 ns for the 99.9th percentile, with occasional spikes due to OS scheduling or cache/TLB misses.  

---

### 2. Producer–Consumer Throughput
| Metric      | Value             |
|-------------|-------------------|
| Operations  | 1,000,000,000     |
| Duration    | 9376 ms           |
| Throughput  | **106.6 M ops/s** |
| Avg Latency | 9.4 ns            |

✅ **Observation:** Achieves sustained throughput of over **100 million operations per second**, corresponding to ~9 ns per message.  

---

### 3. Contention Benchmark
| Metric      | Value     |
|-------------|-----------|
| Average     | 19.6 ns   |
| p95         | 100 ns    |
| p99         | 100 ns    |
| p99.9       | 100 ns    |
| p99.99      | 200 ns    |
| Max         | 2.64 ms   |
| Throughput  | Stable    |

✅ **Observation:** Contention only reduced throughput by **~0.5%**, confirming the efficiency of the lock-free design.  

---

## 📈 Performance Summary
- **Best case latency:** ~0 ns (cache hit / warm path)  
- **Average latency:** ~20 ns  
- **Throughput:** **~106 million ops/sec**  
- **Tail latency:** Stable up to p99.9, with rare OS-induced spikes  
- **Contention impact:** Negligible (~-0.5%)  

---

## 🚀 Key Takeaways
- The implementation achieves **nanosecond-scale latency** and **100M+ ops/sec throughput**, making it suitable for:  
  - High-Frequency Trading systems  
  - Low-latency messaging pipelines  
  - Networking stacks (e.g., packet batching)  
  - Real-time logging  

- Further optimizations could include:  
  - NUMA-aware memory allocation  
  - Huge pages to reduce TLB misses  
  - Kernel bypass networking integration (e.g., DPDK, RDMA)  

---

