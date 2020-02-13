#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define M_VERSION "0.1.0"

static const uint32_t k[64] =
{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const uint32_t r[64] =
{
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

#define LEFT_ROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

void to_bytes(uint32_t val, uint8_t *bytes)
{
    int i, amt;
    for (i = 0, amt = 0; i < 4; ++i)
    {
        bytes[i] = (uint8_t)(val >> amt);
        amt += 8;
    }
}

uint32_t to_int32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 0) |
           ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest)
{
    uint32_t h0, h1, h2, h3;

    uint8_t *msg = NULL;

    size_t new_len, offset;
    uint32_t w[16];
    uint32_t a, b, c, d, i, f, g, temp;

    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;

    new_len = ((((initial_len + 8) / 64) + 1) * 64) - 8;

    if ((msg = malloc(new_len + 8)) == NULL)
        return;

    memcpy(msg, initial_msg, initial_len);

    msg[initial_len] = 0x80;

    for (offset = initial_len + 1; offset < new_len; ++offset)
        msg[offset] = 0;

    to_bytes(initial_len * 8, msg + new_len);
    to_bytes(initial_len >> 29, msg + new_len + 4);

    for (offset = 0; offset < new_len; offset += (512 / 8))
    {
        for (i = 0; i < 16; ++i)
            w[i] = to_int32(msg + offset + i * 4);

        a = h0;
        b = h1;
        c = h2;
        d = h3;

        for (i = 0; i < 64; ++i)
        {
            if (i < 16)
            {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48)
            {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else
            {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }

            temp = d;
            d = c;
            c = b;
            b += LEFT_ROTATE((a + f + k[i] + w[g]), r[i]);
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
    }

    free(msg);

    to_bytes(h0, digest);
    to_bytes(h1, digest + 4);
    to_bytes(h2, digest + 8);
    to_bytes(h3, digest + 12);
}

char *readfile(const char *filename)
{
    char *buffer = NULL;
    size_t string_size, read_size;
    FILE *fp = fopen(filename, "r");

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        string_size = ftell(fp);
        rewind(fp);

        if ((buffer = malloc(string_size + 1)) == NULL)
            return NULL;

        read_size = fread(buffer, 1, string_size, fp);

        buffer[string_size] = '\0';

        if (string_size != read_size)
        {
            free(buffer);
            buffer = NULL;
        }

        fclose(fp);
    }

    return buffer;
}

int main(int argc, char **argv)
{
    char *msg;
    int i, j;
    uint8_t result[16];

    if (argc < 2)
    {
        printf("Usage: %s [-s str] [files]\n", argv[0]);
        return -1;
    }

    for (i = 1; i < argc; ++i)
    {
        if ((msg = readfile(argv[i])) == NULL)
        {
            fprintf(stderr, "Cannot read file '%s'\n", argv[i]);
            continue;
        }

        md5((uint8_t *)msg, strlen(msg), result);

        printf("[%s] = ", argv[i]);
        for (j = 0; j < 16; ++j)
            printf("%2.2x", result[j]);
        puts("");
    }

    free(msg);

    return 0;
}
