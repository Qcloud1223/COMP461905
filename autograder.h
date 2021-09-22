#define HASHFUNC SHA256
#define CAT(a, b) a##_##b
#define SLOW_CAT(a, b) CAT(a, b)
#define HASH_DIGEST_LENGTH SLOW_CAT(HASHFUNC, DIGEST_LENGTH)

#define HASH_Update SLOW_CAT(HASHFUNC, Update)
#define HASH_Init SLOW_CAT(HASHFUNC, Init)
#define HASH_Final SLOW_CAT(HASHFUNC, Final)
#define HASH_CTX SLOW_CAT(HASHFUNC, CTX)

#define MAX_LINE_LEN 256