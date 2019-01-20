#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct block{
	
	unsigned long long int tag; //tag holder 
	int valid; 
	long long int time; //for FIFO and LRU
	
	long int misses;
	long int hits;	

	long int reads;
	long int writes;

};	

struct block AssocW(int assocN,long int index,struct block temp,unsigned long long int tag,struct block** cache,long int counter);

struct block AssocR(int assocN,long int index,struct block temp, unsigned long long int tag,struct block** cache,long int counter);

struct block AssocWPre(unsigned long long int tag2,int assocN, long int index, struct block temp, unsigned long long int tag,struct block** cache,long int counter,long int index2,long int counter2,struct block temp2,int setNum);

struct block AssocRPre(unsigned long long int tag2,int assocN, long int index, struct block temp, unsigned long long int tag,struct block** cache,long int counter,long int index2,long int counter2,struct block temp2,int setNum);

int blockOffset(int bSize);
int indexOffset(int setNum);

unsigned long long int tagBuild(unsigned long long int address, int totalOff);
int fifo(struct block** cache, long int index,int assocN);

int findOpen(struct block** cache, long int index, int assocN);
int powerOfTwo(int num);

long long int count = 0;
long long int count2 = 0;

int lru = 0;

int main(int argc, char** argv){

	if(argc != 6){
		printf("error\n");
		return 0;
	}

	FILE *fp = NULL;

	int i;
	char* cacheSize = argv[1];
	int cSize = atoi(cacheSize);

	char* blockSize = argv[4];
	int bSize = atoi(blockSize);

	if(bSize > cSize){
		printf("error\n");
		return 0;
	}

	int check = powerOfTwo(cSize); //checks if cacheSize is a power of 2

	if(check == 0 || check == 2 || check == 3){ //0 if it an odd number, 2 if it is zero, 3 if it is one
		printf("error\n");
		return 0;
	}
	check = powerOfTwo(bSize); //check if blockSize is a power of 2

	if(check == 0 || check == 2){
		printf("error\n");
		return 0;
	}
	
	unsigned int pc; //useless
	char colon; //useless

	char rw;
	unsigned long long int address;
	unsigned long long int address2;
	
	int assocN;
	char policy;
	char assoc;

	unsigned long long int tag;
	unsigned long long int tag2;
	
	assoc = argv[2][0]; //stores associativity

	if(assoc != 'a' && assoc != 'd'){
		printf("error\n");
		return 0;
	}
	
	if(strcmp(argv[3],"fifo") == 0){
		policy = 'f';
	}else if(strcmp(argv[3],"lru") == 0){
		policy = 'l';
	}else{
		printf("error\n");
		return 0;
	}		

		if(policy == 'l'){//LRU implementation
			lru = 1;
		}

		if(assoc == 'a'){

			char* num;
			num = argv[2];

			memmove(&num[0], &num[0+6],strlen(num));
			assocN = atoi(num);

			int pow = powerOfTwo(assocN);

			if(pow == 0){
				printf("error\n");
				return 0;
			}

			if(assocN != 0){//N-WAY ASSOC

				int blockNum = cSize/bSize;
				int setNum = blockNum/assocN;

				if(assocN > blockNum){
					printf("error\n");
					return 0;
				}
	
				int blockOff = blockOffset(bSize); //how many bits dedicated to blockoffset
				int indexOff = indexOffset(setNum);//how many bits dedicated to index offset
				int totalOff = blockOff + indexOff;//total offset bits for the tag

				struct block temp;

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;			

				struct block** cache; //allocate cache
					cache = (struct block**)malloc(setNum*sizeof(struct block*));
						
					for(i = 0; i < setNum; i++){
						cache[i] = (struct block*)malloc(assocN*sizeof(struct block));
					}

				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}
	
				long int counter = 1;

				printf("no-prefetch\n");
				
				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){
					if(rw == 'W'){
						
						long int index = address >> blockOff;
						index = index%setNum;
						
						tag = tagBuild(address,totalOff); //builds the tag for the block

						temp = AssocW(assocN,index,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
						
						//counter++;
						
					}else if(rw == 'R'){

						long int index = address >> blockOff;
						index = index%setNum;
						
						tag = tagBuild(address,totalOff); //builds the tag for the block

						temp = AssocR(assocN,index,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						//counter++;	
					}
				}
			
				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);
				fclose(fp);

//============================================ASSOC:N WITH PREFETCH====================================	

				setNum = blockNum/assocN;

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;	
				temp.tag = 0;
				
				struct block temp2;
				
				temp2.hits = 0;
				temp2.misses = 0;
				temp2.reads = 0;
				temp2.writes = 0;
				temp2.valid = 0;
				temp2.time = 0;	
				temp2.tag = 0;	
				
				int j;
				for(i = 0; i < setNum; i++){
					for(j = 0; j < assocN; j++){
						cache[i][j] = temp;
					}
				}
				
				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}

				printf("with-prefetch\n");
				
				long int counter2 = 2;

				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){

					if(rw == 'W'){

						address2 = address + bSize;

						long int index = address >> blockOff;
						index = index%setNum;

						long int index2 = address2 >> blockOff;
						index2 = index2%setNum;

						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);//builds tag for second address;
						
						temp = AssocWPre(tag2,assocN,index,temp,tag,cache,counter,index2,counter2,temp2,setNum);
					
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						
					}else if(rw == 'R'){
					
						address2 = address + bSize;

						long int index = address >> blockOff;
						index = index%setNum;

						long int index2 = address2 >> blockOff;
						index2 = index2%setNum;

						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);

						temp = AssocRPre(tag2,assocN,index,temp,tag,cache,counter,index2,counter2,temp2,setNum);
						
						//pass through the index, the temp block structure, the tag and the actual 2D cache
				
					}
				}

				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);


				
			}else{ //=============================FULLY ASSOC
	
				int j;
			
				int blockNum = cSize/bSize;
				
				assocN = blockNum; 

				struct block temp;

				if(assocN > blockNum){
					printf("error\n");
					return 0;
				}

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;	
							

				struct block** cache; //allocate cache set---------->all blocks
					cache = (struct block**)malloc(1*sizeof(struct block*));
						
					for(i = 0; i < 1; i++){
						cache[i] = (struct block*)malloc(assocN*sizeof(struct block));
					}

				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}

				long int counter = 1;
				
				printf("no-prefetch\n");				

				int blockOff = blockOffset(bSize); //how many bits dedicated to blockoffset
				int indexOff = 0;//how many bits dedicated to index offset
				int totalOff = blockOff + indexOff;//total offset bits for the tag
			
				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){

					if(rw == 'W'){
						
						tag = tagBuild(address,totalOff); //builds the tag for the block
					
						temp = AssocW(assocN,0,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
						
						//counter++;
				
					}else if(rw == 'R'){

						tag = tagBuild(address,totalOff); //builds the tag for the block
						//printf("tag: %lx\n",tag);

						temp = AssocR(assocN,0,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						//counter++;
						
					}
				
				}
		
				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);
				
				fclose(fp);

//===============================================FULLY ASSOC WITH PREFETCH========================================================
				
				blockNum = cSize/bSize;

				int setNum = 1;
		
				assocN = blockNum; 

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;	
				temp.tag = 0;
				
				struct block temp2;
				
				temp2.hits = 0;
				temp2.misses = 0;
				temp2.reads = 0;
				temp2.writes = 0;
				temp2.valid = 0;
				temp2.time = 0;	
				temp2.tag = 0;	

				for(i = 0; i < setNum; i++){
					for(j = 0; j < assocN; j++){
						cache[i][j] = temp;
					}
				}

				long int counter2 = 0;

				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}

				printf("with-prefetch\n");

				blockOff = blockOffset(bSize); //how many bits dedicated to blockoffset
				indexOff = 0;//how many bits dedicated to index offset
				totalOff = blockOff + indexOff;//total offset bits for the tag
				
				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){


					if(rw == 'W'){

						address2 = address + bSize;
						
						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);//builds tag for second address;
						
						temp = AssocWPre(tag2,assocN,0,temp,tag,cache,counter,0,counter2,temp2,setNum);
						
						//pass through the index, the temp block structure, the tag and the actual 2D cache
						
					}else if(rw == 'R'){
			
						address2 = address + bSize;

						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);
						
						temp = AssocRPre(tag2,assocN,0,temp,tag,cache,counter,0,counter2,temp2,setNum);
						
						//pass through the index, the temp block structure, the tag and the actual 2D cache	
						
					}
				}

				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);
				fclose(fp);
			}
	

		}else if(assoc == 'd'){ //DIRECTED MAPPING
			
				int blockNum = cSize/bSize;
				int setNum = blockNum; //number of sets is the number of blocks

				assocN = 1; //one block in each set 

				if(assocN > blockNum){
					printf("error\n");
					return 0;
				}

				struct block temp;

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;			

				struct block** cache; //allocate cache set---------->all blocks
					cache = (struct block**)malloc(setNum*sizeof(struct block*));
						
					for(i = 0; i < setNum; i++){
						cache[i] = (struct block*)malloc(assocN*sizeof(struct block));
					}

				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}

				printf("no-prefetch\n");

				int counter = 1;
				
				int blockOff = blockOffset(bSize); //how many bits dedicated to blockoffset
				int indexOff = indexOffset(setNum);//how many bits dedicated to index offset
				int totalOff = blockOff + indexOff;//total offset bits for the tag
				
				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){
				
					if(rw == 'W'){

						long int index = address >> blockOff;
						index = index%setNum;						
							
						tag = tagBuild(address,totalOff); //builds the tag for the block
					
						temp = AssocW(assocN,index,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
						
						//counter++;
				
					}else if(rw == 'R'){
			
						long int index = address >> blockOff;
						index = index%setNum;

						tag = tagBuild(address,totalOff); //builds the tag for the block

						temp = AssocR(assocN,index,temp,tag,cache,counter);
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						//counter++;
						
					}
				}

				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);
				fclose(fp);

//=======================================DIRECTED MAPPING WITH PREFETCH==========================================
				
		
				assocN = 1; 
				setNum = blockNum;

				temp.hits = 0;
				temp.misses = 0;
				temp.reads = 0;
				temp.writes = 0;
				temp.valid = 0;
				temp.time = 0;	
				temp.tag = 0;
				
				struct block temp2;
				
				temp2.hits = 0;
				temp2.misses = 0;
				temp2.reads = 0;
				temp2.writes = 0;
				temp2.valid = 0;
				temp2.time = 0;	
				temp2.tag = 0;	
				
				int j;
				for(i = 0; i < setNum; i++){
					for(j = 0; j < assocN; j++){
						cache[i][j] = temp;
					}
				}
				
				fp = fopen(argv[5],"r");

				if(fp == NULL){
					printf("error\n");
					return 0;
				}

				printf("with-prefetch\n");

				counter = 1;
				long int counter2 = 2;

				blockOff = blockOffset(bSize); //how many bits dedicated to blockoffset
				indexOff = 0;//how many bits dedicated to index offset
				totalOff = blockOff + indexOff;//total offset bits for the tag
				
				while(fscanf(fp,"%x %c %c %llx", &pc, &colon, &rw, &address) == 4){

					if(rw == 'W'){

						address2 = address + bSize;

						long int index = address >> blockOff;
						index = index%setNum;

						long int index2 = address2 >> blockOff;
						index2 = index2%setNum;

						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);//builds tag for second address;
						
						temp = AssocWPre(tag2,assocN,index,temp,tag,cache,counter,index2,counter2,temp2,setNum);
					
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						
					}else if(rw == 'R'){
					
						address2 = address + bSize;

						long int index = address >> blockOff;
						index = index%setNum;

						long int index2 = address2 >> blockOff;
						index2 = index2%setNum;

						tag = tagBuild(address,totalOff); //builds the tag for the block
						tag2 = tagBuild(address2,totalOff);

						temp = AssocRPre(tag2,assocN,index,temp,tag,cache,counter,index2,counter2,temp2,setNum);
						
						//pass through the index, the temp block structure, the tag and the actual 2D cache
					
						
					}
				}

				printf("Memory reads: %ld\n",temp.reads); 
				printf("Memory writes: %ld\n",temp.writes); 
				printf("Cache hits: %ld\n", temp.hits);
				printf("Cache misses: %ld\n",temp.misses);
				fclose(fp);

	
		}

return 0;
}

//========================================METHODS===================================================

int blockOffset(int bSize){ //finds the total number of bits dedicated to the block Offset

	int tempB = bSize;
	int bcount = 0;

	while(tempB != 1){
	
		tempB = tempB/2;
		bcount = bcount + 1;

	}

	return bcount;
}

int indexOffset(int setNum){ //finds the total number of bits dedicated to the index

	int tempS = setNum;
	int Scount = 0;
	
	if(setNum == 1){ //for fully assoc
		return Scount;
	}
	
	while(tempS != 1){
	
		tempS = tempS/2;
		Scount = Scount + 1;

	}

	return Scount;
}
unsigned long long int tagBuild(unsigned long long int address,int totalOff){ //builds the tag for every address in text file

	unsigned long long int temp = address >> totalOff; 

	return temp;

}

struct block AssocW(int assocN, long int index, struct block temp, unsigned long long int tag,struct block** cache,long int counter){
	
	temp.tag = tag;
	temp.valid = 1;
	//temp.time = counter;
	int j = 0;
	int validCt = 0;
	
	while(j < assocN){

		if(cache[index][j].tag == temp.tag){

			if(lru == 1){
				count2++;
				temp.time = count2;
				
			}
	
			temp.hits++;
			temp.writes++;
			return temp;
		}
		if(cache[index][j].valid == 1){
			validCt++;
		}
		
		if(cache[index][j].valid == 0){//set the temp block to the location in the cache

			count2++;
			temp.time = count2;
			cache[index][j] = temp;

			temp.reads++;
			temp.misses++;
			temp.writes++;
			break;
	}

		j++;
	}
	
	if(validCt == assocN){ //set is full we need to evict

		int evict = fifo(cache,index,assocN);

		count2++;
		temp.time = count2;
		cache[index][evict] = temp; //replace old block

		temp.misses++;
		temp.writes++;
		temp.reads++;
	}

	return temp;
}

struct block AssocWPre(unsigned long long int tag2,int assocN, long int index, struct block temp, unsigned long long int tag,struct     block** cache,long int counter,long int index2,long int counter2,struct block temp2,int setNum){
	
	temp.tag = tag;
	temp.valid = 1;
	
	temp2.tag = tag2;
	temp2.valid = 1;
	
	int validCt = 0;
	int present = 0;
	
	int j;

	j = 0;
	
	while(j < assocN){

		if(cache[index][j].tag == temp.tag){

			if(lru == 1){
				count++;
				temp.time =count;
			}	
 
			temp.hits++; //increment hits
			temp.writes++;
			return temp; //done
		}


		if(cache[index][j].valid == 1){ //checks if the cache is full
			validCt++;
			
		}
		
		if(cache[index][j].valid == 0){ //found an empty place for the block and its prefetched
			
			count++;
			temp.time = count;
			cache[index][j] = temp;

			temp.reads++;
			temp.writes++;
			temp.misses++;
			
			int k = 0;
			while(k < assocN){ //searchs for prefetched 
				if(cache[index2][k].tag == temp2.tag){
					present = 1;
					break;
				}

				k++;
			}

			if(present == 0){

				count++;
				temp2.time = count;

			int location = findOpen(cache,index2,assocN);

			if(location != 0){
				cache[index2][location] = temp2;

				temp.reads++;
				return temp;
			}else{
				int evict = fifo(cache,index2,assocN);
				cache[index2][evict] = temp2;
				
				temp.reads++;
				return temp;

			}
		}
			return temp;
		
	}

		j++;
}
	
	if(validCt == assocN){ //full set so it calls replacement method

		int evict = fifo(cache,index,assocN); //make room for just one eviction
			count++;
			temp.time = count;
			cache[index][evict] = temp;
		
			temp.reads++;
			temp.misses++;
			temp.writes++;

		int k = 0;
			while(k < assocN){ //searchs for prefetched 
				if(cache[index2][k].tag == temp2.tag){
					present = 1;//prefetched block is in the cache
					break;
				}

				k++;
			}

		if(present == 0){ //prefetched block is not yet in the cache

			evict = fifo(cache,index2,assocN); //makes space for the prefetched address
			count++;
			temp2.time = count;
			cache[index2][evict] = temp2;

			temp.reads++;

		} 
			
	}

	return temp;
}
struct block AssocR(int assocN, long int index, struct block temp, unsigned long long int tag,struct block** cache,long int counter){

	temp.tag = tag;
	temp.valid = 1;
	//temp.time = counter;
	
	int j = 0;
	int validCt = 0;
	
	while(j < assocN){

		if(cache[index][j].tag == temp.tag){
			
			if(lru == 1){
				count2++;
				temp.time = count2;
				
			}
			
			temp.hits++;
			return temp;
		}
		if(cache[index][j].valid == 1){
			validCt++;
		}
		
		if(cache[index][j].valid == 0){//set the temp block to the location in the cache

			count2++;
			temp.time = count2;
			cache[index][j] = temp;

			temp.misses++;
			temp.reads++;
			break;
		}

		j++;
	}
	
	if(validCt == assocN){

		int evict = fifo(cache,index,assocN);
		count2++;
		temp.time = count2;
		cache[index][evict] = temp; //replace old block

		temp.misses++;
		temp.reads++;
	}

	return temp;
}

struct block AssocRPre(unsigned long long int tag2,int assocN, long int index, struct block temp, unsigned long long int tag,struct block** cache,long int counter,long int index2,long int counter2,struct block temp2,int setNum){

	temp.tag = tag;
	temp.valid = 1;
	
	temp2.tag = tag2;
	temp2.valid = 1;
	
	int validCt = 0;
	int present = 0;
	
	int j;

	j = 0;
	
	while(j < assocN){

		if(cache[index][j].tag == temp.tag){ 

			if(lru == 1){
				count++;
				temp.time =count;
			}
			
			temp.hits++; //increment hits
			return temp; //done
		}


		if(cache[index][j].valid == 1){ //checks if the cache is full
			validCt++;
			
		}
		
		if(cache[index][j].valid == 0){ //found an empty place for the block and its prefetched
			
			count++;
			temp.time = count;
			cache[index][j] = temp;

			temp.reads++;
			temp.misses++;

			int k = 0;
			while(k < assocN){ //searchs for prefetched 
				if(cache[index2][k].tag == temp2.tag){
					present = 1;
					break;
				}

				k++;
			}

			if(present == 0){

				count++;
				temp2.time = count;

			int location = findOpen(cache,index2,assocN);

			if(location != 0){
				cache[index2][location] = temp2;
				
				temp.reads++;
				return temp;
			}else{
				int evict = fifo(cache,index2,assocN);
				cache[index2][evict] = temp2;
				
				
				temp.reads++;
				return temp;

			}
		}
			return temp;
		
	}

		j++;
}
	
	if(validCt == assocN){ //full set so it calls replacement method

		int evict = fifo(cache,index,assocN); //make room for just one eviction
			count++;
			temp.time = count;
			cache[index][evict] = temp;
		
			temp.reads++;
			temp.misses++;

		int k = 0;
			while(k < assocN){ //searchs for prefetched 
				if(cache[index2][k].tag == temp2.tag){
					present = 1;
					break;
				}

				k++;
			}

		if(present == 0){ //prefetched block is not yet in the cache

			evict = fifo(cache,index2,assocN); //makes space for the prefetched address
			count++;
			temp2.time = count;
			cache[index2][evict] = temp2;

			temp.reads++;

		} //prefetched block is in the cache
			
	}

	return temp;
}

int fifo(struct block** cache, long int index, int assocN){
	
	int evictLocation = 0;
	int ct= 1;
	long int hold = cache[index][0].time;

	while(ct < assocN){
	
	if(cache[index][ct].time < hold){ 
		evictLocation = ct;
		break;
	}
		
		ct++;
	}

	return evictLocation;
}

int findOpen(struct block** cache, long int index, int assocN){ //a method to find an open spot in a cache line

int j = 0;
int location = 0;

	while(j < assocN){

		if(cache[index][j].valid == 0){
			location = j;
			break;
		}
		j++;
	}
	
	return location; //if location is zero, the set was full
}

int powerOfTwo(int num){ //finds out if the integer is a power of two

	int power = 1;

	if(num == 0){ //assoc will return 0
		power = 2;
		return power;
	}
	if(num == 1){
		power = 3;
		return power;
	}

	 while(num != 1){
     
     		 if(num % 2 != 0){
        		 power = 0;
			break;
		 }

        num = num/2;
   }

	return power;

}
