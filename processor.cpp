// main.cpp
#include <chrono>
#include <iostream>
#include <sys/sdt.h>
#include <thread>
#include <unistd.h>

void process_buffer(unsigned long seq_id) {
  DTRACE_PROBE1(buffer_tracker, beg_awe, seq_id);
  std::this_thread::sleep_for(std::chrono::milliseconds(15));
  DTRACE_PROBE1(buffer_tracker, end_awe, seq_id);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  DTRACE_PROBE1(buffer_tracker, beg_writing, seq_id);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  DTRACE_PROBE1(buffer_tracker, end_writing, seq_id);
}

int main() {
  unsigned long seq_id = 0;
  for (std::size_t i = 0; i < 10000; i++) {
    process_buffer(seq_id);
    seq_id++;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return 0;
}
