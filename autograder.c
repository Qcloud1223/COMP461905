/*
 *  program to check your source file, run your loader, and compare the output
 *  note that this program may change without notification
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "autograder.h"

void hash_pretty_print(char *buffer, const char *hash)
{
    for (int i = 0; i < HASH_DIGEST_LENGTH; i++)
    {
        sprintf(buffer + i*2, "%02x", hash[i]);
    }
    buffer[HASH_DIGEST_LENGTH*2] = 0;
}

void check_main_hash()
{
    // make sure you did not modify main to spoof an output
    FILE *f = fopen("./test.c", "r");
    if (!f)
    {
        fprintf(stderr, "Source file not found!\n");
        exit(-1);
    }
    unsigned char *real_hash = "ff132e5fff0eff29507b291c10ff342f42ffff3952361effffffff41ff14ffff";
    unsigned char hash[HASH_DIGEST_LENGTH];
    unsigned char hex_hash[HASH_DIGEST_LENGTH*2+1];
    HASH_CTX ctx;
    HASH_Init (&ctx);
    char line[MAX_LINE_LEN];
    while (fgets(line, MAX_LINE_LEN, f))
    {
        HASH_Update (&ctx, line, strlen(line));
    }
    HASH_Final (hash, &ctx);
    hash_pretty_print(hex_hash, hash);

    if (strncmp(real_hash, hex_hash, HASH_DIGEST_LENGTH*2))
    {
        fprintf(stderr, "Real hash: %s\nYour hash: %s\nmain tainted!\n", 
            real_hash, hex_hash);
        exit(-1);
    }
}

int main()
{
    check_main_hash();
    printf("Sanity test...passed\n");
    
    return 0;
}