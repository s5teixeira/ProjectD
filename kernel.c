/* Maria Bui, Ryan Valler, Steph Teixeira, Troy Josti */

void printString(char*);
void printChar(char);
void readString(char*);
void readSector(char*, int);
void readFile(char*, char*, int*);
void executeProgram(char*);
void terminate();
void writeSector(char*, int);
void deleteFile(char*);
void writeFile(char*, char*, int);
void handleInterrupt21(int, int, int, int);

void main() {
	makeInterrupt21();
//	interrupt(0x21, 8, "this is a test message", "testmg", 3);
	interrupt(0x21, 4, "shell", 0, 0);
	while(1);
}

void printString(char* chars) {
	while (*chars != 0x0) {
		interrupt(0x10, 0xe*256+*chars, 0, 0, 0);
		chars++;
	}
}

void printChar(char c) {
	interrupt(0x10, 0xe*256+c, 0, 0, 0);
}

void readString(char* chars) {
	int i = 0;
	char key;

	while (i < 80) {
		key = interrupt(0x16, 0, 0, 0, 0);

		if (key == 0x8) {
			if (i != 0) {
				i--;
				interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
				interrupt(0x10, 0xe*256+' ', 0, 0, 0);
				interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
			}
		}
		else {
			chars[i] = key;

			if (key == 0xd) {
				interrupt(0x10, 0xe*256+0xd, 0, 0, 0);
				interrupt(0x10, 0xe*256+0xa, 0, 0, 0);
				chars[i+1] = 0xa;
				chars[i+2] = 0x0;
				return;
			}

			interrupt(0x10, 0xe*256+key, 0, 0, 0);
			i++;
		}
	}
}

void readSector(char* buffer, int sector) {
	interrupt(0x13, 0x2*256+1, buffer, 0*256+sector+1, 0*256+0x80);
}

void readFile(char* filename, char* buffer, int* sectorsRead) {
	char dir[512];
	int fileentry, i;

	readSector(dir, 2);
	*sectorsRead = 0;

	for (fileentry = 0; fileentry < 512; fileentry += 32) {
		for (i = 0; i < 6; i++) {
			if (filename[i] == dir[fileentry+i]) {
				if ((filename[i] == 0 && dir[fileentry+i] == 0) || i == 5) {
					for (i = 6; i < 32; i++) {
						if (dir[fileentry+i] != 0) {
							readSector(buffer, dir[fileentry+i]);
							buffer += 512;
							(*sectorsRead)++;
						}
						else return;
					}
				}
			}
			else break;
		}
	}
}

void executeProgram(char* name) {
	char buffer[13312];
	int sectorsRead, address;

	readFile(name, buffer, &sectorsRead);

	for (address = 0; address < 13312; address++)
		putInMemory(0x2000, address, buffer[address]);

	if (sectorsRead > 0)
		launchProgram(0x2000);
}

void terminate() {
	char shellname[6];

	shellname[0]='s';
	shellname[1]='h';
	shellname[2]='e';
	shellname[3]='l';
	shellname[4]='l';
	shellname[5]='\0';

	executeProgram(shellname);

	while(1);
}

void writeSector(char* buffer, int sector) {
	interrupt(0x13, 0x3*256+1, buffer, 0*256+sector+1, 0*256+0x80);
}

void deleteFile(char* filename) {
	char dir[512], map[512];
	int fileentry, i;

	readSector(dir, 2);
	readSector(map, 1);

	for (fileentry = 0; fileentry < 512; fileentry += 32) {
		for (i = 0; i < 6; i++) {
			if (filename[i] == dir[fileentry+i]) {
				if ((filename[i] == 0 && dir[fileentry+i] == 0) || i == 5) {
					dir[fileentry] = '\0';
					for (i = 6; i < 32; i++) {
						if (dir[fileentry+i] != 0)
							map[dir[fileentry+i]] = 0;
						else break;
					}
					writeSector(dir, 2);
					writeSector(map, 1);
					return;
				}
			}
			else break;
		}
	}
}

void writeFile(char* buffer, char* filename, int numberOfSectors) {
	char map[512], dir[512];
	int entry, i, sector, freeentry = 0, freesector = 0;

	readSector(map, 1);
	readSector(dir, 2);

	for (entry = 0; entry < 512; entry += 32) {
		if (dir[entry] == '\0') {
			freeentry = 1;
			for (i = 0; i < 6; i++) {
				if (filename[i] != 0)
					dir[entry+i] = filename[i];
				else
					dir[entry+i] = '\0';
			}
			break;
		}
	}
	if (freeentry == 0) return;

	for (i = 0; i < numberOfSectors; i++) {
		freesector = 0;
		for (sector = 3; sector < 512; sector++) {
			if (map[sector] == 0) {
				freesector = 1;
				map[sector] = 0xFF;
				dir[entry+6+i] = sector;
				writeSector(buffer, sector);
				buffer += 512;
				break;
			}
		}
		if (freesector == 0) {
			for (; i < 26; i++)
				dir[entry+6+i] = 0;
			return;
		}
	}

	for (; i < 26; i++)
		dir[entry+6+i] = 0;

	writeSector(map, 1);
	writeSector(dir, 2);
}

void handleInterrupt21(int ax, int bx, int cx, int dx) {
	if (ax == 0)
		printString(bx);
	else if (ax == 1)
		readString(bx);
	else if (ax == 2)
		readSector(bx, cx);
	else if (ax == 3)
		readFile(bx, cx, dx);
	else if (ax == 4)
		executeProgram(bx);
	else if (ax == 5)
		terminate();
	else if (ax == 6)
		writeSector(bx, cx);
	else if (ax == 7)
		deleteFile(bx);
	else if (ax == 8)
		writeFile(bx, cx, dx);
	else
		printString("function not found");
}
