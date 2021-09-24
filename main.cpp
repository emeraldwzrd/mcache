#include <iostream>
#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include "mcache.hpp"
#include <thread>
#include "timer.hpp"

using namespace std;

void addWave(MCache<string, int> *mc, int start, int finish) {
  string baseString;
  for(int i = start; i < finish; i++) {
    baseString = "hello" + to_string(i);
    if(i % 2 == 0) {
      mc->insert(baseString, i, 120);
    } else {
      mc->insert(baseString, i);
    }
  }
}

void removeWave(MCache<string, int> *mc, int start, int finish) {
  string baseString;
  for(int i = start; i < finish; i++) {
    baseString = "hello" + to_string(i);
    mc->remove(baseString);
  }
}

int main() {
  auto *mc = new MCache<string, int>();
  const int max = 10000000;
  thread tAdd1(addWave, mc, 0, max / 4); 
  thread tAdd2(addWave, mc, max / 4, max / 3);
  thread tAdd3(addWave, mc, max / 3, max / 2); 
  thread tAdd4(addWave, mc, max / 2, max); 
  thread tRemove1(removeWave, mc, 550000, 1500000); 
  thread tRemove2(removeWave, mc, 7500000, 8900000); 
  tAdd1.join();
  tAdd2.join();
  tAdd3.join();
  tAdd4.join();
  tRemove1.join();
  tRemove2.join();
  string baseString;
  vector<unsigned int> times;
  thread tAdd5(addWave, mc, 550000, 1500000); 
  thread tAdd6(addWave, mc, 7500000, 8900000); 
  tAdd5.join();
  tAdd6.join();
  for(int i = 0; i < max; i++) {
    int* test  = nullptr;
    baseString = "hello" + to_string(i);
    unsigned int latency = 0;
    {
      Timer timer(&latency);
      test = mc->get(baseString);
    }
    if(test != nullptr) {
      if(*test != i) {
         cout << "Wrong value! Expected: " << i << "Retrieved: " << *test << endl;
      }
      times.push_back(latency);
    } else {
      cout << "Could not retrieve expected key/value pair!: " << i << endl;
    }
  }
  sort(times.begin(), times.end()); 
  unsigned int ninetyFifth = times.size() * .95;
  unsigned int ninetyNinth = times.size() * .99;
  cout << "Size: " <<  times.size() << endl;
  cout << "95th percentile: " <<  times[ninetyFifth] << " nanoseconds" << endl;
  cout << "99th percentile: " <<  times[ninetyNinth] << " nanoseconds" << endl;
  cout << "Waiting for timed cache to finish..." << endl;
  this_thread::sleep_for(chrono::seconds(180));
  cout << "Size of cache after timed cache was supposed to remove ~half the entries: " << mc->getSize() << endl;
  cout << "Remaining entries in timed cache: " << mc->getTimedQueueSize() << endl;
}