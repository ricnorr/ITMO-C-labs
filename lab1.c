#define _CRT_SECURE_NO_WARNINGS
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

//mode
//0 - UTF-8 BE
//1 - UTF-8 LE
//2 - UTF-16 BE
//3 - UTF-16 LE
//4 - UTF-32 BE
//5 - UTF-32 LE

typedef struct arr
{
	unsigned char d[4];
	int size;
} binCluster;


typedef struct fileBinaryStr
{
	int codeRes;
	unsigned char* d;
	size_t size;
	size_t pos;
	int mode;
} fileBinaryStr;



//80 - 10000000
//c0 - 11000000
//e0 - 11100000
//f0 - 11110000

binCluster reverseStruct(binCluster p) //for LE
{
	for (int i = 0; i < p.size - 1; i += 2)
	{
		unsigned char tmp = p.d[i];
		p.d[i] = p.d[i + 1];
		p.d[i + 1] = tmp;
	}
	return p;
}


int hasNextChar(fileBinaryStr* p)
{
	switch (p->mode)
	{
	case(1):
	case(0):
		return (p->pos < p->size) ? 1 : 0;
	case(2):
	case(3):
		return (p->pos + 1 < p->size) ? 1 : 0;
	case(4):
	case(5):
		return (p->pos + 3 < p->size) ? 1 : 0;
	}
}


binCluster checkUTF8Bytes(binCluster x) { // check they're correct
	binCluster res = { {0xEF, 0xBF, 0xBD}, 3 };
	for (int i = 1; i < x.size; i++)
	{
		if ((x.d[i] >> 6) != 0b10)
		{
			return res;
		}
	}
	return x;
}

binCluster nextUTF8Char(fileBinaryStr* p) {
	binCluster res = { {0xEF, 0xBF, 0xBD}, 3 };
	if ((p->d[p->pos] >> 3 == 0b11110) && (p->pos <= p->size - 4))
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.d[2] = p->d[p->pos + 2];
		res.d[3] = p->d[p->pos + 3];
		res.size = 4;
		p->pos += 4;
	}
	else if ((p->d[p->pos] >> 4 == 0b1110) && (p->pos <= p->size - 3))
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.d[2] = p->d[p->pos + 2];
		res.size = 3;
		p->pos += 3;
	}
	else if ((p->d[p->pos] >> 5 == 0b110) && (p->pos <= p->size - 2))
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.size = 2;
		p->pos += 2;
	}
	else if (p->d[p->pos] >> 7 == 0)
	{
		res.d[0] = p->d[p->pos];
		res.size = 1;
		p->pos++;
	} else {
		p->pos++;
	}
	return checkUTF8Bytes(res);
}


binCluster nextUtf16Char(fileBinaryStr* p)
{
	binCluster res = { {0xFF, 0xDD}, 2 };
	uint32_t x = 0;
	if (p->mode % 2 == 0)
	{
		x = (p->d[p->pos] << 8) + p->d[p->pos + 1];
	}
	else
	{
		x = ((p->d[p->pos + 1] << 8) + p->d[p->pos]);
	}
	if (x <= 0xD7FF || (0xE000 <= x && x <= 0xFFFF)) // one utf16 word
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.size = 2;
		p->pos += 2;
	}
	else if ((0xD800 <= x && x <= 0xDBFF && (p->pos + 4 <= p->size))) // two utf16 words
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.d[2] = p->d[p->pos + 2];
		res.d[3] = p->d[p->pos + 3];
		res.size = 4;
		p->pos += 4;
	}
	else
	{
		p->pos ++; // skip byte
	}

	if (p->mode % 2 == 1) // LE
	{
		unsigned char tmp = res.d[0];
		res.d[0] = res.d[1];
		res.d[1] = tmp;
		tmp = res.d[3];
		res.d[3] = res.d[2];
		res.d[2] = tmp;
	}
	return res;
}

binCluster nextUtf32Char(fileBinaryStr* p)
{
	binCluster res = { {0, 0, 0xFF, 0xFD},0 };
	if (p->mode % 2 == 0)
	{
		res.d[0] = p->d[p->pos];
		res.d[1] = p->d[p->pos + 1];
		res.d[2] = p->d[p->pos + 2];
		res.d[3] = p->d[p->pos + 3];
	}
	else
	{
		res.d[0] = p->d[p->pos + 1];
		res.d[1] = p->d[p->pos];
		res.d[2] = p->d[p->pos + 3];
		res.d[3] = p->d[p->pos + 2];
	}
	res.size = 4;
	p->pos += 4;
	return res;

}

binCluster nextChar(fileBinaryStr* p)
{
	if (p->mode == 0 || p->mode == 1)
	{
		return nextUTF8Char(p);
	}
	if (p->mode == 2 || p->mode == 3)
	{
		return nextUtf16Char(p);
	}
	return nextUtf32Char(p);
}


uint32_t utf8ToUnicode(binCluster p)
{
	uint32_t res = 0xFFFD; // Unknown char
	if (p.size == 1)
	{
		res = (p.d[0] & 0b1111111);
	}
	else if (p.size == 2)
	{
		res = (p.d[0] & 0b11111) << 6;
		res += p.d[1] & 0b111111;
	}
	else if (p.size == 3)
	{
		res = (p.d[0] & 0b1111) << 6;
		res += p.d[1] & 0b111111;
		res = res << 6;
		res += p.d[2] & 0b111111;
	}
	else if (p.size == 4)
	{
		res = (p.d[0] & 0b111) << 6;
		res += (p.d[1] & 0b111111);
		res = res << 6;
		res += p.d[2] & 0b111111;
		res = res << 6;
		res += p.d[3] & 0b111111;

	}
	return res;
}


uint32_t utf32ToUnicode(binCluster p)
{
	return (p.d[0] << 24) + (p.d[1] << 16) + (p.d[2] << 8) + (p.d[3]);
}

binCluster unicodeToUtf32(uint32_t x)
{
	binCluster  res = { {0}, 0 };
	res.d[0] = (x >> 24)& (0xFF);
	res.d[1] = (x >> 16)& (0xFF);
	res.d[2] = (x >> 8)& (0xFF);
	res.d[3] = x & 0xFF;
	res.size = 4;
	return res;
}

fileBinaryStr* readFileBytes(const char* name)
{
	fileBinaryStr* tmp = malloc(sizeof(fileBinaryStr));
	FILE* f;
	if ((f = fopen(name, "rb")) == NULL)
	{
		tmp->codeRes = 2; // fileNotFound
		return tmp;
	}
	tmp = malloc(sizeof(fileBinaryStr));
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	unsigned char* x = malloc(size * sizeof(char));
	if (x == NULL || tmp == NULL)
	{
		return NULL;
	}
	rewind(f);
	fread(x, size, 1, f);

	tmp->codeRes = 0;
	tmp->d = x;
	tmp->size = size;
	tmp->pos = 0;
	tmp->mode = -1;


	fclose(f);
	return tmp;
}

int bitCount(uint32_t x)
{
	int cnt = 0;
	uint32_t tmp = x;
	while (tmp > 0)
	{
		cnt += 1;
		tmp = tmp / 2;
	}
	return cnt;
}

binCluster unicodeToUtf8(uint32_t x)
{
	binCluster res = { {0xEF,0xBF,0xBD},3 };
	int cnt = bitCount(x);
	uint32_t tmp = x;
	if (cnt <= 7)
	{
		res.size = 1;
		res.d[0] = x & 0b1111111;
	}
	else if (cnt <= 11)
	{
		res.size = 2;
		res.d[1] = (tmp & 0b111111) + 0b10000000;
		res.d[0] = ((tmp >> 6) & 0b11111) + 0b11000000;
	}
	else if (cnt <= 16)
	{
		res.size = 3;
		res.d[2] = (tmp & 0b111111) + 0b10000000;
		res.d[1] = ((tmp >> 6) & 0b111111) + 0b10000000;
		res.d[0] = ((tmp >> 12) & 0b1111) + 0b11100000;
	}
	else if (cnt <= 21)
	{
		res.size = 4;
		res.d[3] = (tmp & 0b111111) + (1 << 7);
		res.d[2] = ((tmp >> 6) & 0b111111) + 0b10000000;
		res.d[1] = ((tmp >> 12) & 0b111111) + 0b10000000;
		res.d[0] = ((tmp >> 18) & 0b111) + 0b11110000;
	}
	return res;
}


binCluster unicodeToUTF16(unsigned int x)
{
	binCluster res = { {0xFF, 0xDD} , 2 }; //unknown character
	if ((0x10000 <= x) && (x <= 0x10FFFF))
	{
		x -= 0x10000;
		uint32_t young = ((x >> 10) & 0b1111111111) + 0xD800;
		uint32_t old = (x & (0b1111111111)) + 0xDC00;
		uint32_t sur = (young << 16) + old;
		res.d[0] = (sur >> 24) & 0xFF;
		res.d[1] = (sur >> 16) & 0xFF;
		res.d[2] = (sur >> 8) & 0xFF;
		res.d[3] = (sur) & 0xFF;
		res.size = 4;

	}
	else if (x <= 0xD7FF || (0xE000 <= x && x <= 0xFFFF))
	{
		res.d[1] = x & 0xFF;
		res.d[0] = (x >> 8) & 0xFF;
		res.size = 2;
	}
	return res;


}

uint32_t utf16ToUnicode(binCluster p)
{
	uint32_t res = 0xFFDD;
	uint32_t first = (p.d[0] << 8) + p.d[1];
	if (first <= 0xD7FF|| first >= 0xE000)
	{
		return first;
	}
	uint32_t second = (p.d[2] << 8) + p.d[3];
	if (first >= 0xD800 && first <= 0xDBFF && p.size == 2)
	{
		//Unknown character
		return res;
	}

	res = first & 0b1111111111;
	res = res << 10;
	res += second & (0b1111111111);
	return res + 0x10000;
}

int isUtf8Bom(fileBinaryStr* p)
{
	if (p->size > 3 && p->d[0] == 0xEF && p->d[1] == 0xBB && p->d[2] == 0xBF)
	{
		p->pos = 3;
		return 1;
	}
	return 0;
}

int isUtf16BEBom(fileBinaryStr* p)
{
	if (p->size > 1 && p->d[0] == 0xFE && p->d[1] == 0xFF)
	{
		p->pos = 2;
		return 1;
	}
	return 0;
}

int isUtf32BEBom(fileBinaryStr* p)
{
	if (p->size > 3 && p->d[0] == 0x00 && p->d[1] == 0x00 && p->d[2] == 0xFE && p->d[3] == 0xFF)
	{
		p->pos = 4;
		return 1;
	}
	return 0;
}

int isUTF32LEBom(fileBinaryStr* p)
{
	if (p->size > 3 && p->d[0] == 0xFF && p->d[1] == 0xFE && p->d[2] == 0x00 && p->d[3] == 0x00) {
		p->pos = 4;
		return 1;
	}
	return 0;
}

int isUtf16LEBom(fileBinaryStr* p)
{
	if (p->size > 1 && p->d[0] == 0xFF && p->d[1] == 0xFE)
	{
		p->pos = 2;
		return 1;
	}
	return 0;
}

//mode
//0 - UTF-8 BOM
//1 - UTF-8 NO BOM
//2 - UTF-16 BE
//3 - UTF-16 LE
//4 - UTF-32 BE
//5 - UTF-32 LE
void chooseMode(fileBinaryStr* fileStr)
{
	if (isUTF32LEBom(fileStr))
	{
		fileStr->mode = 5;
	}
	else if (isUtf16BEBom(fileStr))
	{
		fileStr->mode = 2;
	}
	else if (isUtf32BEBom(fileStr))
	{
		fileStr->mode = 4;
	}
	else if (isUtf16LEBom(fileStr))
	{
		fileStr->mode = 3;
	}
	else if (isUtf8Bom(fileStr))
	{
		fileStr->mode = 0;
	}
	else
	{
		fileStr->mode = 1;
	}
}

int fileWriter(FILE* f, binCluster src, int outMode)
{
	if (outMode == 3 || outMode == 5)
	{
		src = reverseStruct(src);
	}
	for (int i = 0; i < src.size; i++)
	{
		putc(src.d[i], f);
	}
}

uint32_t toUnicode(fileBinaryStr* fileStr, binCluster arr)
{
	if (fileStr->mode == 0 || fileStr->mode == 1)
	{
		return utf8ToUnicode(arr);
	}
	else if (fileStr->mode == 2 || fileStr->mode == 3)
	{
		return utf16ToUnicode(arr);
	}
	else
	{
		return utf32ToUnicode(arr);
	}
}

binCluster toUTF(int mode, uint32_t code)
{
	if (mode == 0 || mode == 1)
	{
		return unicodeToUtf8(code);
	}
	else if (mode == 2 || mode == 3)
	{
		return unicodeToUTF16(code);
	}
	else
	{
		return unicodeToUtf32(code);
	}
}

void writeBom(int outMode, FILE* f)
{
	if (outMode == 0)
	{
		putc(0xEF, f);
		putc(0xBB, f);
		putc(0xBF, f);
	}
	else if (outMode == 2)
	{
		putc(0xFE, f);
		putc(0xFF, f);
	}
	else if (outMode == 3)
	{
		putc(0xFF, f);
		putc(0xFE, f);
	}
	else if (outMode == 4)
	{
		putc(0x00, f);
		putc(0x00, f);
		putc(0xFE, f);
		putc(0xFF, f);
	}
	else if (outMode == 5)
	{
		putc(0xFF, f);
		putc(0xFE, f);
		putc(0x00, f);
		putc(0x00, f);
	}
}

int main(int argc, char** argv) {
	if (argc != 4)
	{
		printf("Error. Need 3 arguments <input file name> <output file name> <mode>");
		return 4;
	}

	fileBinaryStr* fileStr = readFileBytes(argv[1]);
	switch (fileStr->codeRes)
	{
	case (2):
		printf("Error. Input file not found");
		return 2;
	case (1):
		printf("Error. Not enough memory to work with file");
		return 1;
	}
	chooseMode(fileStr);

	FILE* f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Error. Cannot open output file");
		return 3;
	}
	int outMode = atoi(argv[3]);
	if (outMode == 6)
	{
		return 3;
	}
	writeBom(outMode, f);
	binCluster tmp = { {0}, 0 };
	while (hasNextChar(fileStr))
	{
		tmp = nextChar(fileStr);
		uint32_t code = toUnicode(fileStr, tmp);
		tmp = toUTF(outMode, code);
		fileWriter(f, tmp, outMode);
	}


	fclose(f);
	free(fileStr);
	return 0;
}














