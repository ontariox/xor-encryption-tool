#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#define MAX_KEY_LEN 50
#define MAX_TEXT    1048576
#define ASCII_START 32
#define ASCII_END   126

typedef struct {
    unsigned char key;
    int score;
} KeyCandidate;


int reduce_key_length(const unsigned char* key, int key_len);


int language_score(const unsigned char* data, int len);

double index_of_coincidence(const unsigned char* data, int len);

void xor_crypt(
    const unsigned char* in,
    unsigned char* out,
    int len,
    const unsigned char* key,
    int key_len
);

unsigned char guess_key_byte(
    const unsigned char* cipher,
    int len,
    int key_len,
    int pos
);

void recover_key(
    const unsigned char* cipher,
    int len,
    unsigned char* key,
    int key_len
);


unsigned char input[MAX_TEXT];
unsigned char output[MAX_TEXT];
unsigned char crack_buffer[MAX_TEXT];



int language_score(const unsigned char* data, int len) {

    int score = 0;
    int letters_en = 0;
    int letters_ru = 0;
    int spaces = 0;
    int bad = 0;

    for (int i = 0; i < len; i++) {

        unsigned char c = data[i];

        if (c == ' ') {
            spaces++;
            score += 10;
        }

        else if (c >= 'a' && c <= 'z') {
            letters_en++;
            score += 5;
        }

        else if (c >= 'A' && c <= 'Z') {
            letters_en++;
            score += 4;
        }

        else if (c >= 0xC0 && c <= 0xFF) {
            letters_ru++;
            score += 5;
        }

        else if (c >= 32 && c <= 126) {
            score += 1;
        }
        else if (c == '\n' || c == '\r') {

        }


        else {
            bad++;
            score -= 25;
        }
    }

    int total_letters = letters_en + letters_ru;

    if (total_letters > len / 5) {

        if (letters_ru > letters_en) {
            for (int i = 0; i < len; i++) {
                unsigned char c = data[i];
                if (c == 0xEE || c == 0xE5 || c == 0xE0 ||
                    c == 0xE8 || c == 0xED || c == 0xF2)
                    score += 3;
            }
        }
        else {
            for (int i = 0; i < len; i++) {
                unsigned char c = data[i];
                if (c == 'e' || c == 't' || c == 'a' ||
                    c == 'o' || c == 'n')
                    score += 3;
            }
        }
    }

    if (bad > len / 10)
        score -= 500;

    if (spaces < len / 25)
        score -= 200;

    return score;
}




void xor_crypt(
    const unsigned char* in,
    unsigned char* out,
    int len,
    const unsigned char* key,
    int key_len
) {
    for (int i = 0; i < len; i++)
        out[i] = in[i] ^ key[i % key_len];
}

double index_of_coincidence(const unsigned char* data, int len) {
    int freq[256] = { 0 };
    double ic = 0.0;

    if (len < 2) return 0.0;

    for (int i = 0; i < len; i++)
        freq[data[i]]++;

    for (int i = 0; i < 256; i++)
        ic += freq[i] * (freq[i] - 1);

    return ic / (double)(len * (len - 1));
}

int guess_key_length(const unsigned char* cipher, int len) {

    double ic_vals[MAX_KEY_LEN + 1] = { 0 };
    int k, i, j;

    printf("IC statistics:\n");

    for (k = 1; k <= MAX_KEY_LEN; k++) {

        double ic_sum = 0.0;

        for (i = 0; i < k; i++) {

            unsigned char block[4096];
            int blen = 0;

            for (j = i; j < len && blen < 4096; j += k)
                block[blen++] = cipher[j];

            ic_sum += index_of_coincidence(block, blen);
        }

        ic_vals[k] = ic_sum / k;
        printf("Key length %2d -> IC = %.5f\n", k, ic_vals[k]);
    }

    double max_ic = 0.0;
    for (k = 1; k <= MAX_KEY_LEN; k++)
        if (ic_vals[k] > max_ic)
            max_ic = ic_vals[k];

    printf("\nMax IC = %.5f\n", max_ic);

    double threshold = max_ic * 0.95;

    printf("Threshold (95%% of max) = %.5f\n", threshold);

    for (k = 1; k <= MAX_KEY_LEN; k++) {
        if (ic_vals[k] >= threshold) {
            printf("Selected key length: %d\n\n", k);
            return k;
        }
    }

    return 1;
}

void guess_key_byte_top2(
    const unsigned char* cipher,
    int len,
    int key_len,
    int pos,
    KeyCandidate* best1,
    KeyCandidate* best2
) {
    best1->score = -100000000;
    best2->score = -100000000;

    for (int k = 0; k < 256; k++) {

        unsigned char decoded[4096];
        int dlen = 0;

        for (int i = pos; i < len && dlen < 4096; i += key_len)
            decoded[dlen++] = cipher[i] ^ (unsigned char)k;

        int score = language_score(decoded, dlen);

        if (score > best1->score) {
            *best2 = *best1;
            best1->key = (unsigned char)k;
            best1->score = score;
        }
        else if (score > best2->score) {
            best2->key = (unsigned char)k;
            best2->score = score;
        }
    }
}


void recover_key(
    const unsigned char* cipher,
    int len,
    unsigned char* key,
    int key_len
) {
    KeyCandidate best[MAX_KEY_LEN];
    KeyCandidate alt[MAX_KEY_LEN];

    for (int i = 0; i < key_len; i++)
        guess_key_byte_top2(cipher, len, key_len, i, &best[i], &alt[i]);

    for (int i = 0; i < key_len; i++)
        key[i] = best[i].key;

    unsigned char test_key[MAX_KEY_LEN];
    memset(crack_buffer, 0, MAX_TEXT);
    unsigned char* decoded = crack_buffer;

    xor_crypt(cipher, decoded, len, key, key_len);
    int best_score = language_score(decoded, len);


    for (int i = 0; i < key_len; i++) {

        memset(test_key, 0, MAX_KEY_LEN);
        memcpy(test_key, key, key_len);
        test_key[i] = alt[i].key;

        xor_crypt(cipher, decoded, len, test_key, key_len);
        int score = language_score(decoded, len);

        if (score > best_score) {
            best_score = score;
            key[i] = alt[i].key;
        }
    }
}


int reduce_key_length(const unsigned char* key, int key_len) {
    for (int m = 1; m <= key_len / 2; m++) {
        if (key_len % m != 0) continue;

        int good_blocks = 0;
        double avg_ratio = 0.0;
        int total_blocks = 0;
        int perfect_blocks = 0;

        for (int r = 0; r < m; r++) {
            int freq[256] = { 0 };
            int count = 0;

            for (int i = r; i < key_len; i += m) {
                freq[key[i]]++;
                count++;
            }

            int max_freq = 0;
            for (int b = 0; b < 256; b++)
                if (freq[b] > max_freq) max_freq = freq[b];

            double ratio = count ? (double)max_freq / count : 0;
            avg_ratio += ratio;
            total_blocks++;

            if (ratio >= 0.9) perfect_blocks++;
            else if (ratio >= 0.6) good_blocks++;
        }

        avg_ratio /= total_blocks;
        double good_pct = (double)(good_blocks + perfect_blocks) / total_blocks;

        if (good_pct >= 0.8 || avg_ratio >= 0.75 || (perfect_blocks > 0 && avg_ratio >= 0.7)) {
            printf("Reduced %d -> %d (%.1f%% good blocks)\n", key_len, m, good_pct * 100);
            return m;
        }
    }
    return key_len;
}






int main(void) {


    setlocale(LC_ALL, " ");
    FILE* fin, * fout;
    unsigned char key[MAX_KEY_LEN];
    int mode;
    long len;

    memset(input, 0, MAX_TEXT);
    memset(output, 0, MAX_TEXT);
    memset(crack_buffer, 0, MAX_TEXT);
    memset(key, 0, MAX_KEY_LEN);

    printf("Select mode:\n1 - encrypt\n2 - decrypt\n3 - crack\n> ");
    scanf("%d", &mode);
    getchar();

    fin = fopen("input.txt", "rb");
    if (!fin) { perror("input.txt"); return 1; }

    fseek(fin, 0, SEEK_END);
    len = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if (len <= 0 || len > MAX_TEXT) { printf("Invalid input size\n"); fclose(fin); return 1; }

    fread(input, 1, len, fin);
    fclose(fin);

    if (mode == 1 || mode == 2) {
        int key_len;
        printf("Enter key: ");
        fgets((char*)key, MAX_KEY_LEN, stdin);
        key_len = strlen((char*)key);
        if (key[key_len - 1] == '\n') key[--key_len] = '\0';
        xor_crypt(input, output, len, key, key_len);
    }
    else if (mode == 3) {
        int key_len = guess_key_length(input, len);
        recover_key(input, len, key, key_len);

        int reduced_len = reduce_key_length(key, key_len);
        if (reduced_len != key_len) {
            printf("Key reduced from %d to %d\n", key_len, reduced_len);
            key_len = reduced_len;
            recover_key(input, len, key, key_len);  
        }

        printf("Final key length: %d\n", key_len);
       

        printf("Key length: %d\n", key_len);
        printf("Key (hex): ");
        for (int i = 0; i < key_len; i++)
            printf("%02X ", key[i]);
        printf("\n");
        printf("Key: ");
        for (int i = 0; i < key_len; i++) {

            unsigned char c = key[i];

            if ((c >= 32 && c <= 126) || (c >= 0xC0 && c <= 0xFF))
                printf("%c", c);
            else
                printf(".");
        }
        printf("\n");


        xor_crypt(input, output, len, key, key_len);
    }
    else {
        printf("Unknown mode\n");
        return 1;
    }

    fout = fopen("output.txt", "wb");
    fwrite(output, 1, len, fout);
    fclose(fout);

    printf("Done\n");
    return 0;
}
