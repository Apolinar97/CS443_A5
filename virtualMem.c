#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#define FRAME_SIZE 256
#define TOTAL_FRAMES 256
#define ADDRESS_MASK 0xFFFF
#define OFFSET_MASK 0xFF
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256

#define BUFFER 10
#define READ_CHUNK 256

//Page tables
int pageTableFrames[PAGE_TABLE_SIZE];
int pageTableNums[PAGE_TABLE_SIZE];
signed char val; //contains bit value.

//TLB tables
int TLBFrameNum[TLB_SIZE];
int TLBPageNum[TLB_SIZE];

//Physical mem
int physicalMemory[TOTAL_FRAMES][FRAME_SIZE];

int pageFault = 0;
int TLBHits = 0;

int firstAvaFrame = 0;
int firstAvaPageTableNum = 0;
int numOfTLBEntries = 0;

char address[BUFFER];
int logicalAddress;

signed char buffer[READ_CHUNK];//used for reading backing-buffer.

//files
FILE *addressFile;
FILE *backing;

//function prototypes
void getPage(int logical_address);
void readFromStore(int pageNum);
void insertIntoTLB(int pageNum,int frameNum);
void printStats(int translatedNum);
//--------------------------------------------
int main(int argc, char* argv[]) {
  //algo outline:
  //Read files, Call getPage.
  //getPage inserts into TLB and prints out req. vals.
  //printStats()

  //Note that error cases must be checked for reading in files. 
  // addressFile = fopen("addresses.txt","r");

  int numOfTranslatedAddys = 0;
  
  addressFile = fopen(argv[1],"r");//allows user to input file of choice into cmd.

  backing = fopen("BACKING_STORE.bin","rb");

  if( (addressFile == NULL) ||(backing == NULL)) {
    printf("Error in opening files.\n");
    return -1;
  }


  //while readying file call getPage and increment numofTrnasaddys

  while(fgets(address, BUFFER, addressFile) != NULL) {
    logicalAddress = atoi(address);
    getPage(logicalAddress);

    numOfTranslatedAddys++;
    
  }
  printStats(numOfTranslatedAddys);
  
  
  return 0;
}




//----------------------------------------------
void readFromStore(int pageNum) {
  if(fseek( backing, pageNum * READ_CHUNK, SEEK_SET) != 0) {
    fprintf(stderr, "Error seeking in backing store\n");
    printf("Error in seeking backing store.\n");

  }

  if(fread(buffer, sizeof(signed char), READ_CHUNK, backing) == 0) {
    fprintf(stderr,"Error in reading backing store\n");
    printf("Error in reading backig store\n");

  }
  //base case: load bits to firstava.

  for(int i=0; i<READ_CHUNK; i++) {
    physicalMemory[firstAvaFrame][i] = buffer[i];

  }

  pageTableNums[firstAvaPageTableNum] = pageNum;
  pageTableFrames[firstAvaPageTableNum] = firstAvaFrame;

  firstAvaFrame++;
  firstAvaPageTableNum++;
  return;

}
//---------------------------------------------------------------------------


void getPage(int logical_address) {
  int pageNum = (( logical_address & ADDRESS_MASK) >> 8);
  int offset = (logical_address & OFFSET_MASK);

  int frameNum = -1; //-1 if it has no value.

  int i;
  for(i=0; i<TLB_SIZE; i++) {
    if(TLBPageNum[i] == pageNum) {
      frameNum = TLBFrameNum[i];
      TLBHits++;
    }//end if
    
  }//end for

  if(frameNum == -1) {
    int i;
    for(i=0;i < firstAvaPageTableNum; i++) {
      if(pageTableNums[i] == pageNum) {
	frameNum = pageTableFrames[i];
      }//end inner if

    }//end for

    if(frameNum == -1) {
      readFromStore(pageNum);
      pageFault++;
      frameNum = firstAvaFrame - 1;
    }//fault has occured

    
  }//end outter if

  insertIntoTLB(pageNum,frameNum);
  val = physicalMemory[frameNum][offset];
  printf("offset: %d\n",offset);

  printf("---------------------------\n");

  printf("Frame number: %d\n",frameNum);

  printf("---------------------------\n");
  
  printf("Virtual address: %d , Physical address: %d, Value: %d \n", logicalAddress,
	 (frameNum << 8) | offset,val); 
  


  
  
  
  
  
  insertIntoTLB(pageNum,frameNum);
}//end function
//-----------------------------------------------------------------

void insertIntoTLB(int pageNum,int frameNum) {
  int i;

  for(i=0; i<numOfTLBEntries;i++) {
    if(TLBPageNum[i] == pageNum)
      break;
  }//end for. 

  if( i == numOfTLBEntries ) {
    
    if(numOfTLBEntries < TLB_SIZE) { //table still has room

      TLBPageNum[numOfTLBEntries] = pageNum;
      TLBFrameNum[numOfTLBEntries] = frameNum;
      
    }

    else {
      for(i=0; i<TLB_SIZE -1; i++) {
	TLBPageNum[i] = TLBPageNum[i+1];
	TLBFrameNum[i] = TLBFrameNum[i+1];
      }//end for
      TLBPageNum[numOfTLBEntries-1] = pageNum;
      TLBFrameNum[numOfTLBEntries -1] = frameNum;

    }//no room, have to move things over. 
  }

  else {
    for( i=i; i<numOfTLBEntries - 1; i++) {
      TLBPageNum[i] = TLBPageNum[i+1];
      TLBFrameNum[i] = TLBFrameNum[i+1];
    }
    if(numOfTLBEntries < TLB_SIZE) {
      TLBPageNum[numOfTLBEntries] = pageNum;
      TLBFrameNum[numOfTLBEntries] = frameNum;
      
    }

    else {
      TLBPageNum[numOfTLBEntries-1] = pageNum;
      TLBFrameNum[numOfTLBEntries-1] = frameNum;
    }

    if(numOfTLBEntries < TLB_SIZE)
      numOfTLBEntries++;
    
  }
  
  
}

void printStats(int translatedNum) {
  double pageFaultRate = pageFault/(double)translatedNum;
  double tableHitRate = TLBHits/(double)translatedNum;


  printf("Stats:\n");
  printf("=================================================\n");
	
  printf("/tPage-fault-rate = %f\n",pageFaultRate);
  printf("/tTLB hit rate = %f\n",tableHitRate);

  printf("=================================================\n");
  
}


