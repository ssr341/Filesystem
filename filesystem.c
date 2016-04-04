// Stephen Russo 
// Filesystem using FUSE

// Sorry, but I couldn't really figure out how to make it work with FUSE. Mkdir does not work. From my understanding, you would write in the name of the 
// directory you want to make the new directory in and then add that new directory to the parent's inode list. When attempting to do that in FUSE, it says
// file already exsits, and from my understanding it was supposed to. Init works! Not sure what destroy is really supposed to do as I'm not sure where to
// store file handles. I originally thought it was supposed to delete all files. I have it set up that anything that opens a file closes that same file,
// so I had no need for handles. getattr is commented out because it to seg fault when mounted in FUSE, I don't know why. It compiles okay besides for 
// that. In Visual Studio it works perfect! And by perfect, I mean how I thought it was supposed to work. All of the commented out code is what I used 
// to debug it in Visual Studio.

//#define FUSE_USE_VERSION 30
//#include <fuse.h>
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

struct Block{
	time_t creationTime;
	int mounted;
	int devId;
	int freeStart;
	int freeEnd;
	int root;
	int maxBlocks;
};
struct Block super;
int directoryArray[10000];
int numDir = 0;
int inodeArray[10000];
int numInode = 0;

//static int file_getattr(const char *path, struct stat *stbuf){
////static int getattr(const char* path){
//	int res = 0;
//	memset(stbuf, 0, sizeof(struct stat));
//
//	// read file to see what type it is
//	FILE* file = fopen(path, "r");
//	fseek(file, 0, SEEK_END);
//	int fileSize = ftell(file);
//	char* fileInfo = (char*)malloc(fileSize);
//	fseek(file, 0, SEEK_SET);
//	fread(fileInfo, 1, fileSize, file);
//
//	char* isDir = strstr(fileInfo, "filename");
//	char* isInode = strstr(fileInfo, "indirect");
//
//	// check if directory
//	if (isDir != NULL){
//		fseek(file, 0, SEEK_SET);
//		char size[20];
//		char uid[20];
//		char gid[20];
//		char mode[20];
//		char atime[20];
//		char ctime[20];
//		char mtime[20];
//		char linkcount[20];
//		char string[200];
//		fscanf(file, "%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],", 
//			string, size, string, uid, string, gid, string, mode, string, atime, string, ctime, string, mtime, string, linkcount);
//		//printf("%s %s %s %s %s %s %s", size, uid, mode, atime, ctime, mtime, linkcount);
//		int isize = atoi(size);
//		int iuid = atoi(uid);
//		int igid = atoi(gid);
//		int imode = atoi(mode);
//		int iatime = atoi(atime);
//		int ictime = atoi(ctime);
//		int imtime = atoi(mtime);
//		int ilinkcount = atoi(linkcount);
//		//printf("%i %i %i %i %i %i %i %i", isize, iuid, igid, imode, iatime, ictime, imtime, ilinkcount);
//		stbuf->st_size = isize;
//		stbuf->st_uid = iuid;
//		stbuf->st_gid = igid;
//		stbuf->st_mode = S_IFDIR | 0755;
//		//stbuf->st_mode = imode;
//		stbuf->st_atime = iatime;
//		stbuf->st_ctime = ictime;
//		stbuf->st_mtime = imtime;
//		stbuf->st_nlink = ilinkcount;
//	}
//
//	// check if inode
//	else if (isInode != NULL){
//		fseek(file, 0, SEEK_SET);
//		char size[20];
//		char uid[20];
//		char gid[20];
//		char mode[20];
//		char linkcount[20];
//		char atime[20];
//		char ctime[20];
//		char mtime[20];
//		char string[200];
//		fscanf(file, "%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],", 
//			string, size, string, uid, string, gid, string, mode, string, linkcount, string, atime, string, ctime, string, mtime);
//		//printf("%s %s %s %s %s %s %s", size, uid, mode, atime, ctime, mtime, linkcount);
//		int isize = atoi(size);
//		int iuid = atoi(uid);
//		int igid = atoi(gid);
//		int imode = atoi(mode);
//		int iatime = atoi(atime);
//		int ictime = atoi(ctime);
//		int imtime = atoi(mtime);
//		int ilinkcount = atoi(linkcount);
//		//printf("%i %i %i %i %i %i %i %i", isize, iuid, igid, imode, iatime, ictime, imtime, ilinkcount);
//		stbuf->st_size = isize;
//		stbuf->st_uid = iuid;
//		stbuf->st_gid = igid;
//		//stbuf->st_mode = imode;
//		stbuf->st_mode = S_IFREG | 0444;
//		stbuf->st_atime = iatime;
//		stbuf->st_ctime = ictime;
//		stbuf->st_mtime = imtime;
//		stbuf->st_nlink = ilinkcount;
//	}
//
//	else
//		res = -ENOENT;
//
//	fclose(file);
//	return res;
//}

//static int file_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
//	(void)offset;
//	(void)fi;
//
//	// check if directory
//	FILE* file = fopen(path, "r");
//	fseek(file, 0, SEEK_END);
//	int fileSize = ftell(file);
//	char* fileInfo = (char*)malloc(fileSize);
//	fseek(file, 0, SEEK_SET);
//	fread(fileInfo, 1, fileSize, file);
//
//	char* isDir = strstr(fileInfo, "filename");
//	if (isDir == NULL)
//		return ENOENT;
//
//	// add to filler
//	filler(buf, ".", NULL, 0);
//	filler(buf, "..", NULL, 0);
//
//	// parse through inode list
//	isDir = strstr(fileInfo, "fusedata.");
//	while (isDir != NULL){
//		isDir = isDir + 9;
//		char inodeNum[5];
//		int count = 0;
//		while (*isDir != ','){
//			inodeNum[count] = *isDir;
//			isDir = isDir + 1;
//			count++;
//		}
//		char name[20] = "fusedata.";
//		strcat(name, inodeNum);
//		filler(buf, name, NULL, 0);
//		isDir = strstr(fileInfo, "fusedata.");
//	}
//
//	return 0;
//}

static int file_open(const char *path, struct fuse_file_info *fi){
	FILE* file = fopen(path, "r+");
	//check if file exists
	if (file == NULL)
		return -ENOENT;
	fclose(file);
	return 0;
}

//static int file_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
////static int file_read(const char *path, char *buf, size_t size, size_t offset, struct fuse_file_info *fi){
//	size_t len = 4096;
//	if (offset < len) {
//		// if offset + size specified is greater than length, read to end of file
//		if (offset + size > len)
//			size = len - offset;
//		FILE* file = fopen(path, "r");
//		fseek(file, offset, 0);
//		fread(buf, 1, size, file); // copy from offset in file
//		fclose(file);
//	}
//	else
//		size = 0;
//	return size;
//}

//static int file_create(const char* path, mode_t mode, struct fuse_file_info* fi){  // returns location of new file
static int create(const char* path){
	// find first free block for inode
	int found = 0;
	char name[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	int count = 1;
	char inodeNum[5];
	while (count < 26 && found == 0){
		char numb[3] = "";
		sprintf(numb, "%i", count);
		strcat(name, numb);
		FILE* file = fopen(name, "r+");
		fscanf(file, "%5[^,],", inodeNum);
		if (inodeNum[0] != '\0'){
			found = 1;
			fseek(file, 0, SEEK_END);
			int fileSize = ftell(file);
			char* fileList = (char*)malloc(fileSize);
			fseek(file, 0, SEEK_SET);
			fread(fileList, 1, fileSize, file);
			int size = strlen(inodeNum) + 1;
			int i = 0;
			while (i < fileSize - size){
				fileList[i] = fileList[i + size];
				i++;
			}
			fileList[fileSize - size] = '\0';
			fclose(file);
			file = fopen(name, "w+");
			fwrite(fileList, 1, fileSize-size, file);
			free(fileList);
			fclose(file);
		}
		else{
			memset(inodeNum, '\0', 5);
			strcpy(name, namecpy);
			fclose(file);
			count++;
		}
	}

	// edit filename_to_inode_dict of parent dir
	FILE* file = fopen(path, "r+");
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	char* dirInfo = (char*)malloc(fileSize + 30);
	fseek(file, 0, SEEK_SET);
	fread(dirInfo, 1, fileSize, file);
	fclose(file);
	// add to filename_to_inode_dict
	char* editDir = dirInfo;
	editDir[fileSize - 1] = '\0';
	editDir[fileSize - 2] = '\0';
	char string[40] = ",f:fusedata.";
	strcat(string, inodeNum);
	strcat(string, ":");
	strcat(string, inodeNum);
	strcat(string, "}}");
	strcat(editDir, string);
	// place editted info in parent dir
	file = fopen(path, "w+");
	fputs(editDir, file);
	fclose(file);
	editDir = NULL;
	free(dirInfo);

	// find another empty block for file
	found = 0;
	char fileLoc[20] = "fusedata.";
	char fileNum[5];
	while (count < 26 && found == 0){
		char numb[3] = "";
		sprintf(numb, "%i", count);
		strcat(fileLoc, numb);
		FILE* file = fopen(fileLoc, "r+");
		fscanf(file, "%5[^,],", fileNum);
		if (fileNum[0] != '\0'){
			found = 1;
			fseek(file, 0, SEEK_END);
			int fileSize = ftell(file);
			char* fileList = (char*)malloc(fileSize);
			fseek(file, 0, SEEK_SET);
			fread(fileList, 1, fileSize, file);
			int size = strlen(fileNum) + 1;
			int i = 0;
			while (i < fileSize - size){
				fileList[i] = fileList[i + size];
				i++;
			}
			fileList[fileSize - size] = '\0';
			fclose(file);
			file = fopen(name, "w+");
			fwrite(fileList, 1, fileSize-size, file);
			free(fileList);
			fclose(file);
		}
		else{
			memset(fileNum, '\0', 5);
			strcpy(name, namecpy);
			fclose(file);
			count++;
		}
	}

	// write inode info to file
	char inodeLoc[20] = "fusedata.";
	strcat(inodeLoc, inodeNum);
	FILE* inode = fopen(inodeLoc, "w");
	int atime = (int)time(NULL);
	int ctime = (int)time(NULL);
	int dtime = (int)time(NULL);
	int fNum = atoi(fileNum);
	fprintf(inode, "{size:0,uid:1,gid:1,mode:33261,linkcount:1,atime:%i,ctime:%i,mtime:%i,indirect:0,location:%i}", atime,
		ctime, dtime, fNum);
	fclose(inode);

	// add to inode array
	int node = atoi(inodeNum);
	inodeArray[numInode] = node;
	numInode++;

	// write something in file to indicate that it is able to be written in
	char theFile[20] = "fusedata.";
	strcat(theFile, fileNum);
	file = fopen(theFile, "w+");
	char permission[13] = "Write in me.";
	fwrite(permission, 1, strlen(permission), file);
	fclose(file);

	return fNum;
}

// deletes fusedata.1 - fusedata.9999. I now know it is not supposed to do that, but I am unsure what it is supposed to do.
//void file_destroy(void* data){
void file_destroy(){
	// delete fusedata.1 - fusedata.9999
	char file[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	char num[5] = "";
	int index = 1;
	while (index < 10000){
		sprintf(num, "%i", index);
		strcat(namecpy, num);
		remove(namecpy);
		strcpy(namecpy, file);
		memset(num, '0', sizeof(num));
		index++;
	}
}

//void* file_init(struct fuse_conn_info *conn){
void file_init(){
	// create empty array and blank files
	FILE* file;
	char name[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	char empty[4096];
	int count = 0;
	while (count < 4096){
		empty[count] = '0';
		count++;
	}
	
	// check if fusedata.0 exists
	file = fopen("fusedata.0", "r");
	int rootExists = 0;
	if (file != NULL){
		count = 1;
		fseek(file, 0, SEEK_END);
		int fileSize = ftell(file);
		char* rootInfo = (char*)malloc(fileSize + 30);
		fseek(file, 0, SEEK_SET);
		fread(rootInfo, 1, fileSize, file);
		fclose(file);
		char* mountCounter = strstr(rootInfo, "mounted:");
		mountCounter = mountCounter + 8;
		mountCounter[0]++;
		mountCounter = strstr(rootInfo, "}");
		mountCounter[1] = '\0';
		file = fopen("fusedata.0", "w");
		fwrite(rootInfo, 1, strlen(rootInfo), file);
		fclose(file);
		mountCounter = NULL;
		free(rootInfo);
		rootExists = 1;
	}
	else
		count = 0;

	char num[5] = "";
	while (count < 10000){
	//while (count < 31){
		sprintf(num, "%i", count);
		strcat(name, num);
		file = fopen(name, "w+");
		fwrite(empty, 1, strlen(empty), file);
		count++;
		memset(num, '0', sizeof(num));
		strcpy(name, namecpy);
		fclose(file);
	}

	// super block info
	if (rootExists == 0){
		super.creationTime = time(NULL);
		super.mounted = 1;
		super.devId = 20;
		super.freeStart = 1;
		super.freeEnd = 25;
		super.root = 26;
		super.maxBlocks = 10000;

		// write into block 0
		FILE* superFile = fopen("fusedata.0", "w");
		int supertime = (int)super.creationTime;
		fprintf(superFile, "{creationTime:%i,mounted:%i,devId:%i,freeStart:%i,freeEnd:%i,root:%i,maxBlocks:%i}", supertime, super.mounted, super.devId,
			super.freeStart, super.freeEnd, super.root, super.maxBlocks);
		fclose(superFile);
	}

	// write into block 1
	int start = 27;
	int end = 399;
	char file1Array[5] = "";
	char numbs[5] = "";
	FILE* file1 = fopen("fusedata.1", "w");
	while (start < end){
		sprintf(numbs, "%i,", start);
		strcpy(file1Array, numbs);
		start++;
		fwrite(file1Array, strlen(file1Array), 1, file1);
	}
	sprintf(numbs, "%i", start);
	strcpy(file1Array, numbs);
	fwrite(file1Array, strlen(file1Array), 1, file1);
	fclose(file1);

	// write into rest of blocks
	int fileCount = 2;
	start = 400;
	end = 799;
	while (fileCount <= 25){
		char fileArray[6] = "";
		char fileName[16] = "fusedata.";
		char numb[6] = "";
		sprintf(numb, "%i", fileCount);
		strcat(fileName, numb);
		FILE* file = fopen(fileName, "w");
		while (start < end){
			sprintf(numb, "%i,", start);
			strcpy(fileArray, numb);
			start++;
			fwrite(fileArray, strlen(fileArray), 1, file);
		}
		sprintf(numb, "%i", start);
		strcpy(fileArray, numb);
		fwrite(fileArray, strlen(fileArray), 1, file);
		fclose(file);
		start = end + 1;
		end = start + 399;
		fileCount++;
		memset(fileArray, '0', sizeof(fileArray));
	}

	// write into root directory
	FILE* directory = fopen("fusedata.26", "w");
	int atime = (int)time(NULL);
	int ctime = (int)time(NULL);
	int dtime = (int)time(NULL);
	fprintf(directory, "{size:0,uid:1,gid:1,mode:16877,atime:%i,ctime:%i,mtime:%i,linkcount:1,filename_to_inode_dict{d:.:26,d:..:26}}", atime, 
		ctime, dtime);
	fclose(directory);

	// add directory to array
	directoryArray[numDir] = 26;
	numDir++;

	return NULL;
}


static int file_link(const char* from, const char* to){
	// check if from is directory or inode
	FILE* file = fopen(from, "r");
	fseek(file, 0, SEEK_END);
	int fromSize = ftell(file);
	char* fromInfo = (char*)malloc(fromSize+20);
	fseek(file, 0, SEEK_SET);
	fread(fromInfo, 1, fromSize, file);
	fclose(file);

	char* isFromDir = strstr(fromInfo, "filename");
	char* isFromInode = strstr(fromInfo, "indirect");
	if (isFromDir == NULL && isFromInode == NULL)
		return ENOENT;

	// add to to from's list
	if (isFromDir != NULL){
		// add to filename_to_inode_dict
		char* editDir = fromInfo;
		editDir[fromSize - 1] = '\0';
		editDir[fromSize - 2] = '\0';
		// check what type of file to is
		FILE* file = fopen(to, "r");
		fseek(file, 0, SEEK_END);
		int toSize = ftell(file);
		char* toInfo = (char*)malloc(toSize);
		fseek(file, 0, SEEK_SET);
		fread(toInfo, 1, toSize, file);
		fclose(file);

		// get numbers at end of to
		char toNum[5];
		int counter = 0;
		int numParse = 9; // start of numbers in name of to
		while (numParse < strlen(to)){
			toNum[counter] = to[numParse];
			counter++;
			numParse++;
		}
		if (counter < 4)
			toNum[counter] = '\0';

		// increment linkcount of to
		char* linkCount = strstr(toInfo, "linkcount:");
		linkCount = linkCount + 10;
		linkCount[0]++;
		linkCount = NULL;

		char* isToDir = strstr(toInfo, "filename");
		char* isToInode = strstr(toInfo, "indirect");

		char string[40] = " ";
		// if to is an inode
		if (isToInode != NULL)
			strcat(string, ",f:fusedata.");
		// if to is dir
		if (isToDir != NULL)
			strcat(string, ",d:fusedata.");
		strcat(string, toNum);
		strcat(string, ":");
		strcat(string, toNum);			
		strcat(string, "}}");
		strcat(editDir, string);
		// place editted info in parent dir
		file = fopen(from, "w+");
		fputs(editDir, file);
		fclose(file);
		editDir = NULL;
		free(toInfo);
	}


	// add to to from's location
	else if (isFromInode != NULL){
		// add to location
		char* editInode = fromInfo;
		editInode[fromSize - 1] = '\0';
		// check what type of file to is
		FILE* file = fopen(to, "r");
		fseek(file, 0, SEEK_END);
		int toSize = ftell(file);
		char* toInfo = (char*)malloc(toSize);
		fseek(file, 0, SEEK_SET);
		fread(toInfo, 1, toSize, file);
		fclose(file);

		char* isToDir = strstr(toInfo, "filename");
		// if to is dir return error. inode cannot point to directory
		if (isToDir != NULL)
			return ENOENT;
		isToDir = NULL;

		// increment linkcount of to
		char* linkCount = strstr(toInfo, "linkcount:");
		linkCount = linkCount + 10;
		linkCount[0]++;
		linkCount = NULL;

		// get numbers at end of to
		char toNum[5];
		int counter = 0;
		int numParse = 10; // start of numbers in name of to
		while (numParse < strlen(to)){
			toNum[counter] = to[numParse];
			counter++;
			numParse++;
		}

		char string[40] = ",";
		strcat(string, toNum);
		strcat(string, "}");
		strcat(editInode, string);
		// place editted info in parent dir
		file = fopen(from, "w+");
		fputs(editInode, file);
		fclose(file);
		editInode = NULL;
		free(toInfo);
	}

	isFromDir = NULL;
	isFromInode = NULL;
	free(fromInfo);

	return 0;
}

//static int file_mkdir(const char* path, mode_t mode){
static int mkdir(const char* path){
	int found = 0;
	char name[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	int count = 1;
	char fileNum[5];
	// find first free block
	while (count < 26 && found == 0){
		char numb[3] = "";
		sprintf(numb, "%i", count);
		strcat(name, numb);
		FILE* file = fopen(name, "r+");
		fscanf(file, "%5[^,],", fileNum);
		if (fileNum[0] != '\0'){
			found = 1;
			fseek(file, 0, SEEK_END);
			int fileSize = ftell(file);
			char* fileList = (char*) malloc(fileSize);
			fseek(file, 0, SEEK_SET);
			fread(fileList, 1, fileSize, file);
			int size = strlen(fileNum)+1;
			int i = 0;
			while (i < fileSize - size){
				fileList[i] = fileList[i + size];
				i++;
			}
			fileList[fileSize - size] = '\0';
			fclose(file);
			file = fopen(name, "w+");
			fwrite(fileList, 1, fileSize-size, file);
			free(fileList);
			fclose(file);
		}
		else{
			strcpy(name, namecpy);
			fclose(file);
		}
	}

	// edit filename_to_inode_dict of parent dir
	FILE* file = fopen(path, "r+");
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	char* dirInfo = (char*)malloc(fileSize+30);
	fseek(file, 0, SEEK_SET);
	fread(dirInfo, 1, fileSize, file);
	fclose(file);
	char* editDir = dirInfo; 
	editDir[fileSize - 1] = '\0';
	editDir[fileSize - 2] = '\0';
	char string[40] = ",d:fusedata.";
	strcat(string, fileNum);
	strcat(string, ":");
	strcat(string, fileNum);
	strcat(string, "}}");
	strcat(editDir, string);
	// place editted info in parent dir
	file = fopen(path, "w+");
	fputs(editDir, file);
	fclose(file);
	editDir = NULL;
	free(dirInfo);

	// write into new directory
	char newDir[20] = "fusedata.";
	strcat(newDir, fileNum);
	FILE* directory = fopen(newDir, "w");
	int atime = (int)time(NULL);
	int ctime = (int)time(NULL);
	int dtime = (int)time(NULL);
	int dirNum = atoi(fileNum);
	fprintf(directory, "{size:0,uid:1,gid:1,mode:16877,atime:%i,ctime:%i,mtime:%i,linkcount:1,filename_to_inode_dict{d:.:%i,d:..:%i}}", atime,
		ctime, dtime, dirNum, dirNum);
	fclose(directory);
	
	// add directory to directory array
	directoryArray[numDir] = dirNum;
	numDir++;

	return 0;
}

static int file_opendir(const char* path, struct fuse_file_info* fi){
	FILE* file = fopen(path, "r+");
	//check if file exists
	if (file == NULL)
		return -ENOENT;
	fclose(file);
	return 0;
}

static int file_release(const char* path, struct fuse_file_info* fi){
//static int file_release(const char* path){
	// check for permission
	char fileName[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	char num[5] = "";
	int index = 0;

	// check to see if file is root/file block list
	while (index < 27){
		sprintf(num, "%i", index);
		strcat(fileName, num);
		if (fileName == path)
			return ENOENT;
		strcpy(fileName, namecpy);
		index++;
	}

	// check to see if file is directory
	int dirCount = 0;
	while (dirCount < numDir){
		int dirNum = directoryArray[dirCount];
		sprintf(num, "%i", dirNum);
		strcat(fileName, num);
		if (fileName == path)
			return ENOENT;
		strcpy(fileName, namecpy);
		dirCount++;
	}

	// check to see if file is inode
	int inodeCount = 0;
	while (inodeCount < numInode){
		int inodeNum = inodeArray[inodeCount];
		sprintf(num, "%i", inodeNum);
		strcat(fileName, num);
		if (fileName == path)
			return ENOENT;
		strcpy(fileName, namecpy);
		inodeCount++;
	}

	// make file eligible for rewriting
	FILE* file = fopen(path, "w+");
	char permission[13] = "Write in me.";
	fwrite(permission, 1, strlen(permission), file);
	fclose(file);
	free(fi);
	return 0;
}

static int file_releasedir(const char* path, struct fuse_file_info* fi){
//static int file_releasedir(const char* path){
	char* num = path+9;
	int directory = atoi(num);

	// check to see if file is directory
	int dirCount = 0;
	int found = 0;
	while (dirCount < numDir && found == 0){
		int dirNum = directoryArray[dirCount];
		if (dirNum == directory)
			found = 1;
		dirCount++;
	}

	if (found == 0)
		return ENOENT;

	// go through directory and release inodes
	FILE* file = fopen(path, "r+");
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	char* dirInfo = (char*)malloc(fileSize + 30);
	fseek(file, 0, SEEK_SET);
	fread(dirInfo, 1, fileSize, file);
	fclose(file);
	char* inodeLoc = strstr(dirInfo, "fusedata.");
	// go through each inode
	while (inodeLoc != NULL){
		inodeLoc = inodeLoc + 9;
		char inode[5];
		int count = 0;
		while (*inodeLoc != ':'){
			inode[count] = *inodeLoc;
			count++;
			inodeLoc = inodeLoc + 1;
		}

		// find file in inode array
		int inodeCount = 0;
		int found = 0;
		while (inodeCount < numInode && found == 0){
			int inodeNum = inodeArray[inodeCount];
			int inodeInt = atoi(inode);
			if (inodeInt == inodeNum)
				found = 1;
			inodeCount++;
		}
		// remove inode from inode array
		int i = inodeCount - 1;
		while (i < numInode){
			inodeArray[i] = inodeArray[i + 1];
			i++;
		}

		// go to the inode and free it and what it points to up
		inode[count] = '\0';
		char fileName[20] = "fusedata.";
		strcat(fileName, inode);
		file = fopen(fileName, "r+");
		fseek(file, 0, SEEK_END);
		int inodeSize = ftell(file);
		char* inodeInfo = (char*)malloc(inodeSize + 30);
		fseek(file, 0, SEEK_SET);
		fread(inodeInfo, 1, inodeSize, file);
		fclose(file);
		char* fileLoc = strstr(inodeInfo, "location");

		// go through each location and free it up
		while (fileLoc != NULL){
			fileLoc = fileLoc + 9;
			char location[5];
			int locCount = 0;
			while (*fileLoc != ',' && *fileLoc != '}'){
				location[locCount] = *fileLoc;
				locCount++;
				fileLoc = fileLoc + 1;
			}
			location[locCount] = '\0';
			// add to free block list
			file = fopen("fusedata.1", "a+");
			char write[6] = ",";
			strcat(write, location);
			//fprintf(file, "%s", write);
			fwrite(write, 1, locCount + 1, file);
			fclose(file);
			// search for comma
			fileLoc = strstr(fileLoc, ",");
			// if comma exists (meaning another location), skip over it
			if (fileLoc != NULL)
				fileLoc = fileLoc + 1;
		}

		free(inodeInfo);
		fileLoc = NULL;

		// add inode to free block list
		file = fopen(fileName, "w");
		char permission[20] = "Write in me.";
		fwrite(permission, 1, strlen(permission), file);
		fclose(file);
		file = fopen("fusedata.1", "a+");
		char write[6] = ",";
		strcat(write, inode);
		//fprintf(file, "%s", write);
		fwrite(write, 1, strlen(write), file);
		fclose(file);

		// look for next inode directory  points too
		inodeLoc = inodeLoc + 1;
		inodeLoc = strstr(inodeLoc, "fusedata.");
	}
	inodeLoc = NULL;
	free(dirInfo);


	// remove directory from directory array
	int i = dirCount - 1; 
	while (i < numDir){
		directoryArray[i] = directoryArray[i + 1];
		i++;
	}

	free(fi);
	return 0;
}

static int file_rename(const char* from, const char* to){
	// if doesn't exist return error
	FILE* file = fopen(from, "r+");
	if (file == NULL){
		fclose(file);
		return -ENOENT;
	}
	fclose(file);

	// attempt to rename it and return result
	return rename(from, to);
}

//static int file_statfs(const char* path, struct statvfs* statbuf){
//	statbuf->f_bsize = 4096;
//	statbuf->f_frsize = 4096;
//	statbuf->f_blocks = 40960000;
//	// count number of free blocks
//	int freeblocks = 0;
//	char name[16] = "fusedata.";
//	char namecpy[16] = "fusedata.";
//	int count = 1;
//	while (count < 26){
//		char numb[3] = "";
//		sprintf(numb, "%i", count);
//		strcat(name, numb);
//		FILE* file = fopen(name, "r");
//		char comma = ',';
//		char search;
//		while (search = fgetc(file)){
//			// if there is a character, that means at least one since no comma after only element
//			if (freeblocks == 0)
//				freeblocks++;
//			if (search == EOF)
//				break;
//			if (search == comma)
//				freeblocks++;
//		}
//		count++;
//		fclose(file);
//		strcpy(name, namecpy);
//	}
//	statbuf->f_bfree = count;
//	statbuf->f_bavail = count;
//	statbuf->f_files = numInode;
//	statbuf->f_ffree = 0;
//	statbuf->f_favail = 0;
//	statbuf->f_fsid = 1;
//	statbuf->f_flag = ST_NOSUID;
//	statbuf->f_namemax = 13;
//
//	return 0;
//}
//
//static int file_unlink(const char* path){
//	FILE* file = fopen(path, "r");
//	fseek(file, 0, SEEK_END);
//	int fileSize = ftell(file);
//	char* fileInfo = (char*)malloc(fileSize + 20);
//	fseek(file, 0, SEEK_SET);
//	fread(fileInfo, 1, fileSize, file);
//	fclose(file);
//
//	char* linkCount = strstr(fileInfo, "linkcount:");
//	if (linkCount == NULL)
//		return ENOENT;
//	// decrement linkcount
//	linkCount = linkCount + 10;
//	linkCount[0]--;
//	// if no more links remain to this file, add to free block list
//	if (linkCount[0] == '0'){
//		char location[5] = " ";
//		int count = 0;
//		int pathCounter = 9;
//		while (pathCounter < strlen(path)){
//			location[count] = path[pathCounter];
//			count++;
//			pathCounter++;
//		}
//		file = fopen("fusedata.1", "a+");
//		char write[6] = ",";
//		strcat(write, location);
//		fwrite(write, 1, count + 1, file);
//		fclose(file);
//	}
//	linkCount = NULL;
//	free(fileInfo);
//
//	// signal path is now empty
//	file = fopen(path, "w+");
//	char permission[13] = "Write in me.";
//	fwrite(permission, 1, strlen(permission), file);
//	fclose(file);
//
//	return 0;
//}

//static int file_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi){
static int file_write(const char* path, const char* buf, size_t size, size_t offset){
	// check for permission
	char fileName[16] = "fusedata.";
	char namecpy[16] = "fusedata.";
	char num[5] = "";
	int index = 0;

	// check to see if file is root/file block list
	while (index < 27){
		sprintf(num, "%i", index);
		strcat(fileName, num);
		if (fileName == path)
			return ENOENT;
		strcpy(fileName, namecpy);
		index++;
	}

	// check to see if file is directory
	int dirCount = 0;
	while (dirCount < numDir){
		int dirNum = directoryArray[dirCount];
		sprintf(num, "%i", dirNum);
		strcat(fileName, num);
		if (fileName == path)
			return ENOENT;
		strcpy(fileName, namecpy);
		dirCount++;
	}


	// check if null (meaning inode hasn't been created for file yet)
	FILE* file = fopen(path, "r+");
	char buffer[5];
	fread(buffer, 1, 3, file);
	fclose(file);
	if (buffer[0] == '0' && buffer[1] == '0')
		return ENOENT;
	
	// otherwise inode has been created for file already
	int len = 4096;
	int overflow = 0;
	int fit = 0;
	// check if offset and size being written is less than length of file
	if (offset < len && offset + size < len) {
		// write to file at offset position
		file = fopen(path, "r+");
		fseek(file, offset, 0);
		// if what want to write doesn't fit
		if (offset + size > len){
			// write what fits
			overflow = (offset + size) - len;
			fit = len - offset;
			fwrite(buf, 1, fit, file);
			fclose(file);
			buf = buf + fit; // move past whatever was written

			// find first free block for file
			int found = 0;
			char name[16] = "fusedata.";
			int count = 1;
			char fileLoc[5];
			while (count < 26 && found == 0){
				char numb[3] = "";
				sprintf(numb, "%i", count);
				strcat(name, numb);
				file = fopen(name, "r+");
				fscanf(file, "%5[^,],", fileLoc);
				if (fileLoc[0] != '\0'){
					found = 1;
					fseek(file, 0, SEEK_END);
					int fileSize = ftell(file);
					char* fileList = (char*)malloc(fileSize);
					fseek(file, 0, SEEK_SET);
					fread(fileList, 1, fileSize, file);
					int fsize = strlen(fileLoc) + 1;
					int i = 0;
					while (i < fileSize - fsize){
						fileList[i] = fileList[i + fsize];
						i++;
					}
					fileList[fileSize - fsize] = '\0';
					fclose(file);
					file = fopen(name, "w+");
					fwrite(fileList, 1, fileSize-fsize, file);
					free(fileList);
					fclose(file);
				}
				else{
					memset(fileLoc, '\0', 5);
					strcpy(name, namecpy);
					fclose(file);
					count++;
				}
			}

			// need to assign another file to inode
			int inode = 0;
			found = 0;
			char inodeName[20] = "fusedata.";
			char namecpy[20] = "fusedata.";
			char num[5] = "";
			int fileNum = atoi(path + 9);
			char fileNumber[5];
			sprintf(fileNum, "%i", fileNum);
			// look through list of directories and see which contains file
			while (inode < numInode && found == 0){
				sprintf(num, "%i", inodeArray[inode]);
				strcat(inodeName, num);
				file = fopen(inodeName, "r+");
				fseek(file, 0, SEEK_END);
				int fileSize = ftell(file);
				char* inodeInfo = (char*)malloc(fileSize + 30);
				fseek(file, 0, SEEK_SET);
				fread(inodeInfo, 1, fileSize, file);
				fclose(file);
				char* pointer = strstr(inodeInfo, fileNumber);
				if (pointer != NULL){ // file found
					pointer = NULL;
					found = 1;
					// add to location
					char* editInode = inodeInfo;
					editInode[fileSize - 1] = '\0';
					char end[40] = ",";
					strcat(end, fileLoc);
					strcat(end, "}");
					strcat(editInode, end);

					// place editted info in inode
					file = fopen(inodeName, "w+");
					fputs(editInode, file);
					fclose(file);
					free(inodeInfo);
					editInode = NULL;

					// now to change size and indirect. first read all data from file
					file = fopen(inodeName, "r+");
					char csize[20];
					char uid[20];
					char gid[20];
					char mode[20];
					char atime[20];
					char ctime[20];
					char mtime[20];
					char linkcount[20];
					char indirect[20];
					char location[5000];
					char string[200];
					fscanf(file, "%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%5000[^}]}",
						string, csize, string, uid, string, gid, string, mode, string, linkcount, string, atime, string, ctime, string, mtime, 
						string, indirect, location);
					fclose(file);

					// put data back into file
					int isize = 4096 + overflow;
					int iuid = atoi(uid);
					int igid = atoi(gid);
					int imode = atoi(mode);
					int iatime = atoi(atime);
					int ictime = atoi(ctime);
					int imtime = atoi(mtime);
					int ilinkcount = atoi(linkcount);
					int iindirect = atoi(indirect);
					if (iindirect == 0)
						iindirect++;
					file = fopen(inodeName, "w+");
					fprintf(file, "size:%i,uid:%i,gid:%i,mode:%i,linkcount:%i,atime:%i,ctime:%i,mtime:%i,indirect:%i,location:%s", isize, iuid, igid,
						imode, iatime, ictime, imtime, ilinkcount, iindirect, location);
					fclose(file);
				}
				else{
					strcpy(inodeName, namecpy);
				}
			}

			// write what wasn't written to new file
			char fileName[20] = "fusedata.";
			strcat(fileName, fileLoc);
			file = fopen(fileLoc, "w+");
			fwrite(buf, 1, overflow, file);
			fclose(file);
		}

		// write it to file
		else{
			fwrite(buf, 1, size, file);
			fclose(file);

			// change size in inode. first find inode path is in
			int inode = 0;
			int found = 0;
			char inodeName[20] = "fusedata.";
			char namecpy[20] = "fusedata.";
			char num[5] = "";
			// look through list of directories and see which contains file
			while (inode < numInode && found == 0){
				int node = inodeArray[inode];
				sprintf(num, "%i", node);
				strcat(inodeName, num);
				file = fopen(inodeName, "r+");
				fseek(file, 0, SEEK_END);
				int fileSize = ftell(file);
				char* inodeInfo = (char*)malloc(fileSize + 30);
				fseek(file, 0, SEEK_SET);
				fread(inodeInfo, 1, fileSize, file);
				fclose(file);
				int fileNum = atoi(path + 9);
				char fileNumber[5];
				sprintf(fileNumber, "%i", fileNum);
				char* pointer = strstr(inodeInfo, fileNumber);
				if (pointer != NULL){ // file found
					pointer = NULL;
					found = 1;

					// now to change size. first read all data from file
					file = fopen(inodeName, "r+");
					char csize[20];
					char uid[20];
					char gid[20];
					char mode[20];
					char atime[20];
					char ctime[20];
					char mtime[20];
					char linkcount[20];
					char indirect[20];
					char location[5000];
					char string[200];
					fscanf(file, "%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,],%20[^:]:%20[^,]%5000[^}]}",
						string, csize, string, uid, string, gid, string, mode, string, linkcount, string, atime, string, ctime, string, mtime,
						string, indirect, location);
					fclose(file);

					// put data back into file
					int isize = offset + size;
					int iuid = atoi(uid);
					int igid = atoi(gid);
					int imode = atoi(mode);
					int iatime = atoi(atime);
					int ictime = atoi(ctime);
					int imtime = atoi(mtime);
					int ilinkcount = atoi(linkcount);
					int iindirect = atoi(indirect);
					file = fopen(inodeName, "w+");
					fprintf(file, "{size:%i,uid:%i,gid:%i,mode:%i,linkcount:%i,atime:%i,ctime:%i,mtime:%i,indirect:%i%s}", isize, iuid, igid,
						imode, ilinkcount, iatime, ictime, imtime, iindirect, location);
					fclose(file);
				}
				else{
					inode++;
					strcpy(inodeName, namecpy);
				}
			}
		}
	}

	// offset is greater than file length, return error
	else
		return ENOENT;

	return size;
}

//static struct fuse_operations file_oper = {
//	.create = file_create,
//	.destroy = file_destroy,
//	//.getattr = file_getattr,
//	.init = file_init,
//	.link = file_link,
//	.mkdir = file_mkdir,
//	.open = file_open,
//	.opendir = file_opendir,
//	.read = file_read,
//	.readdir = file_readdir,
//	.release = file_release,
//	.releasedir = file_releasedir,
//	.rename = file_rename,
//	.statfs = file_statfs,
//	.unlink = file_unlink,
//	.write = file_write,
//};

int main(int argc, char *argv[]){
	//return fuse_main(argc, argv, &file_oper, NULL);
	file_init();
	mkdir("fusedata.26");
	create("fusedata.27");
	//getattr("fusedata.28");
	//file_write("fusedata.29", "asdfgsfbhfgdsafghfdsafgthgfdfghgf", 33, 0);
	create("fusedata.27");
	/*file_write("fusedata.31", "HELLO WORLD", 12, 0);
	file_release("fusedata.31");
	file_releasedir("fusedata.27");
	file_link("fusedata.26", "fusedata.28");
	file_unlink("fusedata.28");
	file_unlink("fusedata.28");*/
	file_destroy();
}