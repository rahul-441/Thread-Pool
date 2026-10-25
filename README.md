# Thread-Pool

A C++17 thread pool built from scratch using `std::thread`, `std::mutex`, `std::condition_variable`, and `std::future`. Header-only, with RAII shutdown.

## Features
- Fixed number of worker threads sharing one task queue
- `submit(f, args...)` accepts any callable and returns a `std::future` with the result
- Idle workers sleep on a `condition_variable` (no busy-waiting)
- Exceptions thrown inside a task are delivered through the `future`
- `shutdown()` finishes queued tasks, then joins all workers; the destructor calls it (RAII)
- `submit` after shutdown throws, so no future can hang forever

## Usage
```cpp
#include "ThreadPool.h"

ThreadPool pool(4);
auto f = pool.submit([](int x) { return x * 2; }, 21);
cout << f.get();   // 42
```

## Benchmark
CPU-bound task (count primes below 200,000), 16 tasks, median of 7 runs after a warm-up, `-O2`, 8 hardware threads, Windows x64. Results are verified identical.

| Mode | Median time | Speedup |
|---|---|---|
| Single-threaded | 285.0 ms | 1.0x |
| Thread pool (8 threads) | 67.2 ms | 4.24x |

Speedup is below 8x because `hardware_concurrency()` counts logical threads that share physical cores, and queueing and thread startup add overhead.

## Files
```
ThreadPool.h   the thread pool (header-only)
main.cpp       correctness test (submit, shutdown, rejected submit)
bench.cpp      benchmark vs single-threaded execution
```

## Build and run
Windows (Developer PowerShell for VS 2022):
```powershell
cl /EHsc /O2 /std:c++17 bench.cpp
.\bench.exe
```
Linux / WSL:
```bash
g++ -std=c++17 -O2 -pthread bench.cpp -o bench && ./bench
```

## Notes
- Tiny tasks can make the pool slower than single-threaded code, because locking and wakeups cost more than the work.
- No task priorities, work stealing, or dynamic resizing.