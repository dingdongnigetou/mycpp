#ifndef __MYCPP_MSWINAPI_HPP__
#define __MYCPP_MSWINAPI_HPP__

#include "Singleton.hpp"

#include <process.h>

namespace mycpp
{
	// windowsϵͳapi
	class MSWinApi : public Singleton<MSWinApi>
	{
		friend class Singleton<MSWinApi>;
	protected:
		MSWinApi()
		{
			HMODULE kernel32_module = nullptr;

			kernel32_module = GetModuleHandleA("kernel32.dll");
			if (kernel32_module != nullptr)
			{
				AcquireSRWLockShared = (sAcquireSRWLockShared)GetProcAddress(kernel32_module, "AcquireSRWLockShared");
				AcquireSRWLockExclusive = (sAcquireSRWLockExclusive)GetProcAddress(kernel32_module, "AcquireSRWLockExclusive");
				TryAcquireSRWLockShared = (sTryAcquireSRWLockShared)GetProcAddress(kernel32_module, "TryAcquireSRWLockShared");
				TryAcquireSRWLockExclusive = (sTryAcquireSRWLockExclusive)GetProcAddress(kernel32_module, "TryAcquireSRWLockExclusive");
				ReleaseSRWLockShared = (sReleaseSRWLockShared)GetProcAddress(kernel32_module, "ReleaseSRWLockShared");
				ReleaseSRWLockExclusive = (sReleaseSRWLockExclusive)GetProcAddress(kernel32_module, "ReleaseSRWLockExclusive");
				InitializeSRWLock = (sInitializeSRWLock)GetProcAddress(kernel32_module, "InitializeSRWLock");
				SleepConditionVariableCS = (sSleepConditionVariableCS)GetProcAddress(kernel32_module, "SleepConditionVariableCS");
				SleepConditionVariableSRW = (sSleepConditionVariableSRW)GetProcAddress(kernel32_module, "SleepConditionVariableSRW");
				WakeAllConditionVariable = (sWakeAllConditionVariable)GetProcAddress(kernel32_module, "WakeAllConditionVariable");
				WakeConditionVariable = (sWakeConditionVariable)GetProcAddress(kernel32_module, "WakeConditionVariable");
				InitializeConditionVariable = (sInitializeConditionVariable)GetProcAddress(kernel32_module, "InitializeConditionVariable");
			}
		}

		~MSWinApi() {}

	public:
		bool IsMSWndHaveSRWLockApi()
		{
			return (InitializeSRWLock != nullptr);
		}

		bool IsMSWndHaveCondVarApi()
		{
			return (InitializeConditionVariable != nullptr && InitializeSRWLock != nullptr);
		}

	private:
		using sInitializeSRWLock = VOID(WINAPI*)(PSRWLOCK SRWLock);
		using sAcquireSRWLockShared = VOID(WINAPI*)(PSRWLOCK SRWLock);
		using sAcquireSRWLockExclusive = VOID(WINAPI*)(PSRWLOCK SRWLock);
		using sTryAcquireSRWLockShared = BOOL(WINAPI*)(PSRWLOCK SRWLock);
		using sTryAcquireSRWLockExclusive = BOOL(WINAPI*)(PSRWLOCK SRWLock);
		using sReleaseSRWLockShared = VOID(WINAPI*)(PSRWLOCK SRWLock);
		using sReleaseSRWLockExclusive = VOID(WINAPI*)(PSRWLOCK SRWLock);
		using sInitializeConditionVariable = VOID(WINAPI*)(PCONDITION_VARIABLE ConditionVariable);
		using sSleepConditionVariableCS = BOOL(WINAPI*)(PCONDITION_VARIABLE ConditionVariable,
														PCRITICAL_SECTION CriticalSection,
														DWORD dwMilliseconds);
		using sSleepConditionVariableSRW = BOOL(WINAPI*)(PCONDITION_VARIABLE ConditionVariable,
														 PSRWLOCK SRWLock,
														 DWORD dwMilliseconds,
														 ULONG Flags);
		using sWakeAllConditionVariable =	VOID(WINAPI*)(PCONDITION_VARIABLE ConditionVariable);
		using sWakeConditionVariable = VOID(WINAPI*)(PCONDITION_VARIABLE ConditionVariable);

	public:
		sInitializeSRWLock InitializeSRWLock;
		sAcquireSRWLockShared AcquireSRWLockShared;
		sAcquireSRWLockExclusive AcquireSRWLockExclusive;
		sTryAcquireSRWLockShared TryAcquireSRWLockShared;
		sTryAcquireSRWLockExclusive TryAcquireSRWLockExclusive;
		sReleaseSRWLockShared ReleaseSRWLockShared;
		sReleaseSRWLockExclusive ReleaseSRWLockExclusive;
		sInitializeConditionVariable InitializeConditionVariable;
		sSleepConditionVariableCS SleepConditionVariableCS;
		sSleepConditionVariableSRW SleepConditionVariableSRW;
		sWakeAllConditionVariable WakeAllConditionVariable;
		sWakeConditionVariable WakeConditionVariable;

	};
}

#endif // !__MYCPP_MSWINAPI_HPP__
