#pragma once

#include <winbase.h>

namespace Utils {


struct PerformaceCounter
{
    typedef void (*undefined_tag)();
    static __forceinline void undefined() {}


    __forceinline PerformaceCounter( const undefined_tag )
    {
    }

    __forceinline PerformaceCounter()
    {
        reset();
    }

    __forceinline void reset()
    {
#if defined(_M_IX86)
        __asm
        {
            rdtsc
            mov ebx, dword ptr this[counter]
            mov dword ptr [ebx], eax
            mov dword ptr [ebx + 4], edx
        }
#else // _M_IX86
        counter = __rdtsc();
#endif // _M_IX86
    }

    // returns delta between counter and now
    __forceinline __int64 time() const
    {
#if defined(_M_IX86)
        __asm
        {
            rdtsc
            mov ebx, dword ptr this[counter]
            sub eax, dword ptr [ebx]
            sbb edx, dword ptr [ebx + 4]
        }
        // result will be in EAX:EDX
#else // _M_IX86
        return __rdtsc() - counter;
#endif // _M_IX86
    }

    // returns delta in seconds
    static __forceinline double delta_s( __int64 delta )
    {
        return (double)delta / get_frequency();
    }

    // returns delta between counter and now (in seconds)
    __forceinline double time_s() const
    {
        return delta_s(time());
    }

    // returns delta in milliseconds
    static __forceinline double delta_ms( __int64 delta )
    {
        return (double)delta / get_frequency();
    }

    // returns delta between counter and now (in milliseconds)
    __forceinline double time_ms() const
    {
        return delta_ms(time() * 1000);
    }

    static __forceinline __int64 get_counter()
    {
#if defined(_M_IX86)
        __asm rdtsc
        // result will be in EAX:EDX
#else // _M_IX86
        return __rdtsc();
#endif // _M_IX86
    }

    // calculates once and uses frequency
    static __int64 get_frequency()
    {
        static __int64 frequency = -1;

        // need to evaluate
        if (frequency == -1)
        {
            // get start timings
            __int64 sysStart;
            ::QueryPerformanceCounter((LARGE_INTEGER *)&sysStart);
            const __int64 cpuStart = get_counter();

            // sleep awhile
            ::Sleep(50);

            // get finish timings
            __int64 sysFinish;
            ::QueryPerformanceCounter((LARGE_INTEGER *)&sysFinish);
            const __int64 cpuFinish = get_counter();

            ::QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

            // get ratio
            const double ratio = double(cpuFinish - cpuStart) / (sysFinish - sysStart);

            if (ratio < 0.99 || ratio > 1.01)
                frequency = __int64(frequency * ratio);
        }

        return frequency;
    }


private:

    __int64 counter;
};


} // namespace utils
