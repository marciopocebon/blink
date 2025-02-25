#ifndef BLINK_TIMESPEC_H_
#define BLINK_TIMESPEC_H_
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>

#include "blink/assert.h"

#define MAX_INTEGRAL_TIME    \
  (((time_t) ~(time_t)0) > 1 \
       ? (time_t) ~(time_t)0 \
       : (time_t)((((uintmax_t)1) << (sizeof(time_t) * CHAR_BIT - 1)) - 1))

static inline struct timespec GetTime(void) {
  struct timespec ts;
  unassert(!clock_gettime(CLOCK_REALTIME, &ts));
  return ts;
}

static inline struct timespec GetMaxTime(void) {
  struct timespec ts;
  ts.tv_sec = MAX_INTEGRAL_TIME;
  ts.tv_nsec = 999999999;
  return ts;
}

static inline struct timespec GetZeroTime(void) {
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  return ts;
}

static inline struct timespec FromSeconds(time_t x) {
  struct timespec ts;
  ts.tv_sec = x;
  ts.tv_nsec = 0;
  return ts;
}

static inline struct timespec FromMilliseconds(time_t x) {
  struct timespec ts;
  ts.tv_sec = x / 1000;
  ts.tv_nsec = x % 1000 * 1000000;
  return ts;
}

static inline struct timespec FromMicroseconds(time_t x) {
  struct timespec ts;
  ts.tv_sec = x / 1000000;
  ts.tv_nsec = x % 1000000 * 1000;
  return ts;
}

static inline struct timespec FromNanoseconds(time_t x) {
  struct timespec ts;
  ts.tv_sec = x / 1000000000;
  ts.tv_nsec = x % 1000000000;
  return ts;
}

static inline int CompareTime(struct timespec a, struct timespec b) {
  int cmp;
  if (!(cmp = (a.tv_sec > b.tv_sec) - (a.tv_sec < b.tv_sec))) {
    cmp = (a.tv_nsec > b.tv_nsec) - (a.tv_nsec < b.tv_nsec);
  }
  return cmp;
}

static inline struct timespec AddTime(struct timespec x, struct timespec y) {
  x.tv_sec += y.tv_sec;
  x.tv_nsec += y.tv_nsec;
  if (x.tv_nsec >= 1000000000) {
    x.tv_nsec -= 1000000000;
    x.tv_sec += 1;
  }
  return x;
}

static inline struct timespec SubtractTime(struct timespec a,
                                           struct timespec b) {
  a.tv_sec -= b.tv_sec;
  if (a.tv_nsec < b.tv_nsec) {
    a.tv_nsec += 1000000000;
    a.tv_sec--;
  }
  a.tv_nsec -= b.tv_nsec;
  return a;
}

static inline struct timespec SleepTime(struct timespec dur) {
  struct timespec unslept;
  if (!nanosleep(&dur, &unslept)) {
    unslept.tv_sec = 0;
    unslept.tv_nsec = 0;
  } else {
    unassert(errno == EINTR);
  }
  return unslept;
}

static inline time_t ToSeconds(struct timespec ts) {
  unassert(ts.tv_nsec < 1000000000);
  if (ts.tv_sec < MAX_INTEGRAL_TIME) {
    if (ts.tv_nsec) {
      ts.tv_sec += 1;
    }
    return ts.tv_sec;
  } else {
    return MAX_INTEGRAL_TIME;
  }
}

static inline time_t ToMilliseconds(struct timespec ts) {
  unassert(ts.tv_nsec < 1000000000);
  if (ts.tv_sec < MAX_INTEGRAL_TIME / 1000 - 999) {
    if (ts.tv_nsec <= 999000000) {
      ts.tv_nsec = (ts.tv_nsec + 999999) / 1000000;
    } else {
      ts.tv_sec += 1;
      ts.tv_nsec = 0;
    }
    return ts.tv_sec * 1000 + ts.tv_nsec;
  } else {
    return MAX_INTEGRAL_TIME;
  }
}

static inline time_t ToMicroseconds(struct timespec ts) {
  unassert(ts.tv_nsec < 1000000000);
  if (ts.tv_sec < MAX_INTEGRAL_TIME / 1000000 - 999999) {
    if (ts.tv_nsec <= 999999000) {
      ts.tv_nsec = (ts.tv_nsec + 999) / 1000;
    } else {
      ts.tv_sec += 1;
      ts.tv_nsec = 0;
    }
    return ts.tv_sec * 1000000 + ts.tv_nsec;
  } else {
    return MAX_INTEGRAL_TIME;
  }
}

static inline time_t ToNanoseconds(struct timespec ts) {
  unassert(ts.tv_nsec < 1000000000);
  if (ts.tv_sec < MAX_INTEGRAL_TIME / 1000000000 - 999999999) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
  } else {
    return MAX_INTEGRAL_TIME;
  }
}

#endif /* BLINK_TIMESPEC_H_ */
