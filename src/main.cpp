#define _WINSOCK_DEPRECATED_NO_WARNINGS

/* _WINSOCK_DEPRECATED_NO_WARNINGS
* is needed for inet_addr();
* could be changed out with
* InetPton(); Seach up doc. 
* on Microsoft.learn.com 
*/

#define _CRT_SECURE_NO_WARNINGS

/* _CRT_SECURE_NO_WARNINGS
* is needed for FILE;
*/

#include "process.hpp"
#include "dataHandler.hpp"

#include <iostream>
#include <Windows.h>
#include <string>


void clearScreen() {
	//system("cls");
	printf("\033[2J\033[H");
}

void printCommands() {
	printf("\nCommands.\n");
	printf("  Add New Session (a)\t\tSettings (x)\n");
	printf("  Select A Session (s)\n");
	printf("  Quiting Program (q)\n");
}

void add(DataHandler &dh) {
	std::string session_name;
	std::string server;
	std::string host_name;

	printf("\n\tInput session name: ");
	std::cin >> session_name;
	printf("\n\tInput server IP: ");
	std::cin >> server;
	printf("\n\tInput host name: ");
	std::cin >> host_name;

	dh.addTXT(session_name, server, host_name);
}

void mod(DataHandler &dh) {
	int iInput;
	char cInput;

	printf("\nAvailable sessions.\n");
	std::string* sessions = dh.getSessionNames();
	int i = 0;
	for (i; i < sessions->size(); i++) {
		if (sessions[i].empty()) break;

		printf("\t(%d) %s\n", i + 1, sessions[i].c_str());
	}

	std::cin >> iInput;

	while (iInput > i || iInput <= 0) {
		printf("Out of bounds! Try again.\n");
		std::cin >> iInput;
	}

	iInput -= 1; // (1,2) --> (0,1)

	printf("Wish to connect to? Or delete this session? (c)onnect/(d)elete ");
	std::cin >> cInput;
	if (cInput == 'd') {
		printf("\nAre you sure you want to delete %s? (Y/n) ", sessions[iInput].c_str());
		std::cin >> cInput;
		if (cInput == 'Y') {
			dh.removeTXT(sessions[iInput]);
		}
	}
	else if (cInput == 'c') {
		dh.readTXT(sessions[iInput]);

		const char* srv = dh.getSrv();
		const char* usr = dh.getUsr();

		const char* emulator = dh.getEmulator(0);

		printf("\nEmulator   --> %s\nIP Address --> '%s'\nHost Name  --> %s\n", emulator, srv, usr);

		const char* alf = "@";

		size_t len = strlen(emulator) + strlen(usr) + strlen(alf) + strlen(srv) + 1;
		char* sargv = new char[len];

		strcpy_s(sargv, len, emulator);
		strcat_s(sargv, len, usr);
		strcat_s(sargv, len, alf);
		strcat_s(sargv, len, srv);

		printf("\nPinging server IP...\n");
		prcs::PING_INET_ADDR(srv);

		printf("\nConnect to your session? (y/n) ");
		std::cin >> cInput;
		if (cInput == 'y' || cInput == 'Y') {
			printf("\nConnecting to you session...\n");
			prcs::EXECUTECOMMAND(sargv);
			printf("Session ended.\n");
		}
		else {
			printf("Not connecting to session.\n");
		}

		delete[] sargv;
	}
}

int main() {
	DataHandler dh;

	bool b = true;
	char cInput;

	while (b) {
		clearScreen();
		printCommands();

		std::cin >> cInput;

		clearScreen();

		switch (cInput) {
		case 97: // Unicode of a
			add(dh);
			break;
		case 115: // Unicode of s
			mod(dh);
			break;
		case 113: // Unicode of q
			b = false;
			break;
		default:
			break;
		}
	}

	printf("\nPress any key to continue . . .");
	(void)_getch();
	return 0;
}
