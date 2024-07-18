#include <wolfssl/options.h>
//#include <iostream>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/memory.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/wolfcrypt/sp_int.h>
#include <wolfssl/wolfcrypt/curve25519.h>
#include <wolfssl/wolfcrypt/ed25519.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
//#include <vector>

#include <sched.h>
#include <inttypes.h>
#include <time.h>

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

 

void print_x(byte *cipher, int len)
{
FILE* fp3=fopen("sign.txt","a+");    
for (int i = 0; i < len; i++)
    {
        fprintf(fp3,"%02X", cipher[i]);
    }
   fprintf(fp3,"\n");
   fclose(fp3);
}
void bit_flip(byte *target,int i){
    int pos=i/(sizeof(byte)*8);
    int num=i%(sizeof(byte)*8);
    //printf("pos,num: %d %d\n",pos,num);
    byte tmp=1;
    for(int k=0;k<num;k++){
        tmp=tmp*2;
    }
    target[pos]=target[pos]^tmp;
}
void read_from_hex(byte *target, word32 num, char *hex){
    for (int i = 0; i < num; i++) {
        sscanf(hex + 2 * i, "%2hhx", &target[i]);
    }
}

char s_hex[]="E65F2664920521728A94CAC7D50C3E16C0BAB06A690279DBDF3"
            "A2E80A308B80C3B398C0A56A9AF7E302D494ED8A7F7968220E3F0EAC8ECAB728284C25D75F566";
byte sec2[64];
byte sec[64];
int main()
{
    ed25519_key key;
    WC_RNG rng;
    word32 ret, sigSz;
    int verified = 0;

    byte sig[64]; // will hold generated signature
    sigSz = sizeof(sig);
    byte message[] = {"hello world"};

    wc_InitRng(&rng);                    // initialize rng
    wc_ed25519_init(&key);               // initialize key
    //wc_ed25519_make_key(&rng, 32, &key); // make public/private key pair
    char pub_hex[]="D579E7F40D32BE867FDD9163FFE18D54B5BD6D57463DCA1D54A396AD77E9DCC6";
    char priv_hex[]="C1361B023124F32E0E7CA9F1C8F8B7165A9F2BAB05B6EC8E2064F00B0FDA87CD";
    byte pub[32];
    word32 pubSz = sizeof(pub);
    byte priv[32];
    word32 privSz = sizeof(priv);
    read_from_hex(pub,pubSz,pub_hex);
    read_from_hex(priv,privSz,priv_hex);
    read_from_hex(sec,64,s_hex);
    wc_ed25519_import_private_key(priv,privSz,pub,pubSz,&key);
    
    
    memcpy(sec2, sec, 64);
    printf("fpip addr: %p\n",sec2);
    printf("d PFN:%lx\n",getPage(sec2));
    sleep(4);
    //Rowhammer bit flip here, or the global variable sec
    //bit_flip(sec2, 98);
    
    //Sign message
    ret=wc_ed25519_sign_msg2(message,sizeof(message),sig,&sigSz,&key,sec2);
    if(ret!=0){
        printf("not found\n");
        print_x(sig,sigSz);
    }
    else{
        print_x(sig,sigSz);
    }
    //print_x(sig,sigSz);
    return 0;
}


//#####################

    // for (int i = 0; i < 5; i++)
    // {
    //     byte sec2[64];
    //     memcpy(sec2,sec,64);
    //     bit_flip(sec2,i);
    //     ret = wc_ed25519_sign_msg2(message, sizeof(message), sig, &sigSz, &key,sec2);
    //     if (ret != 0)
    //     {
    //         printf("sign wrong\n");
    //         // error generating message signature
    //     }
    //     //printf("sig: %d\n",sigSz);
    //     //print_x(sig,sigSz);
    //     ret = wc_ed25519_verify_msg(sig, sizeof(sig), message, sizeof(message), &verified,
    //                                 &key);
    //     if (ret < 0)
    //     {
    //         printf("verify wrong\n");
    //         // error performing verification
    //     }
    //     else if (verified == 0)
    //     {
    //         printf("verify not pass\n");
    //     }
    //     else
    //     {
    //         printf("verify passed\n");
            
    //     }
    // }
