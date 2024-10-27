#ifndef __DATAHANDLER_HPP
#define __DATAHANDLER_HPP

#include <stdio.h>
#include <conio.h>
#include <errhandlingapi.h>
#include <filesystem>
#include <string>

#define SESSIONS_AMOUNT 10
#define MAX_SESSION_VAR_LEN 50
#define EMULATOR_AMOUNT 2
#define BASEDIR "C:\\Users\\gardm\\.putty\\sessions\\"

class DataHandler {
public:
	DataHandler() {
		ses_len = MAX_SESSION_VAR_LEN;
		emu_len = EMULATOR_AMOUNT;
		ses_amo = SESSIONS_AMOUNT;

		lusr = new char[ses_len];
		lsrv = new char[ses_len];

		emulators = new std::string[emu_len];

		// Hardcode the different avalible emulators
		// Only ssh connection supported --> emulator usr@srv
		emulators[0] = "putty.exe ";
		emulators[1] = "plink.exe ";

		sessions = new std::string[ses_amo];
	}

	~DataHandler() {
		delete[] lusr;
		delete[] lsrv;
		delete[] emulators;
		delete[] sessions;
	}

	const char* getSrv();
	const char* getUsr();
	const char* getEmulator(int ilm);
	std::string* getSessionNames();

	void readTXT(std::string fname);
	void addTXT(std::string fname, std::string srv, std::string usr);
	void removeTXT(std::string fnmae);

private:

	std::string getFilePath(std::string fname);
	std::string getDirePath(std::string dname);

	int ses_len;
	char* lusr;
	char* lsrv;

	int emu_len;
	std::string* emulators;

	int ses_amo;
	std::string* sessions;
};

const char* DataHandler::getSrv() {
	return lsrv;
}

const char* DataHandler::getUsr() {
	return lusr;
}

const char* DataHandler::getEmulator(int ilm) {
	switch (ilm) {
	case 0:
		return emulators[0].c_str();
		break;
	case 1:
		return emulators[1].c_str();
		break;
	default:
		break;
	}
}

std::string* DataHandler::getSessionNames() {
	int count = 0;
	for (const auto& entry : std::filesystem::directory_iterator(BASEDIR)) {
		sessions[count] = entry.path().filename().string();
		count++;

		sessions[count].clear();
		if (count >= SESSIONS_AMOUNT) {
			printf("Sessions amount reached.\n");
			return sessions;
		}
	}

	return sessions;
}

void DataHandler::readTXT(std::string fname) {
	FILE* fptr;

	std::string path = getFilePath(fname);

	fptr = fopen(path.c_str(), "r");

	if (fptr == NULL) {
		printf("Was unable to open file %s (%d)\nreadCSV\n", path.c_str(), GetLastError());
		(void)_getch();
		return;
	}

	fgets(lusr, ses_len, fptr);
	fgets(lsrv, ses_len, fptr);

	fclose(fptr);

	// Replace the newline with a null terminator
	lusr[strcspn(lusr, "\n")] = '\0';
}

void DataHandler::addTXT(std::string fname, std::string srv, std::string usr) {
	FILE* fptr;
	// fname = dname --> .putty/ubuntu/ubuntu.txt
	std::string filePath = getFilePath(fname);
	std::string direPath = getDirePath(fname);

	// --> Filesystem requires C++17
	try {
		if (!std::filesystem::create_directories(direPath.c_str())) {
			printf("Directory already exists\n");
			(void)_getch();
			return;
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		printf("Was unable to create directory %s (%d)\naddTXT\n", direPath.c_str(), e.code().value());
		(void)_getch();
		return;
	}

	// Creating and writing to file
	fptr = fopen(filePath.c_str(), "w");

	if (fptr == NULL) {
		printf("Was unable to write to file %s (%d)\naddTXT\n", filePath.c_str(), GetLastError());
		(void)_getch();
		return;
	}

	fprintf(fptr, "%s\n%s", usr.c_str(), srv.c_str());

	fclose(fptr);
}

void DataHandler::removeTXT(std::string fname) {
	std::string direPath = getDirePath(fname);

	try {
		std::uintmax_t num_of_removed_items = std::filesystem::remove_all(direPath);
		printf("\tDeleted %llu item(s) at %s\n", num_of_removed_items, direPath.c_str());
	}
	catch (const std::filesystem::filesystem_error& e) {
		printf("Was unable to delete directory %s (%d)\nremoveTXT\n", direPath.c_str(), e.code().value());
		printf("\n%s\n", e.what());
	}

	(void)_getch();

	// Updates session names after removing a session
	(void)getSessionNames();
}

std::string DataHandler::getFilePath(std::string fname) {
	std::filesystem::path filepath = std::filesystem::path(BASEDIR) / fname / (fname + ".txt");
	return filepath.string();
}

std::string DataHandler::getDirePath(std::string dname) {
	std::filesystem::path direpath = std::filesystem::path(BASEDIR) / dname;
	return direpath.string();
}


#endif // !__DATAHANDLER_HPP


