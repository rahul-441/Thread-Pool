#include <chrono>
#include <iostream>
#include <string>
#include "ThreadPool.h"
using namespace std;

int main() {
  ThreadPool pool(4);

  auto f = pool.submit([](int x) { return x * 2; }, 21);
  cout << "Result before shutdown: " << f.get() << endl;

  auto slow = pool.submit([]() {
    this_thread::sleep_for(chrono::milliseconds(200));
    return string("slow task finished");
  });
  pool.shutdown();
  cout << slow.get() << endl;

  try {
    pool.submit([]() { return 1; });
    cout << "ERROR: submit should have thrown" << endl;
  } catch (const runtime_error& e) {
    cout << "Rejected correctly: " << e.what() << endl;
  }

  pool.shutdown();
  cout << "Second shutdown OK" << endl;
}