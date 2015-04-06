
#include "file.h"
#include "floppy.h"
#include "md5.h"





int File::getFileSize(string fileName)
{
	int ret=-1;

	ifstream is (fileName + ".txt", ifstream::binary);
	if (is)
	{
	    // get length of file:
		is.seekg (0, is.end);
		ret = is.tellg();
		is.close();
	}

	return ret;
}



void File::clear()
{

	if (metaDataFloppy)
	{
		delete metaDataFloppy;
		metaDataFloppy=NULL;
	}

	for (int i=0; i<payloadFloppies.size(); i++)
	{
		delete payloadFloppies[i];
	}
	payloadFloppies.clear();
}



void File::status(string option)
{
	FileMetaData* metaData = metaDataFloppy->getMetaData();

	if (metaData)
	{
		cout << "File name: " << &metaData->fileName << ".txt\n";
		cout << "File (" << option << ") size: " << metaData->fileSize << "\n";
		cout << "File MD5: " << string(metaData->md5, CHKSUM_LEN) << "\n";
	}
}


string File::getFileName()
{
	FileMetaData* metaData = metaDataFloppy->getMetaData();
	if (metaData)
	{
		return string(&metaData->fileName);
	}
	return NULL;
}


string File::resembleFileMD5()
{
	string content;

	for (vector<Floppy*>::iterator it = payloadFloppies.begin() ; it != payloadFloppies.end(); ++it)
	{
		content += string((*it)->getPayload(), (*it)->getLen());
	}

	//cout<< "resembleFileMD5:" << content.length() << "\n";
	return md5(content);
}


char* File::floppiesMD5()
{
	char* buffer = new char[sizeof(ChunkMetaData)*payloadFloppies.size()];
	string md5Str;
	ChunkMetaData* chunk = (ChunkMetaData*)buffer;

	for (vector<Floppy*>::iterator it = payloadFloppies.begin() ; it != payloadFloppies.end(); ++it)
	{
		chunk->offset = (*it)->getOffset();
		chunk->len = (*it)->getLen();
		md5Str = md5(string((*it)->getPayload(), chunk->len));
		memcpy(chunk->md5, md5Str.c_str(), CHKSUM_LEN);

		//cout<< chunk->offset <<","<< chunk->len <<","<< string(chunk->md5,32) <<"\n";

		chunk++;
	}

	//cout<<"floppiesMD5:"<< payloadFloppies.size()<<"\n";
	return buffer;
}


int File::getFileID()
{
	if (metaDataFloppy)
	{
		return metaDataFloppy->getMetaData()->fileID;
	}
	else
	{
		return -1;
	}

}


// pack buffer into a vector of floppies
void File::bufferToFloppies(vector<Floppy*> &floppies, char* buffer, int buffSize, int type)
{
	int ind=0, readingSize=0;
	Floppy *floppy=NULL;

	// send this missing part information back to sender
	while (ind < buffSize)
	{
		readingSize = ((buffSize-ind) < PAYLOAD_LEN) ? buffSize-ind:PAYLOAD_LEN;

		floppy = new Floppy(getFileID(), ind, type, &buffer[ind], readingSize);

		floppies.push_back(floppy);
		ind += readingSize;
	}
}

char* File::sendMetaDataFloppy()
{
	return metaDataFloppy->copyBuffer();
}


char* File::sendNextFloppy(vector<Floppy*> &floppies)
{
	char* ret=NULL;

	//cout<<"sendNextFloppy " << floppies.size()  <<"\n";

	//cout << payloadFloppies.size() << "\n";
	if (floppies.size()>0)
	{
		ret= (*floppies.begin())->copyBuffer();
		delete (*floppies.begin());
		floppies.erase(floppies.begin());
	}

	return ret;
}





