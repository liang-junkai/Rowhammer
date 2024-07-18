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

static char sk2[]="7B2C1FBDBF650F0E76136967AB67E0A514F1AC99D8357DA0A02F4192";
 
/*
* num of wrong signtures
* this algorithm will save sk to sk.txt, pp to pp.txt and signatures to sign.txt,
* creat a total of N signtures
*/
void init(bn_t d){
   
    bn_read_str(d,sk2,strlen(sk2),16);
    //签名没必要用公共参数,我单独保存下来就行
    //g2_mul_gen(q, d);
    //gt_get_gen(z);
}


void save_sign(g1_t s, uint8_t* sign_test){//格式： 长度+签名的二进制内容
      
    //FILE* fp4=fopen("truth.txt","r");
    uint8_t sign_buf[1024];
    size_t s_len=g1_size_bin(s,0);
    fwrite(&s_len,sizeof(s_len),1,fp3);
    memset(sign_buf,0,1024);
    g1_write_bin(sign_buf,1024,s,0);
    
    
    
    
    //compare true_sign with test_sign here.
    for(int i=0;i<s_len;i++){
    fprintf(fp3,"%02x",sign_buf[i]);
    
    }
}





int main()
{
    
    uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN],s_len=65;
    char truth[1024]; 
    uint8_t sign_test[1024];
    
    //true signature
    if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}
    
    if (pc_param_set_any() == RLC_OK){
        int code = RLC_ERR;
	bn_t d;
	printf("beign sign\n");
	sleep(10);
        bn_null(d);
        bn_new(d);
        init(d);
	    g1_t s;
        gt_t z1;
        uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN];
	    g1_null(s);
		g1_new(s);
        gt_new(z1);
       // printf("PFN:%lx, logic address:%lx\n",getPage((uint8_t *)&d), &d);
        printf("sk2 PFN:%lx, logic address:%lx %lx\n",getPage((uint8_t *)sk2), &sk2[0], &sk2[55]);
       // printf("size:%d\n",sizeof(sk2));
        
    //  signature here,
        if(cp_bbs_sig(s, m, sizeof(m), 0, d) != RLC_OK){
            util_printf("sign wrong\n");
            assert(0);
        }
        
    }
    
    
    //get faulty signature
    while(fgets(truth,sizeof(truth),fp4)!=NULL)
    {
    
    
    int temp1,temp2;
    for(int i=0;i<s_len;i++){
    
    sprintf(&temp1, "%x", truth[i*2]);
    sprintf(&temp2, "%x", truth[i*2+1]);
    sign_test[i]=temp1*16+temp2;
    
    
    }
    compare_sign(s,sign_test);
    
    

    }
    
    
    }
