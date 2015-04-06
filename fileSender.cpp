#include "FileSender.h"
#include "md5.h"


FileSender::~FileSender()
{

	for (int i=0; i<additionFloppies.size(); i++)
	{
		delete additionFloppies[i];
	}
	additionFloppies.clear();
}


// receive SYNC_METADATA_TYPE floopy from receiver side
void FileSender::receiveFloppy(char* buffer)
{
	Floppy *fp = new Floppy((PacketBuffer*)buffer);

	//cout<<"S: recvFloppy\n";
	if (fp->getType() == SYNC_METADATA_TYPE)
	{

		//if (string(&fp->getMetaData()->fileName).compare(string(&metaDataFloppy->getMetaData()->fileName))==0)
		if (fp->getFileID() == getFileID())
		{
			//cout<<"same file\n";
			// we are talking about the same file

			vector<Floppy*>::iterator it = peerInfoFloppies.begin();
			while (it != peerInfoFloppies.end())
			{
				if (fp->getOffset() >= (*it)->getOffset() + (*it)->getLen())
				{
					it++;
				}
				else if (fp->getOffset() + fp->getLen() <= (*it)->getOffset())
				{
					peerInfoFloppies.insert(it, fp);
					//cout<<"insert M\n";
					return;
				}
				else
				{
					// we should not be here
					cout<< "error:" << fp->getFileID() << "/" << metaDataFloppy->getFileID()<<"\n";
					return;
				}
			}

			// is not able to insert payload in the middle, then insert it at the end
			peerInfoFloppies.insert(it, fp);
		}

	}
}


// find the difference by comparing the block metaData between peerInfoFloppies and payloadFloppies.
// payloadFloppies stores the content of the latest version of the file;
// peerInfoFloppies stores the blocks' metadata information of the receiver side.
void FileSender::findDiff()
{
	int bufSize=0, chunkSize, sameBlock=0, ind=0;
	char *buffer;
	ChunkMetaData *chunk;


	// let's ignore the gaps here, assume all infor floppies arrive safely.
	for (vector<Floppy*>::iterator it = peerInfoFloppies.begin() ; it != peerInfoFloppies.end(); ++it)
	{
		bufSize += (*it)->getLen();
	}

	chunkSize = bufSize/sizeof(ChunkMetaData);
	buffer = new char[bufSize];

	for (vector<Floppy*>::iterator it = peerInfoFloppies.begin() ; it != peerInfoFloppies.end(); ++it)
	{
		memcpy((void*)&buffer[ind], (void*)(*it)->getPayload(), (*it)->getLen());
		ind += (*it)->getLen();
	}

//	cout<<"-------new file blocks---------\n";
//	for (int i=0; i<payloadFloppies.size(); i++)
//	{
//		cout<<payloadFloppies[i]->getOffset() <<","<<payloadFloppies[i]->getLen() <<","<<md5(string(payloadFloppies[i]->getPayload(), payloadFloppies[i]->getLen()))<<"\n";
//	}
//
//	cout<<"-------old file blocks---------\n";
//	chunk = (ChunkMetaData*)buffer;
//	for (int i=0; i<chunkSize; i++)
//	{
//		cout<< chunk->offset <<","<< chunk->len <<","<< string(chunk->md5,32) <<"\n";
//		chunk++;
//	}


	//cout << peerInfoFloppies.size() << "," << chunkSize<<"\n";

	for (int j=0; j<payloadFloppies.size(); j++)
	{
		sameBlock=0;
		chunk = (ChunkMetaData*)buffer;
		for (int i=0; i<chunkSize; i++)
		{
			if ( chunk->offset == payloadFloppies[j]->getOffset() &&
				 chunk->len == payloadFloppies[j]->getLen() &&
				 memcmp(chunk->md5, md5(string(payloadFloppies[i]->getPayload(), payloadFloppies[i]->getLen())).c_str(), CHKSUM_LEN)==0 )
			{
				cout<<"same block: " << chunk->offset <<"," << chunk->len << "," << string(chunk->md5, CHKSUM_LEN) << "\n";
				sameBlock=1;
				break;
			}
			chunk++;

		}

		if (!sameBlock)
		{
			additionFloppies.push_back(payloadFloppies[j]);
		}
	}

	cout<<"Need update: " << additionFloppies.size() << "/" << payloadFloppies.size() << " blocks\n";

}


// sender read the file and extract the metadat into metaDataFloppy, and payload into payloadFloppies.
void FileSender::prepareFileToFloppies(string fileName, int fileID)
{
	ifstream infile;
	int readSize=0, readingSize;
	char buffer[PAYLOAD_LEN+1];
	FileMetaData *metaData;
	Floppy* floppy = NULL;
	int fileSize = getFileSize(fileName);
	int chunkNum=0;
	string fileContent;

	//cout << fileName << ".txt , " << fileSize << " " << payloadFloppies.size() <<"\n";

	if (fileSize==-1)
	{
		// error, and end the program
		cout << "Error: file stat, wrong file size.";
		return;
	}

	infile.open(fileName+".txt");
	while (readSize < fileSize)
	{
		readingSize = ((fileSize-readSize) < PAYLOAD_LEN) ? fileSize-readSize:PAYLOAD_LEN;
		try
		{
			memset(buffer, 0, PAYLOAD_LEN+1);
			infile.read(buffer, readingSize);
		}
		catch (int e)
		{
			// error, and end the program
			cout << "Error: file read.\n";
			return;
		}

		floppy = new Floppy(fileID, readSize, PAYLOADDATA_TYPE, buffer, readingSize);
		fileContent += string(buffer, readingSize);
		//cout<<string(buffer, readingSize).length()<<"/"<<readingSize<<"\n";
		chunkNum++;

		payloadFloppies.push_back(floppy);
		readSize += readingSize;
	}

	// generate metaDataFLoppy
	metaData = (FileMetaData*) new char[PAYLOAD_LEN];
	memset(metaData, 0, PAYLOAD_LEN);
	string md5Str = md5(fileContent);
	//cout<<md5Str<<","<<fileContent.length()<<"\n";
	memcpy(metaData->md5, md5Str.c_str(), CHKSUM_LEN);
	memcpy(&metaData->fileName, fileName.c_str(), fileName.length());
	metaData->fileID = fileID;
	metaData->floppiesNum = chunkNum;
	metaData->fileSize = readSize;

	metaDataFloppy = new Floppy(fileID, 0, NEW_METADATA_TYPE, (char*)metaData, PAYLOAD_LEN);

	//cout<<string(&metaDataFloppy->getMetaData()->fileName)<<" \n";
	infile.close();
}


char* FileSender::sendNextPayloadFloppy()
{
	return sendNextFloppy(payloadFloppies);
}

char* FileSender::sendNextAdditionFloppy()
{
	return sendNextFloppy(additionFloppies);
}


