#include <stdlib.h>
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>



#define STRINGLENGHT 76

static const BYTE charset[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"}; //алфавит
static const int key = 200; //8 битный ключ

int flag = 10 ; //количество изменяемых символов по ошибке

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
			srand(time(NULL));
            int randfactor = rand() % 4;

            out[idx2] = charset[in[idx] >> 2];
			out[idx2 + 1] = charset[((in[idx] & 0x03) << 4) | (in[idx + 1] >> 4)];
			out[idx2 + 2] = charset[((in[idx + 1] & 0x0f) << 2) | (in[idx + 2] >> 6)];
			out[idx2 + 3] = charset[in[idx + 2] & 0x3F];

            if(flag > 0){//изменение байта
                if(out[idx2 + randfactor] % 2 == 0){
                    out[idx2 + randfactor] += 1;
                } else {
                    out[idx2 + randfactor] -= 1;
                }
                flag--;
            }
			
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


int errors_sum = 0;
int size_samples_array = 2000;

void base64_test(char *text)
{

	
	BYTE buf[3000];
	BYTE out[3000];
	size_t buf_len;
    memset(buf, 0, sizeof(buf)); //очитска памяти переменных от мусора
    memset(out, 0, sizeof(out));
    //printf("%s\n", text);
    buf_len = base64_encode(text, buf, strlen(text), 1);

    //printf("%s\n", buf);
    
    buf_len = base64_decode(buf, out, strlen(buf));

    //printf("%s\n", out);
    for(int i = 0; i < size_samples_array; ++i){
        if(text[i] == out[i])
            continue;
        errors_sum++;
    }
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

}

int main(int argc, char*argv[])
{
    //flag = atoi(argv[1]);
    //char text[1000] = {"The most famous part of the Taj Mahal is the large white dome in the centre. It is 35 metres high and is surrounded by four smaller domes. The rooms inside the building are decorated with beautiful archways and precious stones in the walls. The buildings are surrounded by gardens with pathways, pools, fountains and green gardens"};
    for(int i = 0; i < 11; ++i){
        flag = 10 + 50*i;
        errors_sum = 0;
        char *text = malloc(size_samples_array * sizeof(char) + 10 * sizeof(char));
            for(int i = 0; i < size_samples_array; i+=10){
                text[i] = (char)('A' + rand() % 26);
                text[i + 1] = (char)('a' + rand() % 26);
                text[i + 2] = (char)('0' + rand() % 10);
                text[i + 3] = (char)('A' + rand() % 26);
                text[i + 4] = (char)('a' + rand() % 26);
                text[i + 5] = (char)('A' + rand() % 26);
                text[i + 6] = (char)('0' + rand() % 10);
                text[i + 7] = (char)('0' + rand() % 10);
                text[i + 8] = (char)('a' + rand() % 26);
                text[i + 9] = (char)('A' + rand() % 26);
            }
        base64_test(text);
    printf("errors %d bad bits %d \n", errors_sum, 10 + 50*i);
    }
	return 0;
}
