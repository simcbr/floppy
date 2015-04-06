#ifndef _FILERECEIVER_H_
#define _FILERECEIVER_H_

#include "file.h"

class FileReceiver: public File
{
private:
	vector<Floppy*> fileInfoFloppies;
	vector<Floppy*> missingInforFloppies;
public:
	FileReceiver() {}
	~FileReceiver();

	void receiveFloppy(char* buffer);
	char* sendNextMissingInforFloppy();
	char* sendNextInforFloppy();
	void resembleFloppiesToFile();
	void prepareMissingInforFloppies();


	void recvStatus();
};


#endif
