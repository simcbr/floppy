#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "crc32.h"

#define CHKSUM_LEN 32
#define PAYLOAD_LEN 1024-sizeof(unsigned int)*5
#define DATA_LEN 1020
#define FLOPPY_SIZE 1024

#define PAYLOADDATA_TYPE 0
#define NEW_METADATA_TYPE -1
#define SYNC_METADATA_TYPE -2
#define MISSINGINFOR_TYPE -3

// the metadata of each block
typedef struct
{
	int offset;
	int len;
	char md5[CHKSUM_LEN];
}ChunkMetaData;


// the metadata of the whole file
typedef struct
{
	char md5[CHKSUM_LEN];  // md5 of the whole file
	int fileSize;
	int fileID;
	int floppiesNum;   // how many payload floppies
	char fileName;
} FileMetaData;

// the structure of each floppy, crc is used to check the transmission correctness of each floppy.
typedef struct
{
	int fileID;
	int type;
	int offset;
	int len;
	char payload[PAYLOAD_LEN];
	unsigned int crc;
} PacketBuffer;

class Floppy
{
	int fileID;
	int type;
	int offset;
	int len;
	char payload[PAYLOAD_LEN];

public:
	Floppy(int id, int readSize, int ftype, char* buffer, int bufferSize);
	Floppy(PacketBuffer* buffer);
	int getOffset();
	int getType();
	int getLen();
	int getFileID();
	FileMetaData* getMetaData();
	char* getPayload();
	Floppy* copy();
	char* copyBuffer();
	unsigned int updateCRC(PacketBuffer *buffer);
	int checkCRC(PacketBuffer *buffer);
};

#endif
