/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/****************************************************************************
 *
 * This is a stand-alone program which produces "empty" disk image files
 * with specified performance figures and geometry, or assembles existing
 * data files into a single tape image file.  Disk image files are used to
 * emulate disk devices; tape image files are used to emulate cartridges to
 * be loaded in tape drive devices.
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <armProc/const.h>
#include <armProc/types.h>
#include <armProc/blockdev_params.h>
#include <armProc/aout.h>
#include <services/elf2arm.h>

#define UARMFILETYPE	".uarm"

/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/

// default disk image file name
HIDDEN char diskDflFName[] = "disk0";

// default disk header parameters (see h/blockdev.h)
HIDDEN unsigned int driveDfl[DRIVEPNUM] =	{	DFLCYL,
												DFLHEAD, 
												DFLSECT,
												DFLROTTIME, 
												DFLSEEKTIME,
												DFLDATAS
											};

//
// Program functions
//

HIDDEN void showHelp(const char * prgName);
HIDDEN int mkDisk(int argc, char * argv[]);
HIDDEN int mkTape(int argc, char * argv[]);
HIDDEN bool decodeDriveP(int idx, unsigned int * par, const char * str);
HIDDEN int writeDisk(const char * prg, const char * fname);
HIDDEN void testForCore(FILE * rfile);

// StrToWord is duplicated here from utility.cc to avoid full utility.o linking
HIDDEN bool StrToWord(const char * str, Word * value);


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

// This function scans the line arguments; if no error is found, the
// required file is produced, or a warning/help message is printed.
// See showHelp() for argument format.
// Returns an EXIT_SUCCESS/FAILURE code
int main(int argc, char* argv[])
{
    int ret = EXIT_SUCCESS;

    if (argc == 1)
        showHelp(argv[0]);
    else if (SAMESTRING("-d", argv[1]))
        ret = mkDisk(argc, argv);
    else if (SAMESTRING("-t", argv[1]))
        ret = mkTape(argc, argv);
    else {
        fprintf(stderr, "%s : Unknown argument(s)\n", argv[0]);
        showHelp(argv[0]);
        ret = EXIT_FAILURE;
    }

    return ret;
}

/****************************************************************************/
/* Definitions strictly local to the module.                                */
/****************************************************************************/

// This function prints a warning/help message on standard error
HIDDEN void showHelp(const char * prgName)
{
    fprintf(stderr, "%s syntax : %s {-d | -t} [parameters..]\n\n", prgName, prgName);
    fprintf(stderr, "%s -d <diskfile%s> [cyl [head [sect [rpm [seekt [datas]]]]]]\n\n",prgName, UARMFILETYPE);
    fprintf(stderr, "where:\n\ncyl = no. of cylinders\t\t[1..%u]\t(default = %u)\n", MAXCYL, driveDfl[CYLNUM]);
    fprintf(stderr, "head = no. of heads\t\t[1..%u]\t(default = %u)\n", MAXHEAD, driveDfl[HEADNUM]);
    fprintf(stderr, "sect = no. of sectors\t\t[1..%u]\t(default = %u)\n", MAXSECT, driveDfl[SECTNUM]); 
    fprintf(stderr, "\nrpm = disk rotations per min.\t\t[%u..%u]\t(default = %.0f)\n", MINRPM, MAXRPM, 6E7F / driveDfl[ROTTIME]);
    fprintf(stderr, "seekt = avg. cyl2cyl time (microsecs.)\t[1..%u]\t(default = %u)\n", MAXSEEKTIME, driveDfl[SEEKTIME]);
    fprintf(stderr, "datas = sector data occupation %%\t[%u%%..%u%%]\t(default = %u%%)\n", MINDATAS, MAXDATAS, driveDfl[DATASECT]); 
    fprintf(stderr, "\n<diskfile> = disk image file name\t\t(default = %s%s)\n", diskDflFName, UARMFILETYPE);
    fprintf(stderr, "\n\n%s -t <tapefile%s> <file> [file]...\n\n", prgName, UARMFILETYPE);
    fprintf(stderr, "where:\n\n<tapefile%s> = tape image file name\n\n", UARMFILETYPE);
}


// This function builds an empty disk image file, putting geometry and
// performance figures (by default or passed as command line arguments)
// in file header.
// Returns an EXIT_SUCCESS/FAILURE code 
HIDDEN int mkDisk(int argc, char * argv[])
{
	char * diskFile;
	int i;
	bool error = false;
	int ret = EXIT_SUCCESS;
	
    if (argc < 3 || argc > 9 || strstr(argv[2], UARMFILETYPE) == NULL)
	{
		// too many or too few args
		fprintf(stderr, "%s : disk image file parameters wrong/missing\n", argv[0]); 
		ret = EXIT_FAILURE;
	}
	else
	{ 
		// start argument decoding
		diskFile = argv[2];
		if (argc == 3)
			// all by default: build file image
			ret = writeDisk(argv[0], argv[2]);
		else
		{
			// scan args and places them in driveDfl[]
			for (i = 0; i < argc - 3 && !error; i++)
				error = decodeDriveP(i, &(driveDfl[i]), argv[i + 3]);	
			if (!error)
				// build file images
				ret = writeDisk(argv[0], argv[2]);
			else
			{
				fprintf(stderr, "%s : disk image file parameters wrong/missing\n", argv[0]); 
				ret = EXIT_FAILURE;
			}
		}
	}
	return(ret);
}


// This function builds a tape image file from a list of data files passed
// as command line arguments. Each data file is split into BLOCKSIZE blocks
// with a end-of-block-marker at the end; each file ends with an end-of-file
// marker and tape ends itself with an end-of tape marker. 
// Returns an EXIT_SUCCESS/FAILURE code
HIDDEN int mkTape(int argc, char * argv[])
{
    FILE * tfile = NULL;
    FILE * rfile = NULL;
    Word blk[BLOCKSIZE];
    Word tapeid = TAPEFILEID;
    Word eof = TAPEEOF;
    Word eot = TAPEEOT;
    Word eob = TAPEEOB;

    int i, j;
    int ret = EXIT_SUCCESS;

    if (argc < 4 || strstr(argv[2], UARMFILETYPE) == NULL)
    {
        // too few args
        fprintf(stderr, "%s : file name(s) wrong/missing\n", argv[0]);
        ret = EXIT_FAILURE;
    }
    else
    {
        // tries to open output file
        if ((tfile = fopen(argv[2], "w")) == NULL || \
                fwrite((void *) &tapeid, WORDLEN, 1, tfile) != 1)
            ret = EXIT_FAILURE;
        else
        {
            // loops thru arguments trying to process each data file
            for (i = 3; i < argc && ret != EXIT_FAILURE; i++)
                if((rfile = fopen(argv[i], "r")) == NULL)
                    ret = EXIT_FAILURE;
                else
                {
                    // file exists and is readable: process it
                    fclose(rfile);
                    coreElf *elf = new coreElf(argv[i]);
                    if(!elf->allRight()){
                        fprintf(stderr, "%s : error reading executable file %s : %s\n", argv[0], argv[i], elf->getError());
                        return EXIT_FAILURE;
                    }

                    // splits file into blocks inside the tape image
                    while (!elf->eof())
                    {
                        // clear block
                        for (j = 0; j < BLOCKSIZE; j++)
                            blk[j] = 0UL;

                        if (elf->read((void *) blk, WORDLEN, BLOCKSIZE) > 0)
                            // copy block to output file
                            fwrite((void *) blk, WORDLEN, BLOCKSIZE, tfile);
                        else
                            // EOF: rewind output file to write EOF/EOT marker
                            fseek(tfile, -WORDLEN, SEEK_CUR);

                        if (!elf->eof())
                            // marks end-of-block
                            fwrite((void *) &eob, WORDLEN, 1, tfile);
                        else
                            if (i < (argc - 1))
                                // more files to be processed: marks end-of-file
                                fwrite((void *) &eof, WORDLEN, 1, tfile);
                            else
                                // marks end-of-tape
                                fwrite((void *) &eot, WORDLEN, 1, tfile);
                    }
                }

            // tries to close tape image file
            if (fclose(tfile) != 0)
                ret = EXIT_FAILURE;
        }

        if (ret == EXIT_FAILURE)
            fprintf(stderr, "%s : error writing tape file image %s : %s\n", argv[0], argv[2], strerror(errno));
    }
    return(ret);
}


// This function decodes drive parameters contained in string str by
// argument position idx on the command line.
// The decoded parameter returns thru par pointer, while decoding success
// is signaled returning TRUE if an error occurred, and FALSE otherwise

HIDDEN bool decodeDriveP(int idx, unsigned int * par, const char * str)
{
	Word temp;
	bool error = false;
	
	if (!StrToWord(str, &temp))
		// error decoding parameter
		error = true;
	else 
		switch (idx)
		{
			// argument decoded by position on command line
			// min and max values are checked if needed
			
			case CYLNUM:
				if (INBOUNDS(temp, 1, MAXCYL + 1))
					*par = (unsigned int) temp;
				else 
					error = true;
				break;
			
			case HEADNUM:
				if (INBOUNDS(temp, 1, MAXHEAD + 1))
					*par = (unsigned int) temp;
				else 
					error = true;
				break;
			
			case SECTNUM:
				if (INBOUNDS(temp, 1, MAXSECT + 1))
					*par = (unsigned int) temp;
				else 
					error = true;
				break;
			
			case ROTTIME:
				if (INBOUNDS(temp, MINRPM, MAXRPM + 1))
					*par = (unsigned int) (6E7F / temp);
				else 
					error = true;
				break;
			
			case SEEKTIME:
				if (INBOUNDS(temp, 1, MAXSEEKTIME + 1))
					*par = (unsigned int) temp;
				else 
					error = true;
				break;
			
			case DATASECT:
				if (INBOUNDS(temp, MINDATAS, MAXDATAS + 1))
					*par = (unsigned int) temp;
				else 
					error = true;
				break;
			
			default:
				// unknown parameter
			error = true;
		}
	return(error);			
}


// This function creates the disk image file on the disk, prepending it with 
// a header containing geometry and performance figures.
// A number of 512-byte empty blocks is created, depending on disk geometry.
// Returns an EXIT_SUCCESS/FAILURE code
HIDDEN int writeDisk(const char * prg, const char * fname)
{
	FILE * dfile = NULL;
	int ret = EXIT_SUCCESS;
	
	unsigned int i;
	unsigned int dfsize = driveDfl[CYLNUM] * driveDfl[HEADNUM] * driveDfl[SECTNUM];
	Word blk[BLOCKSIZE];
	Word diskid = DISKFILEID;
	
	// clears block
	for (i = 0; i < BLOCKSIZE; i++)
		blk[i] = 0;

	// tries to open image file and write header	
	if ((dfile = fopen(fname, "w")) == NULL || \
		fwrite((void *) &diskid, WORDLEN, 1, dfile) != 1 || \
		fwrite((void *) driveDfl, sizeof(unsigned int), DRIVEPNUM, dfile)  != DRIVEPNUM)
		ret = EXIT_FAILURE;
	else
	{
		// writes empty blocks 
		for (i = 0; i < dfsize && ret != EXIT_FAILURE; i++)
			if (fwrite((void *) blk, WORDLEN, BLOCKSIZE, dfile) != BLOCKSIZE)
				ret = EXIT_FAILURE;
		if (fclose(dfile) != 0)
			ret = EXIT_FAILURE;
	}
	
	if (ret == EXIT_FAILURE)
		fprintf(stderr, "%s : error writing disk file image %s : %s\n", prg, fname, strerror(errno));

	return(ret);
}


// This function tests if file to be inserted is a .core file; if so the
// magic file tag should be skipped for alignment reasons, else file should
// be inserted as-is
HIDDEN void testForCore(FILE * rfile)
{
	Word tag;
	
	// file existence have been tested by caller
	if (fread((void *) &tag, WORDLEN, 1, rfile) == 0 || tag != COREFILEID)
		// file is empty or not .core
		rewind(rfile);
	// else file is .core: leave file position as it is to skip tag
}


// This function converts a string to a Word (typically, an address) value.
// Returns TRUE if conversion was successful, FALSE otherwise
HIDDEN bool StrToWord(const char * str, Word * value)
{
	char * endp;
	bool valid = true;

	// tries to convert the string into a unsigned long
	*value = strtoul(str, &endp, 0); 
	
	if (endp != NULL)
	{
		// there may be some garbage
		while (*endp != EOS && valid)
		{
			if (!isspace(*endp))
				valid = false;
			endp++;
		}
	}
	return(valid);
} 

	
