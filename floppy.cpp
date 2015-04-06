

#include "floppy.h"
using namespace std;

Floppy::Floppy(int id, int readSize, int ftype, char* buffer, int bufferSize)
{
	fileID=id;
	type=ftype;
	offset = readSize;
	len = bufferSize;
	memcpy(payload, buffer, bufferSize);
}


Floppy::Floppy(PacketBuffer* buffer)
{
	fileID=buffer->fileID;
	type=buffer->type;
	offset = buffer->offset;
	len = buffer->len;
	memcpy(payload, buffer->payload, buffer->len);
}


int Floppy::getOffset()
{
	return offset;
}


int Floppy::getType()
{
	return type;
}


int Floppy::getLen()
{
	return len;
}

char* Floppy::getPayload()
{
	return payload;
}

int Floppy::getFileID()
{
	return fileID;
}

FileMetaData* Floppy::getMetaData()
{
	if (type == NEW_METADATA_TYPE)
	{
		return (FileMetaData*)payload;
	}
	else
	{
		return NULL;
	}
}

unsigned int Floppy::updateCRC(PacketBuffer *buffer)
{
	return crc32((unsigned char*)buffer, DATA_LEN);
}


int Floppy::checkCRC(PacketBuffer *buffer)
{
	unsigned int crc = crc32((unsigned char*)buffer, DATA_LEN);

	//cout<<"check CRC:" << buffer->crc << "|" << crc  << "\n";
	if (crc != buffer->crc )
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


// copy the floppy
Floppy* Floppy::copy()
{
	return new Floppy(fileID, offset, type, payload, len);
}


// this function exports all member varialbes to a buffer
char* Floppy::copyBuffer()
{
	PacketBuffer* buffer = (PacketBuffer*) new char[sizeof(PacketBuffer)];

	buffer->fileID=fileID;
	buffer->type=type;
	buffer->len = len;
	buffer->offset = offset;
	memcpy(buffer->payload, payload, PAYLOAD_LEN);
	buffer->crc = updateCRC(buffer);

	return (char*)buffer;
}
