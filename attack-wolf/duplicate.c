#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <assert.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sys/sysinfo.h>
#include <utility> 
#include <sys/personality.h>


#include <sched.h>
#include <inttypes.h>
#include <time.h>



#define PAGE_SIZE 4096
#define TOGGLES 2000000
#define HAMMER_ITERATIONS 1
#define NUM_PATTERNS 4
#define TEST_ITERATIONS 10
#define MAX_POSITION 9
#define JUNK1 5
using namespace std;

int STACK_SIZE = 0;
int NUMBER_PAGES=0,NUMBER_TARGET=6;
int TARGET_PAGE[100];
size_t mem_size;
char *memory;
double fraction_of_physical_memory = 0.6;

int *hammer;
uintptr_t attackPages[10086], abovePages[10086], belowPages[10086],offset[10086];

enum Pattern { zero_zero_zero, zero_zero_one, one_zero_zero, one_zero_one };

struct PageCandidate {
    uint64_t pageNumber;
    uint64_t abovePage;
    uint64_t belowPage;

    uint8_t* pageVA;
    uint8_t* aboveVA[2];
    uint8_t* belowVA[2];
    uint8_t offset;
};


uint64_t getPage(uint8_t* virtual_address) {
    int pagemap = open("/proc/self/pagemap", O_RDONLY);
    assert(pagemap != -1);

    uint64_t value;
    int got = pread(pagemap, &value, 8, (reinterpret_cast<uintptr_t>(virtual_address) / 0x1000) * 8);
    assert(got == 8);
    uint64_t page_frame_number = value & ((1ULL << 54) - 1);
    assert(page_frame_number != 0);
    close(pagemap);
    return page_frame_number;

}




bool pagesFilled(PageCandidate p) {
    if(p.pageVA != 0 && p.aboveVA[0] != 0 && p.belowVA[0] != 0) {
        return true;
    } else {
        return false;
    }
}

/*
void *GetBlockByOrder(int order){
    size_t s=PAGE_SIZE*pow(2, order);
    mem_size = s;
    void *ptr = memory;

    for (uint64_t index = 0; index < s; index += 0x1000) {
        uint64_t* temporary = reinterpret_cast<uint64_t*>(static_cast<uint8_t*>(ptr) + index);
        temporary[0] = index+1;
    }
    return ptr;
}
*/
void *GetBlockByOrder(int order){

	size_t s=200*4096;
        mem_size = s;
        
    void *ptr = memory;
    assert(ptr!= (void*)-1);

    for (uint64_t index = 0; index < s; index += 0x1000) {
        uint64_t* temporary = reinterpret_cast<uint64_t*>(static_cast<uint8_t*>(ptr) + index);
        temporary[0] = index+1;
    }
    return ptr;
}


uint64_t GetPhysicalMemorySize() {
    struct sysinfo info;
    sysinfo(&info);
    return (size_t) info.totalram * (size_t) info.mem_unit;
}

void setupMapping() {
    mem_size = (size_t) (((GetPhysicalMemorySize()) *
                          fraction_of_physical_memory));

    printf("MemorySize: %ld\n", mem_size);  


    memory = (char *) mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
                           MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    hammer = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(memory != MAP_FAILED);

    for(size_t i = 0; i < mem_size; i++) {
	    memory[i] = 77 + i;
	    //printf("%lx\n",getPage((uint8_t*)&memory[i]));
    }
}

void fillMemory(uint8_t* victimVA, uint8_t* aboveVA, uint8_t* belowVA) {
    memset((void *) victimVA, 0x00, PAGE_SIZE);

    uint8_t lowerBits = 0x00;
    uint8_t upperBits = 0x01;
    for(int i = 0; i < PAGE_SIZE; i++) {
        if(i % 2 == 0) {
            memset((void *) (aboveVA + i), lowerBits, 1);
            memset((void *) (belowVA + i), lowerBits, 1);
        } else {
            memset((void *) (aboveVA + i), upperBits, 1);
            memset((void *) (belowVA + i), upperBits, 1);
        }
    }
}




void rowhammer(uint8_t* aboveVA, uint8_t* belowVA, int iterations, int togs) {
    for(int i = 0; i < iterations; i++) {
        volatile uint64_t *f = (volatile uint64_t *)aboveVA;
        volatile uint64_t *s = (volatile uint64_t *)belowVA;
        unsigned long iters = togs;

        for(; iters --> 0;) {
            asm volatile("clflush (%0)" : : "r" (f) : "memory");
            *f;
            asm volatile("clflush (%0)" : : "r" (s) : "memory");
            *s;
        }
    }
}





void addVAstoPages(vector<PageCandidate> &pages) {
    printf("Searching for page VAs...\n");
    uint8_t* moverVA = (uint8_t*) memory;
    while (moverVA < (((uint8_t*) memory) + mem_size)) {
        uintptr_t moverPA = getPage(moverVA);
	    bool match = false;
        for(int i = 0; i < pages.size(); i++) {
            PageCandidate p = pages[i];
            if(p.pageNumber == moverPA) {
                p.pageVA = moverVA;
		        match = true;
            }

            if(p.abovePage == moverPA) {
                p.aboveVA[0] = moverVA;
	        	match = true;
            }
            if(p.belowPage == moverPA) {
                p.belowVA[0] = moverVA;
	        	match = true;
            }

            pages[i] = p;
        }



            moverVA += 0x1000;
    }
    printf("Done searching...\n\n");
}







char output[10086][10086];


int main(int argc, char *argv[]) {
    printf("Starting program...\n");
    int opt;
    while ((opt = getopt(argc, argv, "to:")) != -1) {
        switch(opt) {
            case 'o':
                STACK_SIZE = (int) strtol(optarg, NULL, 10);
		        break;
            case 't':
                
                printf("Starting test mode...\n\n");
                break;
        }
    }
 cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(4, &set);

        if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
            printf("ERROR WITH SCHEDAFFINITY");
            
    printf("Stack size is %i\n", STACK_SIZE);
    FILE *fp=fopen("bitflip_addrs_temp","r");
    FILE *fp2=fopen("bitflip_addrs","w");
    if(fp==NULL){
    printf("can not load file!\n");
    }
    printf("\n");
    char line[1000];
    int bit,i;
    long t1,t2;

    while(fgets(line,sizeof(line),fp)!=NULL)
    {
    	line[strcspn(line,"\n")]='\0';
    	for(i=0;i<NUMBER_PAGES;i++)
    	{
    	if(strcmp(output[i],line)==0){
    	break;
    	
    	}
    	
    	}
    	printf("%d\n",i);
    	
    	if(i==NUMBER_PAGES){
    	strcpy(output[NUMBER_PAGES],line);
    	fprintf(fp2,"%s\n",output[NUMBER_PAGES]);
    	
    	NUMBER_PAGES++;
    	
    	
    	}
    	
    	
    	
    	
 
    	
 
    	}
    printf("get %d pages!\n",NUMBER_PAGES);


    return 0;
}
