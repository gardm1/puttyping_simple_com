#ifndef __PROCESS_HPP
#define __PROCESS_HPP

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include <iostream>
#include <string>
#include <Windows.h>
#include <atlstr.h>

#ifdef _WIN32
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#define UTF_8_ENCODED_STRINGS

#ifdef _UNICODE
typedef WCHAR TCHAR;
#else
typedef char TCHAR;
#endif

namespace prcs {

	struct Process {
	public:
		Process(ULONG n) {
			ses_len = n;
			hProcess = NULL;
			hThread = NULL;
			dwProcessId = 0;
			dwThreadId = 0;

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			args = new WCHAR[ses_len];
		}

		void free() {

			delete[] args;

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}


		// Convert to wide string
		const wchar_t* string_to_pconstwchar_t(std::string s) {
#ifdef _UNICODE
			wchar_t* wideString = new wchar_t[ses_len];

			int result = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wideString, ses_len);

			// Check if the conversion succeeded
			if (result == 0) {
				printf("Was unable to convert string to wide string (%d)\nstring_to_pconstwchar_t", GetLastError());
				delete[] wideString;
				return nullptr;
			}

			return wideString;


#elif _ANSI

#endif

			printf("Neither UTF-8 or ANSI was selected (check debug inf.)\n");
			return nullptr;

		}


		const wchar_t* pchar_to_pconstwchar_t(const char* c) {
#ifdef _UNICODE
			wchar_t* wideString = new wchar_t[ses_len];

			int result = MultiByteToWideChar(CP_UTF8, 0, c, -1, wideString, ses_len);

			// Check if the conversion succeeded
			if (result == 0) {
				printf("Was unable	to convert string to wide string (%d)\npchar_to_pconstwchar_t", GetLastError());
				delete[] wideString;
				return nullptr;
			}

			return wideString;


#elif _ANSI

#endif

			printf("Neither UTF-8 or ANSI was selected (check debug inf.)\n");
			return nullptr;

		}



		LPTSTR args;
		ULONG ses_len;
		HANDLE hProcess;
		HANDLE hThread;
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		DWORD dwProcessId;
		DWORD dwThreadId;
	};

	void EXECUTECOMMAND(const char* sargv) {
		//ULONG len = (ULONG)sArgs.length() + 1;
		ULONG len = (ULONG)strlen(sargv) + 1;
		Process process(len);

		//const wchar_t* pArgs = string_to_pconstwchar_t(sArgs, process.nChars);
		const wchar_t* pArgs = process.pchar_to_pconstwchar_t(sargv);

		wcscpy_s(process.args, process.ses_len, pArgs);

		if (!CreateProcess(
			NULL,
			process.args,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&process.si,
			&process.pi)
			)
		{
			printf("CreateProcess Failed (%d).\nEXECUTECOMMAND\n", GetLastError());
			process.free();
			delete[] pArgs;
			return;
		}

#ifdef DEBUG
		printf("GetProcessID -> %d\n", GetProcessId(process.pi.hProcess));
		printf("GetThreadID -> %d\n", GetThreadId(process.pi.hThread));
#endif

		WaitForSingleObject(process.pi.hProcess, INFINITE);

		process.free();
		delete[] pArgs;
	}

	// Source:
	// https://learn.microsoft.com/nb-no/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho2?redirectedfrom=MSDN
	int __cdecl PING_INET_ADDR(const char* argv) {
		// Declare and initialize variables.
		HANDLE hIcmpFile;
		unsigned long ipaddr = INADDR_NONE;
		DWORD dwRetVal = 0;
		DWORD dwError = 0;
		char SendData[] = "Data Buffer";
		LPVOID ReplyBuffer = NULL;
		DWORD ReplySize = 0;

		ipaddr = inet_addr(argv);
		if (ipaddr == INADDR_NONE) {
			printf("Invalid Ip address\n");
			return 1;
		}

		hIcmpFile = IcmpCreateFile();
		if (hIcmpFile == INVALID_HANDLE_VALUE) {
			printf("\tUnable to open handle.\n");
			printf("IcmpCreatefile returned error: %ld\n", GetLastError());
			return 1;
		}

		// Allocate space for a single reply.
		ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + 8;
		ReplyBuffer = (VOID*)malloc(ReplySize);
		if (ReplyBuffer == NULL) {
			printf("\tUnable to allocate memory for reply buffer\n");
			return 1;
		}

		dwRetVal = IcmpSendEcho2(hIcmpFile, NULL, NULL, NULL,
			ipaddr, SendData, sizeof(SendData), NULL,
			ReplyBuffer, ReplySize, 5000);
		if (dwRetVal != 0) {
			PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
			struct in_addr ReplyAddr;
			ReplyAddr.S_un.S_addr = pEchoReply->Address;
			printf("\tSent icmp message to %s\n", argv);
			if (dwRetVal > 1) {
				printf("\tReceived %ld icmp message responses\n", dwRetVal);
				printf("\tInformation from the first response:\n");
			}
			else {
				printf("\tReceived %ld icmp message response\n", dwRetVal);
				printf("\tInformation from this response:\n");
			}
			printf("\t  Received from %s\n", inet_ntoa(ReplyAddr));
			printf("\t  Status = %ld  ", pEchoReply->Status);
			switch (pEchoReply->Status) {
			case IP_DEST_HOST_UNREACHABLE:
				printf("(Destination host was unreachable)\n");
				break;
			case IP_DEST_NET_UNREACHABLE:
				printf("(Destination Network was unreachable)\n");
				break;
			case IP_REQ_TIMED_OUT:
				printf("(Request timed out)\n");
				break;
			default:
				printf("\n");
				break;
			}

			printf("\t  Roundtrip time = %ld milliseconds\n",
				pEchoReply->RoundTripTime);
		}
		else {
			printf("Call to IcmpSendEcho2 failed.\n");
			dwError = GetLastError();
			switch (dwError) {
			case IP_BUF_TOO_SMALL:
				printf("\tReplyBufferSize too small\n");
				break;
			case IP_REQ_TIMED_OUT:
				printf("\tRequest timed out\n");
				break;
			default:
				printf("\tExtended error returned: %ld\n", dwError);
				break;
			}
			free(ReplyBuffer);
			return 1;
		}
		free(ReplyBuffer);
		return 0;
	}

}

#endif // !__PROCESS_HPP


