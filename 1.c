#include<windows.h>
#include<stdio.h>

#include <TlHelp32.h>  

#include "winternl.h"  
typedef NTSTATUS(WINAPI *NtQueryInformationProcessFake)(HANDLE, DWORD, PVOID, ULONG, PULONG);

NtQueryInformationProcessFake ntQ = NULL;

typedef struct tagPROCESS_BASIC_INFORMATION
{
	DWORD ExitStatus;
	DWORD PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}_PROCESS_BASIC_INFORMATION;



int GetParentProcessID111(DWORD dwId)
{
	LONG                      status;
	DWORD                     dwParentPID = 0;
	HANDLE                    hProcess;
	_PROCESS_BASIC_INFORMATION pbi;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwId);
	if (!hProcess)
		return -1;

	status = ntQ(hProcess, SystemBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (!status)
		dwParentPID = pbi.InheritedFromUniqueProcessId;

	CloseHandle(hProcess);
	return dwParentPID;
}

void getProcCMD(DWORD pid,FILE* fp) {

	HANDLE                    hProcess;
	LONG                      status;
	_PROCESS_BASIC_INFORMATION lpbi;
	DWORD                     dwParentPID = 0;
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!hProcess)
		return -1;
	HMODULE hm = LoadLibraryA("ntdll.dll");
	ntQ = (NtQueryInformationProcessFake)GetProcAddress(hm, "NtQueryInformationProcess");
	printf("ntdll handle %d \n", hm);
	status = ntQ(hProcess, SystemBasicInformation, (PVOID)&lpbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (!status)
		dwParentPID = lpbi.InheritedFromUniqueProcessId;

	HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwParentPID);
	if (INVALID_HANDLE_VALUE != hproc) {
		HANDLE hnewdup = NULL;
		PEB peb;
		RTL_USER_PROCESS_PARAMETERS upps;
		WCHAR buffer[MAX_PATH] = { NULL };
		if (DuplicateHandle(GetCurrentProcess(), hproc, GetCurrentProcess(), &hnewdup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
			PROCESS_BASIC_INFORMATION pbi;
			NTSTATUS isok = ntQ(hnewdup, 0/*ProcessBasicInformation*/, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), 0);
			if (BCRYPT_SUCCESS(isok)) {
				if (ReadProcessMemory(hnewdup, pbi.PebBaseAddress, &peb, sizeof(PEB), 0))
					if (ReadProcessMemory(hnewdup, peb.ProcessParameters, &upps, sizeof(RTL_USER_PROCESS_PARAMETERS), 0)) {
						WCHAR *buffer = malloc(upps.CommandLine.Length*2 + 2);
						ZeroMemory(buffer, (upps.CommandLine.Length + 1) * sizeof(WCHAR));
						ReadProcessMemory(hnewdup, upps.CommandLine.Buffer, buffer, upps.CommandLine.Length, 0);
						wprintf(L"command  is %ls\n", buffer);
						fwrite(buffer, 2, upps.CommandLine.Length, fp);
						fflush(fp);
						free(buffer);
					}
			}
			CloseHandle(hnewdup);
		}

		CloseHandle(hproc);
	}
}

BOOL traverseProcesses(FILE* fp)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	UINT32 ppid;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);



	if (hProcessSnap == INVALID_HANDLE_VALUE) 
	{
		printf("CreateToolhelp32Snapshot error!\n");
		return FALSE;
	}
	BOOL bResult = Process32First(hProcessSnap, &pe32);
	while (bResult)
	{
	   wchar_t* name = pe32.szExeFile;
	   int id = pe32.th32ProcessID;
	   if (!lstrcmpiW(name, L"dw20.exe"))
	   {
		   wprintf(L"find !\n");
		   getProcCMD(id, fp);
		  HANDLE  hd= OpenProcess(PROCESS_ALL_ACCESS, TRUE, id);
		  if (!TerminateProcess(hd, 0))
		  {
			  Sleep(3);
		  }
		   break;
	   }
		bResult = Process32Next(hProcessSnap, &pe32);
	}

	return 1;
}
void main()
{
	FILE* fp=NULL;
	fp=fopen("log.txt", "ab");

	HANDLE              hToken;
	LUID                SeDebugNameValue;
	TOKEN_PRIVILEGES    TokenPrivileges;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &SeDebugNameValue))
		{
			TokenPrivileges.PrivilegeCount = 1;
			TokenPrivileges.Privileges[0].Luid = SeDebugNameValue;
			TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
			{
				CloseHandle(hToken);
			}
			else
			{
				CloseHandle(hToken);
			}
		}
	}
	while (1)
	{
		traverseProcesses(fp);
		Sleep(3);
	}
	fclose(fp);
}