#include <relic/relic.h>
#include <relic/relic_test.h>
#include <relic/relic_bench.h>
#include<assert.h>

int main(){
    FILE *fp1=fopen("./pp.txt","rb");
    FILE *fp2=fopen("./sign.txt","rb");
    if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}
    if (pc_param_set_any() == RLC_OK){
        bn_t d;
        bn_t d2;
	    g1_t s;
	    g2_t q;
	    gt_t z;
        gt_t z1;
        uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN];
        bn_null(d);
        bn_null(d2);
	    g1_null(s);
	    g2_null(q);
	    gt_null(z);
        gt_null(z1);
        bn_new(d);
        bn_new(d2);
		g1_new(s);
		g2_new(q);
		gt_new(z);
        gt_new(z1);
        char pp_q[1024]={0};
        char pp_z[1024]={0};
        char sk[1024]={0};
        //bn_set_dig(d,0);
        memset(sk,0,1024);
        bn_write_str(sk,1000,d,16);
        util_printf("d first: %s\n",sk);
        bn_set_dig(d,0);

        size_t q_len;
        size_t z_len;
        fread(&q_len,sizeof(q_len),1,fp1);
        fread(&z_len,sizeof(z_len),1,fp1);
        fread(pp_q,1,q_len,fp1);
        fread(pp_z,1,z_len,fp1);
        g2_read_bin(q,pp_q,q_len);
        gt_read_bin(z,pp_z,z_len);

        for(size_t i=0;i<200;i++){
            size_t s_len;
            fread(&s_len,sizeof(s_len),1,fp2);
            util_printf("s_len: %d\n",s_len);
            char sign_s[100];
            fread(sign_s,1,s_len,fp2);
            g1_read_bin(s,sign_s,s_len);
            util_printf("here1\n");
            g2_t g_2;g2_null(g_2);g2_new(g_2);
            bn_t c1;bn_null(c1);bn_new(c1);
            gt_t z3;gt_null(z3);gt_new(z3);

            bn_set_2b(c1,i);
            g2_mul_gen(g_2,c1);
            pc_map(z1,s,g_2);
            gt_inv(z1,z1);
            gt_mul(z3,z,z1);
            util_printf("here\n");
            if(cp_bbs_ver(s, m, sizeof(m), 0, q, z3) == 1){
                bn_set_bit(d,i,0);
            }
            else{
                gt_inv(z1,z1);
                gt_mul(z3,z,z1);
                if(cp_bbs_ver(s, m, sizeof(m), 0, q, z3) == 1){
                    bn_set_bit(d,i,1);
                }
                else{
                    util_printf("cannot verify!\n");
                    assert(0);
                }                
            }
            memset(sk,0,1024);
            bn_write_str(sk,1000,d,16);
            util_printf("d: %s\n",sk);
            //break;
        }
        fclose(fp1);
        fclose(fp2);
        util_printf("g1 size: %d, g2 size: %d, gt size: %d\n",g1_size_bin(s,0),
            g2_size_bin(q,0),gt_size_bin(z,0));
    }
}