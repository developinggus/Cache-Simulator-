#include<stdio.h>
#include<stdlib.h>
#include<math.h>
//-----------------------------------------------------------------------------
typedef struct block { //Will store tag and v(valid)

 size_t tag;

 size_t lru;

 struct block * next; //points to next block in set

} block;
//-----------------------------------------------------------------------------
typedef struct line { //will be the array the points to the linked lists
 
 struct block** set;

} line;
//-----------------------------------------------------------------------------
// Global Variables
int hit   = 0;
int miss  = 0;
int write = 0;
int read  = 0;
//-----------------------------------------------------------------------------
line* writer (line* cache, size_t address, int offsetB, int setB, int anum, int policy);
line* reader (line* cache, size_t address, int offsetB, int setB, int anum, int policy);
int spaceCheck (line* cache, size_t s, int anum);
line* fifoHit (line* cache, size_t s, size_t tag);
line* lruHit (line* cache, size_t s, size_t tag, int anum);
line* lruFull (line* cache, size_t s, size_t tag, int anum);
line* lruNotFull (line* cache, size_t s, size_t tag, int anum);
line* cacheClear (line* cache, int sum);
line* prefetcher(line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize);
line* readerPF(line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize);
line* writerPF(line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize);
void freeMe(line* cache, int snum);
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {

 int cachesize = atoi(argv[1]);
 int blocksize = atoi(argv[2]);
 int prefetch  = atoi(argv[5]);
 
 //printf("Cachesize is: %d\n" , cachesize);
 //printf("blocksize is %d\n" , blocksize);

 //int count = 0; 

 int anum,snum,offsetB,setB,i; //associativity,number of sets,offset,set bits

 char action;
 size_t address; 

 int policy; // 0 for FIFO 1 for LRU 

 //associativity 

 if ( argv[4][0] == 'a' ) { //it's either assoc or assoc:n;
  
  if ( argv[4][5] != ':' ) { // fully assoc , one set, all blocks in that one set
   //printf("Line 58\n");

   snum = 1;
   anum = cachesize/blocksize;

  } else { // set assoc
   //printf("Line64\n");
   int n;
   sscanf(argv[4],"assoc:%d", &n); //getting n;
   //printf("Line 67\n");
   anum = n;
   int temp = n * blocksize; //there will be n blocks of size blocksize
   snum = cachesize/temp; 
   //printf("Line 70\n");

  }

 } else { //its a direct cache
  //printf("Its a direct\n");  
  anum = 1; //one block per set
  snum = cachesize/blocksize;
 
 }
 
 //printf("Anum is : %d\n", anum);
 //printf("Snum is : %d\n",snum);

 

 //creating cache 
 
 line* cache = (line*) malloc(sizeof(line));
 cache->set = malloc(snum*sizeof(block*));

 for ( i = 0; i < snum; i++ ) { //setting all sets to NULL
  cache->set[i] = NULL;
 }
 

 //bit sizes of the offset, set and tag
 
 offsetB = log(blocksize)/log(2);
 setB = log(snum)/log(2);

 //printf("Offset bits: %d\n", offsetB);
 //printf("set bits: %d\n", setB);
 
 //checking policy
 
 if (argv[3][0] == 'f' ) { //assuming inputs are correct, fifo
  policy = 0;
 } else {
  policy = 1; //lru
 }
 /*
 if ( policy == 1 ) {
  return 0;
 }
 */
 //first read, no pre-fetch
 
 FILE* fefe;
 fefe = fopen(argv[6], "r");

 while(fscanf(fefe,"%c %zx\n", &action, &address) == 2) {
 
  if ( action == 'R' ) { //its a read
   //count++;
   //printf("READI fried\n");
   cache = reader(cache,address,offsetB,setB,anum,policy);
   //printf("Preformed the reader\n");
  } else if ( action == 'W') {//its a write
   //count++;
   //printf("Write i fired\n");
   cache = writer(cache,address,offsetB,setB,anum,policy);
   //printf("Preformed the writer\n");
  } else {

   //we're at the end
  }

 }
 /*
 block * p = cache->set[0];
 while ( p != NULL ) {
  printf("%zx\n",p->tag);
  p = p->next;
 }
 */
 printf("no-prefetch\n");
 printf("Memory reads: %d\n",read);
 printf("Memory writes: %d\n",write);
 printf("Cache hits: %d\n", hit);
 printf("Cache misses: %d\n",miss);
 //printf("Snum are %d\n",snum);
 //printf("p is %d\n",policy);
 
 //now we do it all again, this time with prefetch
 //to make it easier to read in the future, im creating new methods for this, 
 //since the existing ones are already extremly cluttered
 fclose(fefe);
 cache = cacheClear(cache,snum); //clearing the cache

 FILE* fefe2;
 fefe2 = fopen(argv[6],"r");
 
 read  = 0;
 write = 0;
 hit   = 0; //the outputs were already printed out, now we can reset them to zero
 miss  = 0;

 while(fscanf(fefe2,"%c %zx\n", &action, &address) == 2) {
  
  if ( action == 'R' ) {
   cache = readerPF(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
  } else if ( action == 'W' ) {
   cache = writerPF(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
  } else {
 
  }
 } 


 printf("with-prefetch\n"); 
 printf("Memory reads: %d\n",read);
 printf("Memory writes: %d\n",write);
 printf("Cache hits: %d\n", hit);
 printf("Cache misses: %d\n",miss);

 freeMe(cache,snum);
 fclose(fefe2);
 return 0;
 
}
//-----------------------------------------------------------------------------
void freeMe (line* cache, int snum) {
 block * d = NULL;
 block * hold = NULL;
 for ( int i = 0; i < snum; i++ ) {
  if ( cache->set[i] != NULL ) {
   d = cache->set[i];
   while ( d != NULL ) {
    hold = d->next;
    free(d);
    d = hold;
   }
   hold = d;
   free(hold);
  }
 }

 free(cache);

}
//-----------------------------------------------------------------------------
line* reader (line * cache, size_t address , int offsetB, int setB, int anum, int policy) {

 size_t setmask = ((1<<setB) - 1);
 size_t s = (address>>offsetB)&setmask;
 size_t tag = (address>>(setB+offsetB));

 //now we know the set num, now we check the cache 
 if ( cache->set[s] == NULL ) { //nothing there
  miss++;
  read++;
  //creating node to insert
  block* temp = (block*) malloc(sizeof(block));

  temp->tag = tag;

  if ( policy == 1 ) { 
   temp->lru = anum - 1;// for lru, the first input is the most recently used
  }
  cache->set[s] = temp;
  
  printf("First insert! Tag is %zx in set %zx\n", tag , s);

  return cache;

 } else { //there are other blocks in there
 //pointer 
  block * p = NULL;
  p = cache->set[s];

  while ( p != NULL ) { //going to search the linked list and search for the tag
   if ( p->tag == tag ) { //its a hit!
    hit++;
    printf("TAG %zx is a hit\n", tag);
    
    if ( policy == 0 ) { 
     cache = fifoHit(cache,s,tag);
    } else {  //remove the tag, then reinsert and end of LL, store LRU
     cache = lruHit(cache,s,tag,anum);   
    }
    // its a hit, so, we store the hit tags lru in some var, we then decriment all the other LRU's that are 
    // greater than some var. Then, we set the hit tag's lru to s - 1.  
  
    return cache;
   }
   p = p->next;
  }

  //looks like it's not found, we have to read it in, first lets check to see if its full
 
  miss++;
  read++;
  //printf("READ: NOT found, inserting %zx, into set %zx\n",tag,s);
  int space = spaceCheck(cache,s,anum); // returns 0 if there's space, 1 if its full
  //printf("Line 195\n");
  block* temp = (block*) malloc(sizeof(block));
  temp->tag = tag;
  //printf("Space is %d\n",space);
  if ( space == 0 ) { 
   p = cache->set[s];

   while ( p->next != NULL) {
    p = p->next; //going to the last node
   }

   p->next = temp;
   
   if ( policy == 1 ) {
    cache = lruNotFull(cache,s,tag,anum); 
   }
   printf("Just inserted tag %zx in set %zx\n", tag, s);
   /*
   block * p = cache->set[s];
   while ( p != NULL) {
    printf("Not full: tag %zx in set %zx", p->tag, s);
    p = p->next;
   }
   printf("\n");
   */
   return cache;

  } else { //there's no space, implement fifo or lru

   if ( policy == 0 ) { //fifo
    //printf("Line 220\n");
    p = cache->set[s];
    p = p->next;
    cache->set[s] = p;
    //printf("Line 223\n");
    p = cache->set[s];
    
    
    if ( p == NULL) {
     cache->set[s] = temp;
     return cache;
    }

    while( p->next != NULL ) {
     p = p->next;
    }
    //printf("Line 230\n"); 
    p->next = temp;
    printf("Inserted %zx in set %zx\n", tag, s);
    /*
    block * p = cache->set[s];
    while ( p != NULL) {
     printf("FULL Cache: tag %zx in set %zx", p->tag, s);
     p = p->next;
    }
    printf("\n");
    */
    
    //printf("Line 233\n");
    return cache;

   } else { //lru
 
    cache = lruFull(cache,s,tag,anum);
    
    return cache;
   
   }

  }

 }

 return cache;

}
//-----------------------------------------------------------------------------
line* writer (line* cache, size_t address, int offsetB, int setB, int anum, int policy) {
 //we have to write
 write++; //whether it is a hit or miss, we incriment

 size_t setmask = ((1<<setB) - 1);
 size_t s = (address>>offsetB)&setmask;
 size_t tag = address>>(offsetB+setB);

 //printf("Write! Set is: %zx, tag is: %zx\n", s,tag);
 if ( cache->set[s] == NULL ) { //empty
  //printf("WRITE: Empty! Inserting %zx into set %zx\n", tag,s);
  read++;
  miss++;
  
  block* temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if ( policy == 1 ) {
   temp->lru = anum - 1;
  }
 
  cache->set[s] = temp;
  
  return cache;

 } else { //not empty, lets check if its in there.....somewhere
  block* p = NULL;
  p = cache->set[s];

  while (p != NULL) {
   if ( p->tag == tag ) {
    hit++;
    //printf("Write! FOund %zx in set %zx\n",tag,s);
     
    if ( policy == 0 ) {
     cache = fifoHit(cache,s,tag);
    } else {
     //printf("I FIRED\n");
     cache = lruHit(cache,s,tag,anum);
    }

    return cache;
   }
   p = p->next;
  }

  //hmm..doesn't seem to be there, look like we have to read it in
  read++;
  miss++;

  int space = spaceCheck(cache,s,anum); //return 0 if there's room!
  block* temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if ( space == 0 ) { //there's room
   p = cache->set[s];

   while ( p->next != NULL ) {
    p = p->next;
   }

   p->next = temp;
   
   if ( policy == 1 ) {
    cache = lruNotFull(cache,s,tag,anum);
   }

   return cache;

  } else { //looks like there's no room! have to implement FIFO

   if ( policy == 0 ) {
    p = cache->set[s];
    p = p->next;
    cache->set[s] = p;

    if ( p == NULL ) {
     cache->set[s] = temp;
     return cache;
    }

    while ( p->next != NULL ) {
     p = p->next;
    }

    p->next = temp;

    return cache;

   } else {
   
    cache = lruFull(cache,s,tag,anum);
     
    return cache;
 
   }

  }

 }

 return cache;
}
//-----------------------------------------------------------------------------
int spaceCheck(line* cache, size_t s, int anum) { //checks to see if set is full!
 int count = 0;

 block* p = NULL;
 p = cache->set[s];

 while ( p != NULL ) {
  count++;
  p = p->next;
 }

 if ( count == anum ) { //its full!
  return 1;
 }

 return 0;

}
//-----------------------------------------------------------------------------
line* fifoHit (line* cache, size_t s, size_t tag) {
//* THIS METHOD IS NOT NEEDED AND DOES ABSOULUTLEY NOTHING
//* I implemented fifo wrong, the program doesn't change it i keep this or 
// remove it since I already commented out the linked list rearrangment, so i'll
// just keep it. 
 return cache;
} 
//-----------------------------------------------------------------------------
line* lruHit (line* cache, size_t s, size_t tag, int anum) {
 
 block * p = NULL;
 p = cache->set[s];


 size_t tempLru;
 size_t temp;
 
 p = cache->set[s];
 while ( p != NULL ) {
  if ( p->tag == tag ) {
   tempLru = p->lru;
   //printf("Templru is:%zx\n",tempLru);
  } 
  p = p->next; 
 }

 p = cache->set[s];
 
 while ( p != NULL ) {
  temp = p->lru;
  //printf("p->lru is:%zx\n", temp);

  if ( temp > tempLru ) {
   p->lru--;
   //printf("Just decrimented tag %zx. Now its %zx\n", p->tag, p->lru);
  }
  p = p->next;
 }
 
 
 p = cache->set[s];

 while ( p != NULL ) {
  if ( p->tag == tag ) {
   p->lru = anum - 1;
  }
  p = p->next;
 }
 
 //printf("Updated list is now: \n");

 p = cache->set[0];
 
 return cache;

}

//-----------------------------------------------------------------------------
line* lruFull (line* cache, size_t s, size_t tag, int anum) {
 
 block * p = NULL;
 p = cache->set[s];

 while ( p != NULL ) {
  if ( p->lru == 0 ) {
	  p->lru = anum - 1;
	  p->tag = tag;
  }
  p = p->next;
 }

 p = cache->set[s];

 while ( p != NULL ) {
  if ( p->tag != tag) {
   p->lru--;
  }
  p = p->next;
 }

 return cache;

}
//-----------------------------------------------------------------------------
line* lruNotFull (line* cache, size_t s, size_t tag, int anum) {

 block * p = cache->set[s];
 
 while ( p != NULL ) {
  if ( p->tag == tag ) {
   p->lru = anum - 1;
  } else {
   p->lru--;
  }
  p = p->next;
 }

 return cache;
}
//-----------------------------------------------------------------------------
line* readerPF (line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize ) {
 
 //this is the prefetch reader
 
 size_t setmask = ((1<<setB) - 1);
 size_t s = (address>>offsetB)&setmask;
 size_t tag = (address>>(setB+offsetB));

 if (cache->set[s] == NULL) {
  miss++;
  read++;

  block* temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if ( policy == 1 ) {
   temp->lru = anum - 1;
  }
  
  cache->set[s] = temp;

  // prefetching
  //printf("Just inserted into set %zw, for the first time. The tag is %zw",s,tag);
  cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);

  return cache;

 } else { //there's other blocks
 
  block * p = NULL;
  p = cache->set[s];

  while( p != NULL ) {
   if ( p->tag == tag ) { //found the tag
    hit++;
    if ( policy == 1 ) {
     cache = lruHit(cache,s,tag,anum);
    }
    
    return cache;
   }
   p = p->next;
   
  }

  //its not found, we have to read it in, and PREFETCH

  miss++;
  read++;

  int space = spaceCheck(cache,s,anum);
  block * temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if ( space == 0 ) { //theres space
   p = cache->set[s];
   
   while ( p->next != NULL ) {  
    p = p->next;
   }
   p->next = temp;

   if ( policy == 1 ) {
    cache = lruNotFull(cache,s,tag,anum);
   }
   
   cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);

   return cache;

  } else { //theres no space, implement fifo

   if ( policy == 0 ) { //fifo
    p = cache->set[s];
    p = p->next;
    cache->set[s] = p;
    p = cache->set[s];

    if ( p == NULL ) {
     cache->set[s] = temp;
     cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
     return cache;
    }

    while (p->next != NULL) {
     p = p->next;
    }

    p->next = temp;
    cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
    return cache;

   } else { //lru

    cache = lruFull(cache,s,tag,anum);
    cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
    return cache;

   }

  }

 }

 return cache;

}
//-----------------------------------------------------------------------------
line* writerPF(line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize) {

 write++;

 size_t setmask = ((1<<setB) - 1);
 size_t s = (address>>offsetB)&setmask;
 size_t tag = address>>(offsetB+setB);

 if ( cache->set[s] == NULL ) { //empty
  read++;
  miss++;

  block * temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if ( policy == 1 ) {
   temp->lru = anum-1;
  }

  cache->set[s] = temp;

  cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
  return cache;

 } else { //not empty, lets check if its in there
  block * p = NULL;
  p = cache->set[s];

  while(p != NULL) {
   if (p->tag == tag) {
    hit++;
    if ( policy == 1 ) {
     cache = lruHit(cache,s,tag,anum);
    }

    return cache;
   }
   p = p->next;
  }

  //not there! we have to read it in
  read++;
  miss++;

  int space = spaceCheck(cache,s,anum);
  block * temp = (block*) malloc(sizeof(block));
  temp->tag = tag;

  if (space == 0) { //theres room
   p = cache->set[s];

   while (p->next != NULL) {
    p = p->next;
   }

   p->next = temp;

   if (policy == 1) {
    cache = lruNotFull(cache,s,tag,anum);
   }

   cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
   return cache;

  } else { //looks like theres not room, fifo or lru

   if (policy == 0) {
    p = cache->set[s];
    p = p->next;
    cache->set[s] = p;

    if (p == NULL) {
     cache->set[s] = temp;
     cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
     return cache;
    }

    while (p->next != NULL) {
     p = p->next;
    }

    p->next = temp;

    cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);

    return cache;

   } else {

    cache = lruFull(cache,s,tag,anum);
    cache = prefetcher(cache,address,offsetB,setB,anum,policy,prefetch,blocksize);
    return cache;
   }
  }
 }
 return cache;
}
//-----------------------------------------------------------------------------
line* cacheClear (line* cache, int snum) {

 int c = snum;

 while ( c > 0 ) {
  cache->set[c-1] = NULL;
  c--;
 }

 return cache;

}
//-----------------------------------------------------------------------------
line* prefetcher(line* cache, size_t address, int offsetB, int setB, int anum, int policy, int prefetch, size_t blocksize) {

 //no updates to hits,miss,writes will be made
 
 size_t setmask = ((1<<setB) - 1);
 int c = prefetch;
 size_t tempAdd = address;
 size_t tempS;
 size_t tempTag;
 int found;
 while ( c > 0 ) { //amount of times we're gonna prefetch
  found = 0;
  tempAdd = tempAdd + blocksize;
  tempS = (tempAdd>>offsetB)&setmask;
  tempTag = tempAdd>>(offsetB+setB);

  //now we search to see if the tag is already there

  if (cache->set[tempS] == NULL) { //its not there!
   block * temp = (block*) malloc(sizeof(block));
   temp->tag = tempTag;

   if (policy == 1) {
    temp->lru = anum-1;
   }
   read++; 
   cache->set[tempS] = temp;
  
  } else { //not empty, might be in there

   block * p = NULL;
   p = cache->set[tempS];
   
   while ( p != NULL) {
    if ( p->tag == tempTag) {
     //found
     found = 1;
    }
    p = p->next;
   }

   if ( found == 0 ) {//not found , we have to read it in
    read++;

    int space = spaceCheck(cache,tempS,anum);
    block * temp = (block*) malloc(sizeof(block));
    temp->tag = tempTag;

    if ( space == 0 ) {
     p = cache->set[tempS];

     while(p->next != NULL) {
      p = p->next;
     }
     p->next = temp;

     if (policy == 1) {
      cache = lruNotFull(cache,tempS,tempTag,anum);
     }

    } else { //not room, fifo or lru

     if (policy == 0) {
      p = cache->set[tempS];
      p = p->next;
      cache->set[tempS] = p;

      if ( p == NULL) {
       cache->set[tempS] = temp;
      } else {
       while (p->next != NULL) {
        p = p->next;
       }
       p->next = temp;
      }
     } else {
      cache = lruFull(cache,tempS,tempTag,anum);
     }
    }
   }
  }
  c--;
 }
 return cache;
}


     


   
