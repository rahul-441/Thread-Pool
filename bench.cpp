#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
#include "ThreadPool.h"
using namespace std;

// CPU-heavy work: count primes below n
int countPrimes(int n) {
  int count = 0;
  for (int i = 2; i < n; i++) {
    bool prime = true;
    for (int j = 2; j * j <= i; j++) {
      if (i % j == 0) { prime = false; break; }
    }
    if (prime) count++;
  }
  return count;
}

long long runSingle(int tasks, int limit) {
  long long total = 0;
  for (int i = 0; i < tasks; i++) total += countPrimes(limit);
  return total;
}

long long runPool(int threads, int tasks, int limit) {
  long long total = 0;
  ThreadPool pool(threads);
  vector<future<int>> futures;
  for (int i = 0; i < tasks; i++) futures.push_back(pool.submit(countPrimes, limit));
  for (auto& f : futures) total += f.get();
  return total;
}

template <typename F>
double timeMs(F fn, long long& out) {
  auto s = chrono::steady_clock::now();
  out = fn();
  auto e = chrono::steady_clock::now();
  return chrono::duration<double, milli>(e - s).count();
}

double median(vector<double> v) {
  sort(v.begin(), v.end());
  return v[v.size() / 2];
}

int main() {
  const int TASKS = 16, LIMIT = 200000, RUNS = 7;
  unsigned hw = thread::hardware_concurrency();
  int n = hw == 0 ? 4 : static_cast<int>(hw);

  long long t1 = 0, t2 = 0;
  vector<double> single, pooled;

  // warm-up
  runSingle(TASKS, LIMIT);
  runPool(n, TASKS, LIMIT);

  for (int i = 0; i < RUNS; i++) {
    single.push_back(timeMs([&] { return runSingle(TASKS, LIMIT); }, t1));
    pooled.push_back(timeMs([&] { return runPool(n, TASKS, LIMIT); }, t2));
  }

  double s = median(single), p = median(pooled);
  cout << "Threads in pool : " << n << "\n";
  cout << "Results match   : " << (t1 == t2 ? "yes" : "NO") << "\n";
  cout << "Single median   : " << s << " ms\n";
  cout << "Pool median     : " << p << " ms\n";
  cout << "Speedup         : " << s / p << "x\n";
}