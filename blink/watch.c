/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include <stdlib.h>
#include <string.h>

#include "blink/address.h"
#include "blink/debug.h"
#include "blink/endian.h"
#include "blink/machine.h"
#include "blink/modrm.h"
#include "blink/watch.h"

void PopWatchpoint(struct Watchpoints *wps) {
  if (wps->i) {
    --wps->i;
  }
}

ssize_t PushWatchpoint(struct Watchpoints *wps, struct Watchpoint *b) {
  int i;
  for (i = 0; i < wps->i; ++i) {
    if (wps->p[i].disable) {
      memcpy(&wps->p[i], b, sizeof(*b));
      return i;
    }
  }
  if (wps->i++ == wps->n) {
    wps->n = wps->i + (wps->i >> 1);
    wps->p = (struct Watchpoint *)realloc(wps->p, wps->n * sizeof(*wps->p));
  }
  wps->p[wps->i - 1] = *b;
  return wps->i - 1;
}

ssize_t IsAtWatchpoint(struct Watchpoints *wps, struct Machine *m) {
  u8 *r;
  i64 oldip;
  bool hasop;
  struct XedDecodedInst x;
  if (!wps->i) return -1;
  hasop = !GetInstruction(m, GetPc(m), &x) && x.op.has_modrm;
  for (int i = wps->i; i--;) {
    if (wps->p[i].disable) continue;
    oldip = m->ip;
    m->ip += Oplength(x.op.rde);
    if (hasop && !IsModrmRegister(x.op.rde) &&
        ComputeAddress(m, x.op.rde, x.op.disp, x.op.uimm0) == wps->p[i].addr) {
      return i;
    }
    m->ip = oldip;
    // TODO(jart): Handle case of overlapping page boundary.
    // TODO(jart): Possibly track munmap() type cases.
    if ((r = LookupAddress(m, wps->p[i].addr))) {
      u64 w = Read64(r);
      if (!wps->p[i].initialized) {
        wps->p[i].oldvalue = w;
        wps->p[i].initialized = true;
      } else if (w != wps->p[i].oldvalue) {
        wps->p[i].oldvalue = w;
        return i;
      }
    }
  }
  return -1;
}
