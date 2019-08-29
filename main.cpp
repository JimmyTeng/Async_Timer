// ***********************************************************************
// Copyright (c) 2019 Dreame,All rights reserved.
// CLR Version      : 0.0.1
// Project          : Tools
// Assembly         : Tools
// Author           : Jimmy Teng
// Created          : 2019-08-28
// Description      : Async Timer Example
// ***********************************************************************
// Last Modified By : Jimmy Teng
// Last Modified On : 2019-08-28
// ***********************************************************************
#include "async_timer.hpp"
#include <iostream>
#include <map>
#include <thread>
#include <random>
const std::string id = "sleep 1 - 1000";
const std::string id1= "sleep 1 - 500";
constexpr int thread_count = 2000;
int Sleep(uint32_t index) {
  std::random_device rd;  // 将用于为随机数引擎获得种子
  std::mt19937 gen(rd()); // 以播种标准 mersenne_twister_engine
  std::uniform_int_distribution<> dis(1, 1000);
  AsyncTimer::GetInstance()->Start(id, index);
  std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
  bool flg;
  uint64_t during;
  AsyncTimer::GetInstance()->End(id, index, during, flg);
  return 0;
}
int Sleep1(uint32_t index) {
  std::random_device rd;  // 将用于为随机数引擎获得种子
  std::mt19937 gen(rd()); // 以播种标准 mersenne_twister_engine
  std::uniform_int_distribution<> dis(1, 500);
  AsyncTimer::GetInstance()->Start(id1, index);
  std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
  bool flg;
  uint64_t during;
  AsyncTimer::GetInstance()->End(id1, index, during, flg);
  return 0;
}

int main() {
  std::cout<<sizeof(size_t);
  AsyncTimer::GetInstance()->Initialize();
  std::vector<std::thread> t;
  for (int i = 0; i < thread_count; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    t.emplace_back(Sleep1, i);
    t.emplace_back(Sleep, i);
  }

  for (int i = 0; i < thread_count*2; ++i) {
    t[i].join();
  }
  for (auto &during :AsyncTimer::GetInstance()->GetTimeStatistics(id)) {
    std::cout << during << std::endl;
  }
  for (auto &during :AsyncTimer::GetInstance()->GetTimeStatistics(id1)) {
    std::cout << during << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}