// Intel HEX read/write functions, Paul Stoffregen, paul@ece.orst.edu
// This code is in the public domain.  Please retain my name and
// email address in distributed copies, and let me know about any bugs

// I, Paul Stoffregen, give no warranty, expressed or implied for
// this software and/or documentation provided, including, without
// limitation, warranty of merchantability and fitness for a
// particular purpose.

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ihex.h"
#include "tools.h"
#include "up42.h"


// the memory is global
unsigned char memory[MAX_IHEX_MEM];

// Parses a line of Intel HEX code, stores the data in bytes[]
// and the beginning address in addr, and returns a 1 if the
// line was valid, or a 0 if an error occured. The variable
// num gets the number of bytes that were stored into bytes[]
int parseHexLine(char *theline, int bytes[], int *addr, 
	int *num, int *code)
{
	int sum, len, cksum;
	char *ptr;
	
	*num = 0;
	if (theline[0] != ':')
		return 0;
	if (strlen(theline) < 11)
		return 0;
	ptr = theline+1;
	if (!sscanf(ptr, "%02x", &len))
		return 0;
	ptr += 2;
	if (strlen(theline) < (11 + (len * 2)) )
		return 0;
	if (!sscanf(ptr, "%04x", addr))
		return 0;
	ptr += 4;
	// printf("Line: length=%d Addr=%d\n", len, *addr);
	if (!sscanf(ptr, "%02x", code))
		return 0;
	ptr += 2;
	sum = (len & 255) + ((*addr >> 8) & 255) + (*addr & 255) + (*code & 255);
	while(*num != len)
	{
		if (!sscanf(ptr, "%02x", &bytes[*num]))
			return 0;
		ptr += 2;
		sum += bytes[*num] & 255;
		(*num)++;
		if (*num >= 256)
			return 0;
	}
	
	if (!sscanf(ptr, "%02x", &cksum))
		return 0;
		
	// checksum error
	if ( ((sum & 255) + (cksum & 255)) & 255 )
		return 0;
	return 1;
}


// loads an intel hex file into the global memory[] array
// filename is a string of the file to be opened
int loadHexFile(const char *filename)
{
	char line[1000];
	FILE *fin = NULL;
	int addr, n, status, bytes[256];
	int i, total = 0, lineno = 1;
	int minaddr = MAX_IHEX_MEM, maxaddr = 0;

	if (strlen(filename) == 0)
	{
		printInfo(LOG_NORMAL, stderr,
			"Error: Missing HEX file name.\n");
		return -1;
	}
	fin = fopen(filename, "r");
	if (fin == NULL)
	{
		printError(stderr, "Can't open file '%s' for reading.\n", filename);
		return -1;
	}
	
	while (!feof(fin) && !ferror(fin))
	{
		line[0] = '\0';
		fgets(line, 1000, fin);
		
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		if (line[strlen(line)-1] == '\r')
			line[strlen(line)-1] = '\0';
			
		if (parseHexLine(line, bytes, &addr, &n, &status))
		{
			// Data available
			if (status == 0)
			{
				for(i=0; i<=(n-1); i++) {
					memory[addr] = bytes[i] & 255;
					total++;
					if (addr < minaddr) minaddr = addr;
					if (addr > maxaddr) maxaddr = addr;
					addr++;
				}
			}
			
			// End Of File (EOF)
			if (status == 1)
			{
				fclose(fin);
				fin = NULL;
				printInfo(LOG_DEBUG, stdout,
					"Loaded (%d) bytes between: (%04X) to (%04X)\n", 
						total, minaddr, maxaddr);
				return total;
			}
			
			// Begin Of File (BOF)
			if (status == 2)
				;
		}
		else
		{
			fclose(fin);
			fin = NULL;
			printInfo(LOG_NORMAL, stderr,
				"Error: HEX file (%s), line (%d)\n", filename, lineno);
			return -1;
		}
		
		lineno++;
	}
	
	// this should never happen
	if(fin)
		fclose(fin);
	fin = NULL;
	printInfo(LOG_NORMAL, stderr,
		"Error: No EOF record found in HEX file (%s)\n", filename);
	return -1;
}


// the command string format is "S begin end filename" where
// "begin" and "end" are the locations to dump to the intel
// hex file, specified in hexidecimal.
void save_file(char *command)
{
	int begin, end, addr;
	char *ptr, filename[200];
	FILE *fhex;

	ptr = command + 1;
	
	while (isspace(*ptr))
		ptr++;
	
	if (*ptr == '\0') {
		printf("   Must specify address range and filename\n");
		return;
	}
	
	if (sscanf(ptr, "%x%x%s", &begin, &end, filename) < 3)
	{
		printf("   Invalid addresses or filename,\n");
		printf("   usage: S begin_addr end_addr filename\n");
		printf("   the addresses must be hexidecimal format\n");
		return;
	}	
	begin &= 65535;
	end &= 65535;	
	if (begin > end) {
		printf("   Begin address must be less than end address.\n");
		return;
	}
	
	fhex = fopen(filename, "w");
	if (fhex == NULL) {
		printf("   Can't open '%s' for writing.\n", filename);
		return;
	}
	
	for (addr = begin; addr <= end; addr++)
		hexOut(fhex, memory[addr], addr, 0);
	hexOut(fhex, 0, 0, 1);
	
	printf("Memory %04X to %04X written to '%s'\n",
		begin, end, filename);
}


// Produce Intel HEX file output... call this routine with
// each byte to output and it's memory location. The file
// pointer fhex must have been opened for writing. After
// all data is written, call with end=1 (normally set to 0)
// so it will flush the data from its static buffer
void hexOut(FILE *fhex, int byte, int memory_location, int end)
{
	static int byte_buffer[MAXHEXLINE];
	static int last_mem, buffer_pos, buffer_addr;
	static int writing_in_progress = 0;
	register int i, sum;

	if (!writing_in_progress)
	{
		// initial condition setup
		last_mem = memory_location-1;
		buffer_pos = 0;
		buffer_addr = memory_location;
		writing_in_progress = 1;
	}

	if ( (memory_location != (last_mem+1)) || \
		 (buffer_pos >= MAXHEXLINE) || \
		 ((end) && (buffer_pos > 0)) )
	{
		// it's time to dump the buffer to a line in the file
		fprintf(fhex, ":%02X%04X00", buffer_pos, buffer_addr);
		sum = buffer_pos + ((buffer_addr>>8)&255) + (buffer_addr&255);
		
		for (i=0; i < buffer_pos; i++)
		{
			fprintf(fhex, "%02X", byte_buffer[i]&255);
			sum += byte_buffer[i]&255;
		}
		
		fprintf(fhex, "%02X\n", (-sum)&255);
		buffer_addr = memory_location;
		buffer_pos = 0;
	}

	if (end)
	{
		// end of file marker
		fprintf(fhex, ":00000001FF\n");
		fclose(fhex);
		writing_in_progress = 0;
	}
		
	last_mem = memory_location;
	byte_buffer[buffer_pos] = byte & 255;
	buffer_pos++;
}


const char *hex2bin(const char *hexFileName, const char *binFileName, int *byteLength)
{
	FILE *outputFile;
	const char *outputFileName = binFileName;

	printInfo(LOG_DEBUG, stdout, 
		"Load INTEL HEX file (%s)\n", 
			hexFileName);	
	int bl = loadHexFile(hexFileName);
	if (bl < 0)
		return NULL;

	// create temp file as output file
	if (!outputFileName)
		outputFileName = getTempFile((char *)prgName);
	
	printInfo(LOG_DEBUG, stdout, 
		"Save converted INTEL hex to (%s)\n", 
			outputFileName);	
	outputFile = fopen(outputFileName, "wb");
	if (!outputFile) {
		printError(stderr, "opening binary temp file (%s)", 
			outputFileName);
		return NULL;
	}
	
	fwrite(memory, bl, 1, outputFile);
	fclose(outputFile);
	
	//printf("save hex\n");
	//sprintf(wb, "S 0000 %04x test.hex.2", bl);
	//save_file(wb);
	
	//exitProgram(EXIT_OK);
	if(byteLength)
		*byteLength = bl;
	return outputFileName;
}
