#include "MinSpeedhack.h"

namespace MS {
    static double speed = 1.0;

    static DWORD base32 = 0, last32 = 0;
    static ULONGLONG base64 = 0, last64 = 0;
    static LARGE_INTEGER baseQpc{}, lastQpc{};

    static double elapsed32 = 0;
    static double elapsed64 = 0;
    static double elapsedQpc = 0;

    static DWORD(WINAPI* origGetTickCount)();
    static ULONGLONG(WINAPI* origGetTickCount64)();
    static BOOL(WINAPI* origQPC)(LARGE_INTEGER*);

    static DWORD WINAPI hkGetTickCount() {
        DWORD now = origGetTickCount();

        if (!base32) {
            base32 = now;
            last32 = now;
            return now;
        }

        DWORD delta = now - last32;
        last32 = now;

        elapsed32 += delta * speed;
        return base32 + (DWORD)elapsed32;
    }

    static ULONGLONG WINAPI hkGetTickCount64() {
        ULONGLONG now = origGetTickCount64();

        if (!base64) {
            base64 = now;
            last64 = now;
            return now;
        }

        ULONGLONG delta = now - last64;
        last64 = now;

        elapsed64 += delta * speed;
        return base64 + (ULONGLONG)elapsed64;
    }

    static BOOL WINAPI hkQueryPerformanceCounter(LARGE_INTEGER* lp) {
        LARGE_INTEGER now;
        BOOL r = origQPC(&now);

        if (!baseQpc.QuadPart) {
            baseQpc = now;
            lastQpc = now;
            *lp = now;
            return r;
        }

        LONGLONG delta = now.QuadPart - lastQpc.QuadPart;
        lastQpc = now;

        elapsedQpc += delta * speed;
        lp->QuadPart = baseQpc.QuadPart + (LONGLONG)elapsedQpc;

        return r;
    }

    static Hook hooks[3] = {
        { (void*)&GetTickCount, (void*)&hkGetTickCount, (void**)&origGetTickCount },
        { (void*)&GetTickCount64, (void*)&hkGetTickCount64, (void**)&origGetTickCount64 },
        { (void*)&QueryPerformanceCounter, (void*)&hkQueryPerformanceCounter, (void**)&origQPC },
    };

    void SetSpeed(double value) {
        speed = value;
    }

    double GetSpeed() {
        return speed;
    }

    const Hook* GetHooks(size_t& count) {
        count = 3;
        return hooks;
    }

}