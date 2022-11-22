/* Maria Bui, Ryan Valler, Steph Teixeira, Troy Josti */

main() {
	char line[80], buffer[13312], dir[512], textfile[13312];
	int sectorsRead, i, fileentry, offset;

	while (1) {
		syscall(0, "\r\nSHELL> ");
		syscall(1, line);

		for (i = 0; i < 80; i++) {
			if (line[i] == '\n' || line[i] == '\r' || line[i] == ' ')
				line[i] = '\0';
		}

		if (line[0] == 't' &&
			line[1] == 'y' &&
			line[2] == 'p' &&
			line[3] == 'e') {
			syscall(3, line+5, buffer, &sectorsRead);
			if (sectorsRead > 0)
				syscall(0, buffer);
			else
				syscall(0, "file not found\r\n");
		}
		else if (line[0] == 'e' &&
				line[1] == 'x' &&
				line[2] == 'e' &&
				line[3] == 'c') {
				syscall(3, line+5, buffer, &sectorsRead);
				if (sectorsRead > 0)
					syscall(4, line+5);
				else
					syscall(0, "program not found\r\n");
		}
		else if (line[0] == 'd' &&
				line[1] == 'i' &&
				line[2] == 'r') {
			syscall(2, dir, 2);
			for (fileentry = 0; fileentry < 512; fileentry += 32) {
				if (dir[fileentry] != '\0') {
					dir[fileentry+6] = 0;
					syscall(0, dir+fileentry);
					syscall(0, "\r\n");
				}
			}
		}
		else if (line[0] == 'd' &&
				line[1] == 'e' &&
				line[2] == 'l') {
			syscall(3, line+4, buffer, &sectorsRead);
			if (sectorsRead > 0)
				syscall(7, line+4);
			else
				syscall(0, "file not found\r\n");
		}
		else if (line[0] == 'c' &&
				line[1] == 'o' &&
				line[2] == 'p' &&
				line[3] == 'y') {
			syscall(3, line+5, buffer, &sectorsRead);
			if (sectorsRead > 0) {
				for (i = 5; i < 80; i++) {
					if (line[i] == '\0') {
						i++;
						break;
					}
				}
				syscall(8, buffer, line+i, sectorsRead);
			}
			else
				syscall(0, "file not found\r\n");
		}
		else if (line[0] == 'c' &&
				line[1] == 'r' &&
				line[2] == 'e' &&
				line[3] == 'a' &&
				line[4] == 't' &&
				line[5] == 'e') {
			offset = 0;
			while (offset < 13312) {
				syscall(1, textfile + offset);
				if (textfile[offset] == 0xd) {
					textfile[offset] = 0x0;
					break;
				}
				for (; offset < 13312; offset++) {
					if (textfile[offset] == 0x0) break;
				}
			}
			syscall(8, textfile, line+7, 1);
		}
		else
			syscall(0, "Bad Command!\r\n");
	}
}
