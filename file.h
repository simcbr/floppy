#ifndef _FILE_H_
#define _FILE_H_
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include "floppy.h"
using namespace std;


#define LOGGER cout<< __FILE__ << "," << __LINE__ << "\n"

class File
{
protected:

	Floppy* metaDataFloppy;
	vector<Floppy*> payloadFloppies;
	vector<pair<int,int> > gaps;

public:
	File() {metaDataFloppy=NULL;}
	~File() {delete metaDataFloppy;}
	void clear();

	void prepareMissingInforFloppies();
	int getFileSize(string fileName);
	void status(string option);
	char* sendMetaDataFloppy();
	string getFileName();
	string resembleFileMD5();
	char* floppiesMD5();
	int getFileID();
	void bufferToFloppies(vector<Floppy*> &floppies, char* buffer, int buffSize, int type);
	char* sendNextFloppy(vector<Floppy*> &floppies);
};

#endif
