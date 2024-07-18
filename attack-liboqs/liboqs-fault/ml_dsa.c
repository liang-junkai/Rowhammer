/*
 * example_sig.c
 *
 * Minimal example of using a post-quantum signature implemented in liboqs.
 *
*/
 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <oqs/oqs.h>

#define MESSAGE_LEN 4000

/* Cleaning up memory etc */
void cleanup_stack(uint8_t *secret_key, size_t secret_key_len);

void cleanup_heap(uint8_t *public_key, uint8_t *secret_key,
                  uint8_t *message, uint8_t *signature,
                  OQS_SIG *sig);
    



void write_x(uint8_t *cipher, int len, FILE* fp3)
{   
for (int i = 0; i < len; i++)
    {
        fprintf(fp3,"%02X", cipher[i]);
    }
   fprintf(fp3,"\n");
   //fclose(fp3);
}
void print_x(uint8_t *sig, int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%02X", sig[i]);
    }
    printf("\n");
}
void read_from_hex(int8_t *target, int num, char *hex){
    for (int i = 0; i < num; i++) {
        sscanf(hex + 2 * i, "%2hhx", &target[i]);
    }
}
void bit_flip(uint8_t *target,int i){
    int pos=i/(sizeof(uint8_t)*8);
    int num=i%(sizeof(uint8_t)*8);
    //printf("pos,num: %d %d\n",pos,num);
    int tmp=1;
    for(int k=0;k<num;k++){
        tmp=tmp*2;
    }
    target[pos]=target[pos]^tmp;
}
static OQS_STATUS example_stack(void) {

#ifdef OQS_ENABLE_SIG_ml_dsa_44_ipd

    FILE* fp3=fopen("sign.txt","a+"); 
	OQS_STATUS rc;

	uint8_t public_key[OQS_SIG_ml_dsa_44_ipd_length_public_key];
	uint8_t secret_key[OQS_SIG_ml_dsa_44_ipd_length_secret_key];
	uint8_t message[5]={0,1,2,3,4};
	uint8_t signature[OQS_SIG_ml_dsa_44_ipd_length_signature];
	size_t message_len = MESSAGE_LEN;
	size_t signature_len;
    FILE *fp = fopen("./key.txt", "r");
    
    char pstr[2*OQS_SIG_ml_dsa_44_ipd_length_public_key+10];
    char sstr[2*OQS_SIG_ml_dsa_44_ipd_length_secret_key+10];
    
    fgets(pstr, 2*OQS_SIG_ml_dsa_44_ipd_length_public_key+10, fp);
    fgets(sstr,2*OQS_SIG_ml_dsa_44_ipd_length_secret_key+10,fp);
    
    read_from_hex(public_key,OQS_SIG_ml_dsa_44_ipd_length_public_key,pstr);
    read_from_hex(secret_key,OQS_SIG_ml_dsa_44_ipd_length_secret_key,sstr);

    for(int i=0;i<message_len;i++){
        message[i]=i;
        message2[i]=i;
    }
	// let's create a random test message to sign
	//OQS_randombytes(message, message_len);

	//rc = OQS_SIG_ml_dsa_44_ipd_keypair(public_key, secret_key);
    // printf("public:\n");
    // print_x(public_key,OQS_SIG_ml_dsa_44_ipd_length_public_key);
    // printf("private:\n");
    // print_x(secret_key,OQS_SIG_ml_dsa_44_ipd_length_secret_key);

	// if (rc != OQS_SUCCESS) {
	// 	fprintf(stderr, "ERROR: OQS_ml_DSA_44_keypair failed!\n");
	// 	cleanup_stack(secret_key, OQS_SIG_ml_dsa_44_ipd_length_secret_key);
	// 	return OQS_ERROR;
	// }
    
	rc = OQS_SIG_ml_dsa_44_ipd_sign(signature, &signature_len, message2, message_len, secret_key);
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_ml_DSA_44_sign failed!\n");
		cleanup_stack(secret_key, OQS_SIG_ml_dsa_44_ipd_length_secret_key);
		return OQS_ERROR;
	}
    fprintf(fp3,"signature\n");
    write_x(signature,OQS_SIG_ml_dsa_44_ipd_length_signature,fp3);

	rc = OQS_SIG_ml_dsa_44_ipd_verify(message, message_len, signature, signature_len, public_key);
	if (rc != OQS_SUCCESS) {
		fprintf(fp3, "fault detected\n");
		cleanup_stack(secret_key, OQS_SIG_ml_dsa_44_ipd_length_secret_key);
		return OQS_ERROR;
	}
    fprintf(fp3,"ML-DSA-sign-44 verify pass\n");
	cleanup_stack(secret_key, OQS_SIG_ml_dsa_44_ipd_length_secret_key);
	return OQS_SUCCESS; // success!

#else

	printf("[example_stack] OQS_SIG_dilithium_2 was not enabled at compile-time.\n");
	return OQS_ERROR;

#endif
}


int main(void) {
	if (example_stack() == OQS_SUCCESS) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

void cleanup_stack(uint8_t *secret_key, size_t secret_key_len) {
	OQS_MEM_cleanse(secret_key, secret_key_len);
}

