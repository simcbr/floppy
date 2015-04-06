#include "fileReceiver.h"




FileReceiver::~FileReceiver()
{
	gaps.clear();
	for (int i=0; i<missingInforFloppies.size(); i++)
	{
		delete missingInforFloppies[i];
	}
	missingInforFloppies.clear();
}


// report the received file information
void FileReceiver::recvStatus()
{
	FileMetaData* metaData = metaDataFloppy->getMetaData();

	cout << "RecvStatus-------------------:\n";
	cout << " "<< payloadFloppies.size() << "/" << metaData->floppiesNum << " payload floppies\n";
	cout << " Missing "<< gaps.size() << " blocks :\n";
	for (vector<pair<int,int> >::iterator it = gaps.begin() ; it != gaps.end(); ++it)
	{
		cout << get<0>(*it) << "-" << get<1>(*it) << "\n";
	}

	if (metaData)
	{
		cout << " File name: " << &metaData->fileName << ".txt\n";
		cout << " File size: " << metaData->fileSize << " \n";
		cout << " MD5: " << string(metaData->md5, CHKSUM_LEN) << " \n";

		if (gaps.size()==0)
		{
			string md5Str = resembleFileMD5();
			// calculate MD5 for received file
			cout << " Verify MD5: " << md5Str;
			if (memcmp(md5Str.c_str(), metaData->md5, CHKSUM_LEN)==0)
			{
				cout<<"(correct)\n";
			}
			else
			{
				cout<<"(incorrect)\n";
			}
		}
	}

}




// receive different types of floopies: NEW_METADATA_TYPE, PAYLOADDATA_TYPE
void FileReceiver::receiveFloppy(char* buffer)
{
	Floppy *fp = new Floppy((PacketBuffer*)buffer);
	char *infor;

	if (fp->getType() == NEW_METADATA_TYPE)
	{
		if (metaDataFloppy==NULL)
		{
			// let's start to receive this file
			// save all these information into metaDataFloppy

			cout<<"update metaData\n";
			//metaDataFloppy = new Floppy(metaData->fileID, -1, (char *)metaData, PAYLOAD_LEN);
			metaDataFloppy = fp;

		}
		else
		{
			if (string(&fp->getMetaData()->fileName).compare(string(&metaDataFloppy->getMetaData()->fileName))==0)
			{

				// we are talking about the same file
				if ( fp->getMetaData()->fileSize != metaDataFloppy->getMetaData()->fileSize ||
					memcmp(fp->getMetaData()->md5, metaDataFloppy->getMetaData()->md5, CHKSUM_LEN) )
				{
					cout << "Different version of file " << string(&fp->getMetaData()->fileName)  << ".txt found, synchronizing...\n";
					cout << "  curFile: size (" <<  metaDataFloppy->getMetaData()->fileSize << "), MD5 ("
							<<  string(metaDataFloppy->getMetaData()->md5, CHKSUM_LEN) << ")\n";
					cout << "  newFile: size (" <<  fp->getMetaData()->fileSize << "), MD5 ("
							<<  string(fp->getMetaData()->md5, CHKSUM_LEN) << ")\n";
					delete metaDataFloppy;
					metaDataFloppy = fp;    // use peer's metaData (also use peer's fileID)

					// generate current file's floppies information
					infor = floppiesMD5();

					// pack this infor into fileInfoFloppies
					bufferToFloppies(fileInfoFloppies, infor, sizeof(ChunkMetaData)*payloadFloppies.size(), SYNC_METADATA_TYPE);

					cout<<"Prepare "<< fileInfoFloppies.size() << " floppies for file info (" << payloadFloppies.size() << ") chunk data \n";

				}
			}
			else
			{
				//cout<<"different file\n";
				delete fp;  // discard other files' metaData since we only need the first one.
			}
		}

	}
	else
	{
		// check CRC of received floppy
		if (fp->checkCRC((PacketBuffer*)buffer)!=0)
		{
			//cout<< "Wrong CRC: discard payload floppy\n";
			cout<<"W";
			return;
		}


		// check if it belongs to current receiving file
		if (metaDataFloppy && fp->getFileID() == metaDataFloppy->getFileID() &&
			fp->getType() == PAYLOADDATA_TYPE )
		{
			cout<<"S";

			vector<Floppy*>::iterator it = payloadFloppies.begin();
			while (it != payloadFloppies.end())
			{
				if (fp->getOffset() >= (*it)->getOffset() + (*it)->getLen())
				{
					it++;
				}
				else if (fp->getOffset() + fp->getLen() <= (*it)->getOffset())
				{
					payloadFloppies.insert(it, fp);
					//cout<<"insert M\n";
					return;
				}
				else
				{
					if (fp->getOffset() == (*it)->getOffset())
					{
						// update existing block
						delete (*it);
						*it = fp;

						return;

					}
					else
					{
						cout<< "\nerror:" << fp->getFileID() << "/" << metaDataFloppy->getFileID()<<"\n";
						return;
					}

				}
			}

			// is not able to insert payload in the middle, then insert it at the end
			payloadFloppies.insert(it, fp);
			//cout<<"insert E\n";

		}
		else
		{
			// discard the floppy belongs to other files
			cout<<"D";
			delete fp;
		}

	}

	free(buffer);

}


//check the gaps between received blocks.
void FileReceiver::resembleFloppiesToFile()
{
	int offset=0;

	// empty gaps
	gaps.clear();

	for (vector<Floppy*>::iterator it = payloadFloppies.begin() ; it != payloadFloppies.end(); ++it)
	{
		// since the floppies are inserted in an order, we only check whether there is a gap between floppies.
		if (offset==(*it)->getOffset())
		{
			offset += (*it)->getLen();
		}
		else if (offset < (*it)->getOffset())
		{
			// there is a gap
			gaps.push_back(make_pair(offset, (*it)->getOffset() - offset));
		}
		else
		{
			// this should not happen, if floppies are sorted in order
			cout << "File::resembleFloppiesToFile: wrong order.";
		}
	}
}


void FileReceiver::prepareMissingInforFloppies()
{
	int buffSize = gaps.size() * sizeof(unsigned int) * 2;
	char *buffer = new char[buffSize];
	unsigned int *ptr = (unsigned int*) buffer;
	int fileID;
	FileMetaData* metaData = metaDataFloppy->getMetaData();

	if (metaData)
	{
		fileID = metaData->fileID;
	}
	else
	{
		cout<<"Error: no file metaData infor exist\n";
		return;
	}


	for (vector<pair<int,int> >::iterator it = gaps.begin() ; it != gaps.end(); ++it)
	{
		cout << get<0>(*it) << "-" << get<1>(*it) << "\n";
		*ptr++ = get<0>(*it);
		*ptr++ = get<1>(*it);
	}

	// send this missing part information back to sender
	bufferToFloppies(missingInforFloppies, buffer, buffSize, MISSINGINFOR_TYPE);
}


char* FileReceiver::sendNextInforFloppy()
{
	return File::sendNextFloppy(fileInfoFloppies);
}

char* FileReceiver::sendNextMissingInforFloppy()
{
	return File::sendNextFloppy(missingInforFloppies);}
