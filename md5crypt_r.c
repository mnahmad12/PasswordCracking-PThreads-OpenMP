// Adaptation of md5crypt() function from OpenSSL file
// openssl/apps/passwd.c to use a provided buffer space for thread
// safety rather than using a static buffer which is not thread
// safe. Creates a dependency on the ssl and crypto libraries
// available on most unix systems. Pass
//
// gcc ... -lssl -lcrypto to enable compilation
// 
// Source for OpenSSL: https://www.openssl.org/source/gitrepo.html

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#include <crack.h>

// Table used by md5 functions during encryption
static unsigned const char cov_2char[64] = {
    0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,
    0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A
};

// Utility for md5crypt_r
size_t OPENSSL_strlcpy(char *dst, const char *src, size_t size)
{
    size_t l = 0;
    for (; size > 1 && *src; size--) {
        *dst++ = *src++;
        l++;
    }
    if (size)
        *dst = '\0';
    return l + strlen(src);
}

// Utility for md5crypt_r
size_t OPENSSL_strlcat(char *dst, const char *src, size_t size)
{
    size_t l = 0;
    for (; size > 0 && *dst; size--, dst++)
        l++;
    return l + OPENSSL_strlcpy(dst, src, size);
}

// #define MD5CRYPT_SIZE (6 + 9 + 24 + 2)


// Thread-safe version of MD5-based password algorithm. For magic
// string "1", this should be compatible to the MD5-based BSD password
// algorithm. For 'magic' string "apr1", this is compatible to the
// MD5-based Apache password algorithm. (Apparently, the Apache
// password algorithm is identical except that the 'magic' string was
// changed -- the laziest application of the NIH principle I've ever
// encountered.) Uses an out_buf parameter to store results to allow
// multiple threads to execute it simultaneously.
int md5crypt_r(const char *passwd, const char *magic, const char *salt, char *out_buf)
{
    /* "$apr1$..salt..$.......md5hash..........\0" */
    unsigned char buf[MD5_DIGEST_LENGTH];
    char *salt_out;
    int n;
    unsigned int i;
    EVP_MD_CTX *md, *md2;
    size_t passwd_len, salt_len;

    passwd_len = strlen(passwd);
    out_buf[0] = '$';
    out_buf[1] = 0;
    assert(strlen(magic) <= 4); /* "1" or "apr1" */
    OPENSSL_strlcat(out_buf, magic, MD5CRYPT_SIZE);
    OPENSSL_strlcat(out_buf, "$", MD5CRYPT_SIZE);
    OPENSSL_strlcat(out_buf, salt, MD5CRYPT_SIZE);
    assert(strlen(out_buf) <= 6 + 8); /* "$apr1$..salt.." */
    salt_out = out_buf + 2 + strlen(magic);
    salt_len = strlen(salt_out);
    assert(salt_len <= 8);

    md = EVP_MD_CTX_create();
    if (md == NULL)
        return 0;
    EVP_DigestInit_ex(md, EVP_md5(), NULL);
    EVP_DigestUpdate(md, passwd, passwd_len);
    EVP_DigestUpdate(md, "$", 1);
    EVP_DigestUpdate(md, magic, strlen(magic));
    EVP_DigestUpdate(md, "$", 1);
    EVP_DigestUpdate(md, salt_out, salt_len);

    md2 = EVP_MD_CTX_create();
    if (md2 == NULL)
        return 0;
    EVP_DigestInit_ex(md2, EVP_md5(), NULL);
    EVP_DigestUpdate(md2, passwd, passwd_len);
    EVP_DigestUpdate(md2, salt_out, salt_len);
    EVP_DigestUpdate(md2, passwd, passwd_len);
    EVP_DigestFinal_ex(md2, buf, NULL);

    for (i = passwd_len; i > sizeof buf; i -= sizeof buf)
        EVP_DigestUpdate(md, buf, sizeof buf);
    EVP_DigestUpdate(md, buf, i);

    n = passwd_len;
    while (n) {
        EVP_DigestUpdate(md, (n & 1) ? "\0" : passwd, 1);
        n >>= 1;
    }
    EVP_DigestFinal_ex(md, buf, NULL);

    for (i = 0; i < 1000; i++) {
        EVP_DigestInit_ex(md2, EVP_md5(), NULL);
        EVP_DigestUpdate(md2, (i & 1) ? (unsigned const char *)passwd : buf,
                         (i & 1) ? passwd_len : sizeof buf);
        if (i % 3)
            EVP_DigestUpdate(md2, salt_out, salt_len);
        if (i % 7)
            EVP_DigestUpdate(md2, passwd, passwd_len);
        EVP_DigestUpdate(md2, (i & 1) ? buf : (unsigned const char *)passwd,
                         (i & 1) ? sizeof buf : passwd_len);
        EVP_DigestFinal_ex(md2, buf, NULL);
    }
    EVP_MD_CTX_destroy(md2);
    EVP_MD_CTX_destroy(md);

    {
        /* transform buf into output string */

        unsigned char buf_perm[sizeof buf];
        int dest, source;
        char *output;

        /* silly output permutation */
        for (dest = 0, source = 0; dest < 14;
             dest++, source = (source + 6) % 17)
            buf_perm[dest] = buf[source];
        buf_perm[14] = buf[5];
        buf_perm[15] = buf[11];
#  ifndef PEDANTIC              /* Unfortunately, this generates a "no
                                 * effect" warning */
        assert(16 == sizeof buf_perm);
#  endif

        output = salt_out + salt_len;
        assert(output == out_buf + strlen(out_buf));

        *output++ = '$';

        for (i = 0; i < 15; i += 3) {
            *output++ = cov_2char[buf_perm[i + 2] & 0x3f];
            *output++ = cov_2char[((buf_perm[i + 1] & 0xf) << 2) |
                                  (buf_perm[i + 2] >> 6)];
            *output++ = cov_2char[((buf_perm[i] & 3) << 4) |
                                  (buf_perm[i + 1] >> 4)];
            *output++ = cov_2char[buf_perm[i] >> 2];
        }
        assert(i == 15);
        *output++ = cov_2char[buf_perm[i] & 0x3f];
        *output++ = cov_2char[buf_perm[i] >> 6];
        *output = 0;
        assert(strlen(out_buf) < MD5CRYPT_SIZE);
    }

    return 1;
}
