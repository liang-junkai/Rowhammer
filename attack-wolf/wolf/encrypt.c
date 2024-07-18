

#include <wolfssl/options.h>
#include <iostream>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/memory.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>
#include<wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/types.h>
#include<wolfssl/wolfcrypt/sp_int.h>
#if defined(WOLFSSL_PUBLIC_MP)
using namespace std;
void print_x(byte *cipher,int len){
    for(int i=0;i<len;i++){
        printf("%02X",cipher[i]);
    }  
    printf("\n"); 
}  
void print_mp(mp_int *tp){
	byte out[256];
	mp_to_unsigned_bin(tp,out);
	print_x(out,mp_bitsused(tp)/8);
}
void set_bit(mp_int* tp,int index,int bit){
	if(bit==1){
		mp_set_bit(tp,index);
	}
	else if(bit==0){
		if (mp_is_bit_set(tp, index))
		{
			mp_int a;
			mp_init(&a);
			mp_2expt(&a, index);
			mp_sub(tp, &a, tp);
		}
	}
	else{
		printf("bit must be 0 or 1\n");
	}
}
void bit_flip(mp_int *tp,int i){
	if(mp_is_bit_set(tp,i)){
		set_bit(tp,i,0);
	}
	else{
		set_bit(tp,i,1);
	}
}
int main()
{
	
	RsaKey enc, enc2;
	
	int ret,len;
	WC_RNG rng;
	
	//byte out1[256] = { 0 };
	byte out2[512] = { 0 };
 
	byte msg[128];
	for(int k=0;k<128;k++){
		msg[k]=k;
	}
	byte out[512] = { 0 };

	wc_InitRng(&rng);
	ret = wc_InitRsaKey(&enc, NULL);
	if (ret != 0) 
	{
		cout << "wc_InitRsaKey ret " << ret << endl;
		return 0;
	}
 
	ret = wc_InitRsaKey(&enc2, NULL);
	if (ret != 0) 
	{
		cout << "wc_InitRsaKey ret " << ret << endl;
		return 0;
	}
 
 
	word32 idx = 0;
	byte der2[] = { 48,129,159,48,13,6,9,42,134,72,134,247,13,1,1,1,5,0,3,129,141,0,48,129,137,2,129,129,0,222,137,20,25,217,228,50,196,202,63,143,233,102,89,117,226,238,164,36,167,196,1,139,24,143,124,118,119,139,182,191,199,10,206,204,244,21,232,155,138,240,146,29,180,123,73,231,169,122,0,34,92,27,115,123,100,187,166,68,10,166,208,210,131,37,185,242,222,236,110,65,112,211,162,191,9,164,222,228,185,76,104,235,171,29,97,12,111,126,158,158,89,7,72,23,83,155,50,219,237,135,93,122,46,79,214,3,56,204,254,237,115,50,169,169,240,122,100,88,57,45,27,189,101,101,251,183,53,2,3,1,0,1 };
	// 解密私钥，使用golang.x509.MarshalPKCS1PrivateKey生成
	byte der[] = { 48,130,2,92,2,1,0,2,129,129,0,222,137,20,25,217,228,50,196,202,63,143,233,102,89,117,226,238,164,36,167,196,1,139,24,143,124,118,119,139,182,191,199,10,206,204,244,21,232,155,138,240,146,29,180,123,73,231,169,122,0,34,92,27,115,123,100,187,166,68,10,166,208,210,131,37,185,242,222,236,110,65,112,211,162,191,9,164,222,228,185,76,104,235,171,29,97,12,111,126,158,158,89,7,72,23,83,155,50,219,237,135,93,122,46,79,214,3,56,204,254,237,115,50,169,169,240,122,100,88,57,45,27,189,101,101,251,183,53,2,3,1,0,1,2,129,128,11,50,214,194,30,70,239,143,27,166,107,53,145,162,250,221,186,168,163,247,149,83,134,142,107,218,21,148,26,129,205,208,212,104,103,195,248,190,69,229,142,9,172,89,2,195,75,87,41,254,139,188,26,221,138,222,28,219,87,29,58,27,212,28,17,104,186,110,211,101,138,241,64,132,12,202,80,5,68,107,124,16,23,193,153,132,69,98,253,219,80,6,14,248,55,124,163,135,247,82,77,254,90,41,93,138,155,252,60,43,49,131,204,228,7,139,194,170,46,180,48,158,146,83,218,103,110,25,2,65,0,233,148,141,146,89,109,236,219,35,145,165,99,173,156,161,16,223,186,192,55,36,201,224,206,122,237,27,70,146,145,6,25,162,53,251,58,7,253,30,226,38,245,11,95,180,115,102,136,136,242,110,248,52,175,7,147,20,207,228,117,145,244,194,195,2,65,0,243,229,34,227,124,118,74,110,155,160,233,86,186,71,169,232,9,170,156,246,31,145,51,114,207,90,30,54,21,214,179,179,194,234,86,91,55,74,156,54,75,8,231,43,212,111,195,58,228,184,231,118,199,246,184,241,232,68,42,183,226,14,14,167,2,64,68,101,33,71,55,52,126,228,115,247,211,203,234,44,48,229,117,253,131,7,34,152,146,97,35,145,134,41,22,5,173,25,152,107,226,18,78,0,138,40,130,107,194,86,213,201,236,190,18,11,154,254,198,190,113,163,89,182,190,24,199,18,58,109,2,65,0,178,255,241,170,41,247,155,75,48,119,89,169,232,79,158,88,137,119,169,121,77,211,192,129,187,194,245,55,86,177,219,243,203,211,55,11,253,57,138,10,162,233,102,216,153,50,105,131,184,200,40,64,218,35,174,187,4,111,187,54,119,247,63,51,2,64,60,232,98,115,149,59,37,75,240,146,46,31,0,247,30,79,177,249,118,123,17,134,126,243,187,66,178,46,94,68,55,94,28,26,175,20,249,134,174,121,101,45,70,212,1,80,213,19,112,89,113,233,201,169,88,37,92,30,194,249,64,149,167,6 };
	
 
	// 解密公钥，使用golang.x509.MarshalPKIXPublicKey生成
	idx = 0;
	
	ret = wc_RsaPublicKeyDecode(der2, &idx, &enc2, sizeof(der2));
	if (ret != 0) 
	{
		cout << "wc_RsaPublicKeyDecode ret " << ret << endl;
		return 0;
	}
 
 
	// 公钥加密
	len = wc_RsaPublicEncrypt(msg, sizeof(msg), out, sizeof(out), &enc2, &rng);
	if (len <= 0) 
	{
		cout << "wc_RsaPublicEncrypt ret " << len << endl;
		return 0;
	}
	cout << "encrypted:" << endl;
	
	for (int i = 0; i < len; i++) 
	{  
		cout << (int)out[i] << ",";
	} 
	cout << endl << "encrypted over" << endl;
	//wc_FreeRng(&rng);
	/*解密*/  
	//wc_InitRng(&rng);
	//ret = wc_RsaSetRNG(&enc, &rng);
	if (ret < 0)
	{
		cout << "wc_RsaSetRNG ret " << ret << endl;
		cout << wc_GetErrorString(ret) << endl;
		return 0;
	}
	idx=0;
	ret = wc_RsaPrivateKeyDecode(der, &idx, &enc, sizeof(der));
	
	if (ret != 0) 
	{
		cout << "wc_RsaPrivateKeyDecode ret " << ret << endl;
		return 0;
	}

	printf("enc.dataLen %d out len%d\n",enc.dataLen,len);
	byte out3[256]={0};
	mp_int tp,sk;
	mp_init(&tp);mp_init(&sk);
	mp_copy(&enc.d,&tp);mp_copy(&enc.d,&sk);
	mp_to_unsigned_bin(&tp,out3);
	printf("sk before flip: \n");
	print_x(out3,mp_bitsused(&tp)/8);

	mp_int result;
	mp_init(&result);
	mp_copy(&sk,&result);
	mp_zero(&result);
	int count=0;
	for(int i=0;i<100;i++){
		byte out4[256]={0};
		mp_int tp2;
		mp_init(&tp2);
		mp_copy(&sk,&tp2);
	//mp_set_bit(&tp2,1);
		bit_flip(&tp2,i);
		mp_to_unsigned_bin(&tp2,out4);
		printf("sk after flip bits: \n");
		print_x(out4,mp_bitsused(&tp2)/8);

		mp_int fp,fq;
		mp_init(&fp);
		mp_init(&fq);
		mp_sub_d(&enc.p,1,&fp);
		mp_sub_d(&enc.q,1,&fq);


		mp_mod(&tp2,&fp,&enc.dP);
		mp_mod(&tp2, &fq, &enc.dQ);

		mp_int c,ct;
		mp_init(&c);mp_init(&ct);
		mp_read_unsigned_bin(&c, out, len);
		mp_copy(&c,&ct);
		//wolfSSL_Debugging_ON();
		ret = wc_RsaPrivateDecrypt(out, len, out2, sizeof(out2), &enc);
		//wolfSSL_Debugging_OFF();
		//cout << "enc size: " << wc_RsaEncryptSize(&enc) << endl;
		if (ret <= 0)
		{

			cout << "wc_RsaPrivateDecrypt ret " << ret << endl;
			cout << wc_GetErrorString(ret) << endl;
			return 0;
		}
		//cout << "decrypted:" << endl;
		/*for (int i = 0; i < ret; i++)
		{
			cout << (int)out2[i] << ",";
		}*/
		//cout << endl
		//	 << "decrypted over" << endl;

		//mp_int f;
		//mp_init(&f);
		//mp_set_int(&f, 1);
		mp_int c2;
		mp_init(&c2);
		mp_read_unsigned_bin(&c2, out2, ret);
		mp_int m;
		mp_init(&m);
		mp_read_unsigned_bin(&m, msg, sizeof(msg));
		mp_int x, y;
		mp_init(&x);
		mp_init(&y);

		mp_int d2;mp_init(&d2);mp_2expt(&d2,i);
		mp_exptmod(&c,&d2,&enc.n,&ct);
		mp_mulmod(&c2, &ct, &enc.n, &x);
		mp_copy(&m, &y);
		int flag = mp_cmp(&x, &y);
		if (flag == MP_EQ)
		{
			printf("find bit success pos: %d bit: %d\n",i,1);
			set_bit(&result,i,1);
			print_mp(&result);
			count++;
		}
		else
		{
			mp_mulmod(&m, &ct, &enc.n, &y);
			mp_copy(&c2, &x);
			flag = mp_cmp(&x, &y);
			if (flag == MP_EQ)
			{
				printf("find bit success pos: %d bit: %d\n", i, 0);
				set_bit(&result,i,0);
				print_mp(&result);
				count++;
			}
			else
			{
				printf("find bit failed,index %d\n",i);
				/*printf("x：\n");
				byte out5[256];
				memset(out, 0, sizeof(out5));
				mp_to_unsigned_bin(&x, out5);
				print_x(out5, mp_bitsused(&x) / 8);

				printf("y: \n");
				memset(out, 0, sizeof(out5));
				mp_to_unsigned_bin(&y, out5);
				print_x(out5, mp_bitsused(&y) / 8);*/
			}
		}
		cout<<endl;
	}
	printf("recover %d bits of sk\n",count);
	//cout<<"tp"<<tp<<endl;
	//bit flip
	
	//printf("cipher: \n");
	//print_x(out,len);

	//ret= wc_RsaSSL_Verify(out,len,out2,sizeof(out2),&enc);
	     
	
	

	//wc_FreeRng(&rng);
	wc_FreeRsaKey(&enc);
	wc_FreeRsaKey(&enc2);
	return 0;
}
#endif 