// ***********************************************************************
// Copyright (c) 2019 Dreame,All rights reserved.
// CLR Version      : 0.0.1
// Project          : Tools
// Assembly         : Tools
// Author           : Jimmy Teng
// Created          : 2019-08-28
// Description      : Async Timer
// ***********************************************************************
// Last Modified By : Jimmy Teng
// Last Modified On : 2019-08-28
// ***********************************************************************
#ifndef ASYNC_TIMER_HPP
#define ASYNC_TIMER_HPP
#include <cstdint>
#include <chrono>
#include <map>
#include <vector>
#include <mutex>
#include <iostream>
#include "p7/P7_Trace.h"
#include "p7/P7_Telemetry.h"
class AsyncTimer {
  AsyncTimer() = default;
 public:
  int Initialize(const std::string & str = "127.0.0.1") {
    stTelemetry_Conf l_stConf = {};

    P7_Set_Crash_Handler();

    //create P7 client object
    p_client_ = P7_Create_Client(TM(("/P7.Sink=Baical /P7.Addr=" + str).c_str()));

    if (NULL == p_client_) {
      goto l_lblExit;
    }

    l_stConf.pContext = NULL;
    l_stConf.pTimestamp_Callback = NULL;
    l_stConf.qwTimestamp_Frequency = 0ull;

    //create P7 telemetry object
    p_telemetry_ = P7_Create_Telemetry(p_client_, TM("Timer"), &l_stConf);
    if (NULL == p_telemetry_) {
      goto l_lblExit;
    }
    init_flg = true;
    return 0;
    l_lblExit:
    if (p_telemetry_) {
      p_telemetry_->Release();
      p_telemetry_ = NULL;
    }

    if (p_trace_) {
      p_trace_->Release();
      p_trace_ = NULL;
    }

    if (p_client_) {
      p_client_->Release();
      p_client_ = NULL;
    }

    return -1;
  }
  /// \brief GetInstance
  /// \return Instance Ptr
  static AsyncTimer *GetInstance() {
    static auto *instance = new AsyncTimer();
    return instance;
  }
  /// \brief Start Timer with timer id and call index
  /// \param id timer id
  /// \param index call index
  /// \return timestamp
  uint64_t Start(const std::string& name, uint32_t index) {
    if (!init_flg){
      return -1;
    }
    uint64_t id = 0;
    std::lock_guard<std::mutex> lock(mtx_);
    auto name_iter = name_id_map_.find(name);
    if(name_iter == name_id_map_.end()){
      id = name_id_map_.size();
      name_id_map_.insert(std::make_pair(name,id));
    } else{
      id = name_iter->second;
    }
    uint64_t timer_index = id << 32 | index;
    auto iter_send = id_send_map_.find(id);
    if (iter_send == id_send_map_.end()){
      uint16_t send_id;
      std::string topic = name + " (ms)";
      p_telemetry_->Create(TM(topic.c_str()), 0, 0, 1000, 1000, 1, &send_id);
      id_send_map_.insert(std::make_pair(id, send_id));
      std::cout<<"send_id created "<<send_id<<std::endl;
    }
    auto timestamp =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    start_time_map_.insert(std::make_pair(timer_index, timestamp));
    return timestamp;
  }
  /// \brief End Timer with timer id and call index
  /// \param id timer id
  /// \param index call index
  /// \param during timer during
  /// \param valid  timer valid
  /// \return timestamp
  uint64_t End(const std::string& name, uint32_t index, uint64_t &during, bool &valid) {
    if (!init_flg){
      return -1;
    }
    auto timestamp =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(mtx_);
    uint64_t id = 0;
    auto name_iter = name_id_map_.find(name);
    if(name_iter == name_id_map_.end()){
      valid = false;
      return timestamp;
    } else{
      id = name_iter->second;
    }
    auto iter = start_time_map_.find(id << 32 | index);
    if (iter == start_time_map_.end()) {
      valid = false;
      return timestamp;
    } else {
      valid = true;
      during = timestamp - iter->second;
    }
    auto iter_time_statistics = time_statistics.find(id);
    if (iter_time_statistics == time_statistics.end()) {
      time_statistics.insert(std::make_pair(id, std::vector<uint64_t>(1, during)));
    } else {
      iter_time_statistics->second.emplace_back(during);
    }
    auto send_iter = id_send_map_.find(id);
    if (send_iter != id_send_map_.end()) {
      std::cout<<"send_id add "<<send_iter->second<<"  "<<(tDOUBLE) (during/1000000)<<std::endl;
      p_telemetry_->Add(send_iter->second, (tDOUBLE) (during/1000000));
    }
    return timestamp;
  }
  /// \brief clear Timer
  /// \return
  void Clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    start_time_map_.clear();
    time_statistics.clear();
  }
  /// \brief Get All id time during
  /// \return  time_statistics
  const std::map<uint32_t, std::vector<uint64_t>> &GetTimeStatistics() {
    std::lock_guard<std::mutex> lock(mtx_);
    return time_statistics;
  }
  /// \brief Get time during with timer id
  /// \param index
  /// \return time_statistics
  std::vector<uint64_t> GetTimeStatistics(const std::string& name) {

    std::lock_guard<std::mutex> lock(mtx_);
    auto name_iter = name_id_map_.find(name);
    if(name_iter == name_id_map_.end()){
      return std::vector<uint64_t>();
    }
    return time_statistics[name_iter->second];
  }

 private:
  bool init_flg = false;
  std::mutex mtx_;
  std::map<std::string, uint32_t> name_id_map_;
  std::map<uint64_t, uint64_t> id_send_map_;
  std::map<uint64_t, uint64_t> start_time_map_;
  std::map<uint32_t, std::vector<uint64_t>> time_statistics;
  IP7_Client *p_client_ = nullptr;
  IP7_Trace *p_trace_ = nullptr;
  IP7_Telemetry *p_telemetry_ = nullptr;
};

#endif //ASYNC_TIMER_HPP