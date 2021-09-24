#include <iostream>
#include <memory>
#include <chrono>
using namespace std;
// As inspired by Cherno
class Timer {
private:
chrono::time_point<chrono::high_resolution_clock> startTime;
unsigned int *latency;
public:
  Timer(unsigned int *l) : latency(l) {
    startTime = chrono::high_resolution_clock::now();
  }
  ~Timer() {
    Stop();
  }
  void Stop() {
    auto endTime= chrono::high_resolution_clock::now();
    auto start = chrono::time_point_cast<chrono::nanoseconds>(startTime).time_since_epoch().count();
    auto end = chrono::time_point_cast<chrono::nanoseconds>(endTime).time_since_epoch().count();
    auto duration = end - start;  
    *latency = duration;
  }
};