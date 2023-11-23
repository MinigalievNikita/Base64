#include <stdlib.h>
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#define STRINGLENGHT 76

static const BYTE charset[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"}; //алфавит
static const int key = 250; //8 битный ключ

BYTE revchar(char ch) //нормируем чтобы были числа от 0 до 63
{
	if (ch >= 'A' && ch <= 'Z')
		ch -= 'A';
	else if (ch >= 'a' && ch <='z')
		ch = ch - 'a' + 26;
	else if (ch >= '0' && ch <='9')
		ch = ch - '0' + 52;
	else if (ch == '+')
		ch = 62;
	else if (ch == '/')
		ch = 63;

	return(ch);
}

size_t base64_encode(const BYTE in[], BYTE out[], size_t len, int newline_flag) //кодировщик
{
	size_t idx, idx2, blks, blk_ceiling, left_over, string_size = 0;

	blks = (len / 3);
	left_over = len % 3; //нужно для определения добавления символов

	if (out == NULL) {
		idx2 = blks * 4;
		if (left_over)
			idx2 += 4;
		if (newline_flag)
			idx2 += len / 57;   //перевод на новую строку каждые введенные 57 байтов
	}
	else {
		blk_ceiling = blks * 3;
		for (idx = 0, idx2 = 0; idx < blk_ceiling; idx += 3, idx2 += 4) {
			out[idx2]     = charset[in[idx] >> 2];
			out[idx2 + 1] = charset[((in[idx] & 0x03) << 4) | (in[idx + 1] >> 4)];
			out[idx2 + 2] = charset[((in[idx + 1] & 0x0f) << 2) | (in[idx + 2] >> 6)];
			out[idx2 + 3] = charset[in[idx + 2] & 0x3F];
			
			out[idx2]  = out[idx2] ^ key;
			out[idx2 + 1] = out[idx2 + 1] ^ key;
			out[idx2 + 2] = out[idx2 + 2] ^ key;
			out[idx2 + 3] = out[idx2 + 3] ^ key;

			if (((idx2 - string_size + 4) % STRINGLENGHT == 0) && newline_flag) {
				out[idx2 + 4] = '\n'; //по стандарту перевод записи на новую строку с каждого 76 символа
				idx2++;
				string_size++;
			}
		}

		if (left_over == 1) {
			out[idx2]     = charset[in[idx] >> 2];
			out[idx2 + 1] = charset[(in[idx] & 0x03) << 4];
			out[idx2 + 2] = '=';
			out[idx2 + 3] = '=';
            out[idx2]  = out[idx2] ^ key;
			out[idx2 + 1] = out[idx2 + 1] ^ key;
			idx2 += 4;
		}
		else if (left_over == 2) {
			out[idx2]     = charset[in[idx] >> 2];
			out[idx2 + 1] = charset[((in[idx] & 0x03) << 4) | (in[idx + 1] >> 4)];
			out[idx2 + 2] = charset[(in[idx + 1] & 0x0F) << 2];
			out[idx2 + 3] = '=';
            out[idx2]  = out[idx2] ^ key;
			out[idx2 + 1] = out[idx2 + 1] ^ key;
			out[idx2 + 2] = out[idx2 + 2] ^ key;
			idx2 += 4;
		}
	}

	return(idx2);
}

size_t base64_decode(const BYTE in[], BYTE out[], size_t len) //декодер
{
	BYTE ch;
	size_t idx, idx2, blks, blk_ceiling, left_over;

	if (in[len - 1] == '=')
		len--;
	if (in[len - 1] == '=')
		len--;

	blks = len / 4;
	left_over = len % 4;

	if (out == NULL) {
		if (len >= 77 && (in[STRINGLENGHT] == '\n'))
			len -= len / (STRINGLENGHT + 1);
		blks = len / 4;
		left_over = len % 4;

		idx = blks * 3;
		if (left_over == 2)
			idx ++;
		else if (left_over == 3)
			idx += 2;
	}
	else {
		blk_ceiling = blks * 4;
		for (idx = 0, idx2 = 0; idx2 < blk_ceiling; idx += 3, idx2 += 4) {
			if (in[idx2] == '\n')
				idx2++;

			out[idx]     = (revchar((in[idx2]^ key)) << 2) | ((revchar((in[idx2 + 1]^ key)) & 0x30) >> 4);
			out[idx + 1] = (revchar((in[idx2 + 1]^ key)) << 4) | (revchar((in[idx2 + 2]^ key)) >> 2);
			out[idx + 2] = (revchar((in[idx2 + 2]^ key)) << 6) | revchar((in[idx2 + 3]^ key));
		}

		if (left_over == 2) {
			out[idx]     = (revchar((in[idx2]^ key)) << 2) | ((revchar((in[idx2 + 1]^ key)) & 0x30) >> 4);
			idx++;
		}
		else if (left_over == 3) {
			out[idx]  = (revchar((in[idx2]^ key)) << 2) | ((revchar((in[idx2 + 1]^ key)) & 0x30) >> 4);
			out[idx + 1] = (revchar((in[idx2 + 1]^ key)) << 4) | (revchar((in[idx2 + 2]^ key)) >> 2);
			idx += 2;
		}
	}

	return(idx);
}

double base64_test(char *text)
{

	
	BYTE buf[100000];
	BYTE out[100000];
	size_t buf_len;
    double sum_time = 0;
	int iterations = 10;
	for (int i = 0; i < iterations; i++) {
		memset(buf, 0, sizeof(buf)); //очитска памяти переменных от мусора
		memset(out, 0, sizeof(out));

		struct timeval stop, start;
    	gettimeofday(&start, NULL);

		buf_len = base64_encode(text, buf, strlen(text), 1);

		buf_len = base64_decode(buf, out, strlen(buf));

       	gettimeofday(&stop, NULL);
    	double delta = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000.0;
	    sum_time += delta;

		//printf("%s\n", out);

		memset(buf, 0, sizeof(buf));
		memset(out, 0, sizeof(out));
	}
	return (double)(sum_time/iterations);
}

int main()
{
    int limit = 60;
    double times_array[limit];
    for(int j = 1; j < limit; ++j){
        char *text = malloc(100 * 2 * j * sizeof(char) + 10 * sizeof(char));
        for(int i = 0; i < 100 * 2 * j; i+=10){
            text[i] = 't';
            text[i + 1] = 'r';
            text[i + 2] = 'v';
            text[i + 3] = 'a';
            text[i + 4] = 'z';
            text[i + 5] = 's';
            text[i + 6] = 'p';
            text[i + 7] = 'l';
            text[i + 8] = 'q';
            text[i + 9] = 'y';
        }
        times_array[j-1] = base64_test(text);
        printf("time: %lg size: %d\n", times_array[j-1], 100 * 2 * j);
        free(text);
    }
    
	return 0;
}
