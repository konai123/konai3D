//
// Created by khhan on 2021-06-18.
//

#ifndef KONAI3D_RWLOCK_HPP
#define KONAI3D_RWLOCK_HPP

_START_ENGINE
class RWLock : public SRWLOCK {
public:
    RWLock() {
        ::InitializeSRWLock(this);
    };

    virtual ~RWLock() = default;

public:
    void AcquireShared() {
        ::AcquireSRWLockShared(this);
    }

    void ReleaseShared() {
        ::ReleaseSRWLockShared(this);
    }

    void AcquireExclusive() {
        ::AcquireSRWLockExclusive(this);
    }

    void ReleaseExclusive() {
        ::ReleaseSRWLockExclusive(this);
    }
};

struct LocalWriteLock {
public:
    LocalWriteLock(RWLock &lock) : _lock(lock) {
        _lock.AcquireExclusive();
    }

    virtual ~LocalWriteLock() {
        _lock.ReleaseExclusive();
    }

public:
    RWLock &_lock;
};

struct LocalReadLock {
public:
    LocalReadLock(RWLock &lock) : _lock(lock) {
        _lock.AcquireShared();
    }

    virtual ~LocalReadLock() {
        _lock.ReleaseShared();
    }

public:
    RWLock &_lock;
};
_END_ENGINE

#endif //KONAI3D_RWLOCK_HPP
