#ifndef _FILESENDER_H_
#define _FILESENDER_H_

#include "file.h"

class FileSender: public File
{
private:
	vector<Floppy*> additionFloppies;
	vector<Floppy*> peerInfoFloppies;
public:
	FileSender() {}
	~FileSender();

	void prepareFileToFloppies(string fileName, int fileID);
	char* sendNextPayloadFloppy();
	char* sendNextAdditionFloppy();
	void receiveFloppy(char* buffer);
	void findDiff();
};


#endif
