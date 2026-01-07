#ifndef COMMON_H
#define COMMON_H

struct event_t {
  unsigned long long ts;
  unsigned long long seq_id;
  unsigned int pid;
  unsigned long long cookie_id;
};

#endif
