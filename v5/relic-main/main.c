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

int STACK_SIZE = 0;

size_t mem_size;
char *memory;
double fraction_of_physical_memory = 0.1;



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
    FILE* fp3=fopen("sign.txt","a");
    fprintf(fp3,"\n");
    FILE* fp4=fopen("truth.txt","r");
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

}
int main(){
    
    if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}
    
    if (pc_param_set_any() == RLC_OK){
        util_printf("pc_param: %d\n",pc_param_level());
        int code = RLC_ERR;
	    bn_t d;
        bn_t d2;
	    g1_t s;
	    g2_t q;
	    gt_t z;
        gt_t z1;
        uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN];
        bn_null(d);

	    g1_null(s);
        bn_new(d);
q
		g1_new(s);
        gt_new(z1);
        init(d);
        printf("PFN:%lx, logic address:%lx\n",getPage((uint8_t *)&d), &d);
        
            //signature here,
        if(cp_bbs_sig(s, m, sizeof(m), 0, d) != RLC_OK){
            util_printf("sign wrong\n");
            assert(0);
        }
        
        
        save_sign(s);
        
        util_printf("finish\n");
    }
}
