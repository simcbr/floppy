#include "fileReceiver.h"
#include "fileSender.h"
#include "floppy.h"
#include <random>

#define MAX_FILE_SIZE 100000
#define ADD_FILE_SIZE 10000

static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// generate specific num of random files
vector<string> generateRandomFiles(int num)
{
	string str;
	int fileSize=0;

	default_random_engine rng;
	std::uniform_int_distribution<> dist (0, sizeof(alphabet) - 1);

	vector<string> fileNames;

	srand (time(NULL));
	for (int i = 0; i < num; i++)
	{
		str="";
		fileSize = rand() % MAX_FILE_SIZE;

		ofstream outFile;
		outFile.open (to_string(fileSize) + ".txt");

		for (int j=0; j<fileSize; j++)
		{
			//str += alphabet[rand()%sizeof(alphabet)];
			str += alphabet[dist(rng)];
		}

		//cout<<str.length()<<"\n";
		outFile << str;

		outFile.close();

		fileNames.push_back(to_string(fileSize));
	}

	return fileNames;
}


// append random addition data into previous generated files
void updateFiles(string fileName)
{
	string str;
	int fileSize=0;
	default_random_engine rng;
	std::uniform_int_distribution<> dist (0, sizeof(alphabet) - 1);

	ofstream outFile;
	outFile.open(fileName + ".txt", std::ofstream::out | std::ofstream::app);
	str.clear();
	fileSize = rand() % ADD_FILE_SIZE;

	for (int j=0; j<fileSize; j++)
	{
		str += alphabet[dist(rng)];
	}

	outFile << str;

	outFile.close();
}



//Given a file of arbitrary size, write a function that can split the file into “n”
//floppy disks and then write the function to reassemble the file from “n” floppy disks.
//The first disk is guaranteed to arrive intact first, but every other disk may arrive
//in any order, and there may be some extraneous disks from some other file(s) mixed in
//which got mis-delivered.  By the end of the process, the recipient must verify that it
//has an identical file to the sender to a very high level of certainty, perhaps using a
//cryptographic integrity check or similar mechanism.

/*
 * In this function, we will simulate above process.
 *  There is one sender and one receiver. The sender will send all random files through
 *  floppies to the receiver. Each random file's first chunk (floppy) will be first sent
 *  to receiver. Receiver will only resemble the file according to the first floppy it received.
 *
 *
 */
string task1(vector<string> fileNames, FileSender *sender, FileReceiver *receiver)
{
	vector<char*> payloadFloppyStack;
	vector<char*> metaDataFloppyStack;
	char *chunk;
	int fileID=0;

	cout<<"\nTransmit Task#############################\n";

	cout<<"Prepare random files:\n";
	for (vector<string>::iterator it = fileNames.begin() ; it != fileNames.end(); ++it)
	{

		sender->clear();

		sender->prepareFileToFloppies(*it, fileID);
		fileID++;

		sender->status("sender");

		// send metaDataFloppy first
		chunk = sender->sendMetaDataFloppy();
		metaDataFloppyStack.push_back(chunk);

		chunk = sender->sendNextPayloadFloppy();
		while (chunk)
		{
			payloadFloppyStack.push_back(chunk);
			chunk = sender->sendNextPayloadFloppy();
		}

	}

	// send metadata to receiver in random order
	cout<<"\nS: sending files' metaData to receiver\n";
	while (!metaDataFloppyStack.empty())
	{
		int ind = rand() % metaDataFloppyStack.size();
		receiver->receiveFloppy(metaDataFloppyStack[ind]);
		cout<<"M";
		metaDataFloppyStack.erase(metaDataFloppyStack.begin() + ind);
	}

	cout<<"  ==>100% sending metaData end\n";

	// send payload floppies to receiver in random order
	cout<<"\nS: sending payload to receiver\n";
	while (!payloadFloppyStack.empty())
	{
		int ind = rand() % payloadFloppyStack.size();
		//cout<<ind<<"/"<<payloadFloppyStack.size()<< "\n";
		//cout<<"=";
		receiver->receiveFloppy(payloadFloppyStack[ind]);
		payloadFloppyStack.erase(payloadFloppyStack.begin() + ind);
	}

	cout<<"  ==>100% sending payload end\n";

	// statistic of receiver
	receiver->resembleFloppiesToFile();

	receiver->recvStatus();

	return receiver->getFileName();
}


// Assume the floppy disks are very heavy, and you want to minimize shipping costs.
// On the upside, the recipient may already have the file, or a substantially similar file.
// Get the new file to the recipient while sending only the minimal number of floppy disks
// each direction, even if some floppies get reordered, lost or corrupted en route
// (but again, the first disk is guaranteed to arrive first, intact).
void task2(string processingFileName, FileSender *sender, FileReceiver *receiver)
{
	char *chunk;
	vector<char*> inforFloppyStack;
	vector<char*> syncFloppyStack;

	cout<<"\nSync Task#############################\n";

	sender->clear();
	// sender side send new file infor metadata to receiver
	sender->prepareFileToFloppies(processingFileName, 0);
	chunk = sender->sendMetaDataFloppy();

	// receiver side generate file block infor and send back to sender side
	receiver->receiveFloppy(chunk);

	// receiver side send these infor blocks to sender
	chunk = receiver->sendNextInforFloppy();
	while (chunk)
	{
		inforFloppyStack.push_back(chunk);
		chunk = receiver->sendNextInforFloppy();
	}

	cout<<"R: sending file infor\n";
	while (!inforFloppyStack.empty())
	{
		int ind = rand() % inforFloppyStack.size();
		sender->receiveFloppy(inforFloppyStack[ind]);
		inforFloppyStack.erase(inforFloppyStack.begin() + ind);
	}

	// sender find the difference, and decide which blocks should be sent
	sender->findDiff();

	// sender side send these addition floppies to receiver
	chunk = sender->sendNextAdditionFloppy();
	while (chunk)
	{
		syncFloppyStack.push_back(chunk);
		chunk = sender->sendNextAdditionFloppy();
	}

	cout<<"S: sending sync blocks\n";
	while (!syncFloppyStack.empty())
	{
		int ind = rand() % syncFloppyStack.size();
		receiver->receiveFloppy(syncFloppyStack[ind]);
		syncFloppyStack.erase(syncFloppyStack.begin() + ind);
	}

	cout<<"  ==>100% sending update payload end\n";

	// statistic of receiver
	receiver->resembleFloppiesToFile();

	receiver->recvStatus();
}


int main(int argc, char* argv[])
{
    if (argc !=2)
    {
        printf("Usage: fileTransmit [number of random files]\n");
        return -1;
    }

    int randomFilesNum  = atoi(argv[1]);
    vector<string> fileNames;
    string processingFileName;
    FileSender *sender;
    FileReceiver *receiver;

	sender = new FileSender();
	receiver = new FileReceiver();

    // rm existing .txt files
    system("rm *.txt");

    fileNames = generateRandomFiles(randomFilesNum);

    // send frile to reciver
    processingFileName = task1(fileNames, sender, receiver);

    //==============================
    updateFiles(processingFileName);

    // receiver sync file from sender
    task2(processingFileName, sender, receiver);

}
