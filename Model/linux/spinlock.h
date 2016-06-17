#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

#include <smack.h>

#ifndef SPIN_LOCK_UNINITIALIZED
#define SPIN_LOCK_UNINITIALIZED 0
#endif

#ifndef SPIN_LOCK_INITIALIZED
#define SPIN_LOCK_INITIALIZED 1
#endif

#ifndef SPIN_LOCK_UNLOCKED
#define SPIN_LOCK_UNLOCKED 0
#endif

typedef struct
{
  int init;
  int lock;
} spinlock_t;

#define DEFINE_SPINLOCK(x) spinlock_t x = { SPIN_LOCK_INITIALIZED, SPIN_LOCK_UNLOCKED }

void spin_lock_init(spinlock_t *lock)
{
  lock->init = SPIN_LOCK_INITIALIZED;
  lock->lock = SPIN_LOCK_UNLOCKED;
}

void spin_lock(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ != @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  __SMACK_code("assume @ == @;", lock->lock, SPIN_LOCK_UNLOCKED);
  lock->lock = tid;
  __SMACK_code("call corral_atomic_end();");
}

void spin_lock_irqsave(spinlock_t *lock, unsigned long value)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ != @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  __SMACK_code("assume @ == @;", lock->lock, SPIN_LOCK_UNLOCKED);
  lock->lock = tid;
  __SMACK_code("call corral_atomic_end();");
}

void spin_lock_irq(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ != @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  __SMACK_code("assume @ == @;", lock->lock, SPIN_LOCK_UNLOCKED);
  lock->lock = tid;
  __SMACK_code("call corral_atomic_end();");
}

void spin_lock_bh(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ != @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  __SMACK_code("assume @ == @;", lock->lock, SPIN_LOCK_UNLOCKED);
  lock->lock = tid;
  __SMACK_code("call corral_atomic_end();");
}

void spin_unlock(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ == @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  lock->lock = SPIN_LOCK_UNLOCKED;
  __SMACK_code("call corral_atomic_end();");
}

void spin_unlock_irqrestore(spinlock_t *lock, unsigned long value)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ == @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  lock->lock = SPIN_LOCK_UNLOCKED;
  __SMACK_code("call corral_atomic_end();");
}

void spin_unlock_irq(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ == @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  lock->lock = SPIN_LOCK_UNLOCKED;
  __SMACK_code("call corral_atomic_end();");
}

void spin_unlock_bh(spinlock_t *lock)
{
  int tid = __SMACK_nondet();
  __SMACK_code("call @ := corral_getThreadID();", tid);
  //__SMACK_code("assert @ == @;", tid, lock->lock);
  __SMACK_code("call corral_atomic_begin();");
  lock->lock = SPIN_LOCK_UNLOCKED;
  __SMACK_code("call corral_atomic_end();");
}

#endif /* __LINUX_SPINLOCK_H */
