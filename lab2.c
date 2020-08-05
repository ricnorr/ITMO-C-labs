#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct person {
	char surname[21];
	char name[21];
	char patr[21];
	int64_t phone;
} person;

typedef struct array {
	person* data;
	size_t size;
} arrOfPersons;

int compare(person x, person y)
{
	int res = strcmp(x.surname, y.surname);
	if (res != 0)
	{
		return res;
	}
	res = strcmp(x.name, y.name);
	if (res != 0)
	{
		return res;
	}
	res = strcmp(x.patr, y.patr);
	if (res != 0)
	{
		return res;
	}
	if (x.phone == y.phone)
	{
		return 0;
	}
	return (x.phone > y.phone) ? 1 : -1;
}

typedef struct indexes {
	size_t n;
	size_t m;
} indexes;


indexes advancedSplit(size_t l, size_t r, person* x)
{
	person piv = x[(l + r) / 2];
	indexes ind = { l, l };
	for (size_t j = l; j < r; j++)
	{
		if (compare(x[j], piv) < 0) // x[j] < piv
		{
			person temp = x[j];
			x[j] = x[ind.m];
			x[ind.m] = x[ind.n];
			x[ind.n] = temp;
			ind.m++;
			ind.n++;
		}
		else if (compare(x[j], piv) == 0) //x[j] = piv
		{
			person temp = x[j];
			x[j] = x[ind.m];
			x[ind.m] = temp;
			ind.m++;
		}
	}
	return ind;
}

void advancedQuicksort(size_t l, size_t r, person* x)
{
	bgn:if (r <= 1 + l)
	{
		return;
	}
	indexes ind = advancedSplit(l, r, x);
	if (ind.n - l < r - ind.m)
	{
		advancedQuicksort(l, ind.n, x);
		l = ind.m;
		goto bgn;
	}
	else
	{
		advancedQuicksort(ind.m, r, x);
		r = ind.n;
		goto bgn;
	}
}


// NULL - malloc failed
int readPersons(char * name, arrOfPersons * personArr)
{
	size_t size = 10;
	size_t pos = 0;
	
	FILE* f = fopen(name, "r");
	if (f == NULL)
	{
		return 2;
	}
	
	personArr->data = malloc(size * sizeof(person));
	char* source = malloc(110 * sizeof(char)); //10 ^ 11 + 100
	
	if (!source || !personArr->data)
	{
		return 1;
	}

	while (fgets(source, 110, f) != NULL)
	{
		if (pos >= size - 1)
		{
			size *= 2;
			person* tmp = realloc(personArr->data, sizeof(person) * size);
			if (tmp)
			{
				personArr->data = tmp;
			}
			else
			{
				free(personArr->data);
				free(personArr);
				free(source);
				return 1;
			}
		}
		sscanf(source, "%s %s %s %I64d", personArr->data[pos].surname, personArr->data[pos].name, personArr->data[pos].patr, &(personArr->data[pos].phone));
		pos++;
	}

	free(source);
	personArr->size = pos;
	fclose(f);
	return 0;
}


int fileWriter(char* name, person* data, size_t size)
{
	FILE* f = fopen(name, "w");
	if (f == NULL)
	{
		return 1;
	}
	for (size_t i = 0; i < size; i++)
	{
		fprintf(f, "%s %s %s %I64d\n", data[i].surname, data[i].name, data[i].patr, data[i].phone);
	}
	fclose(f);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Error. Expected 2 arguments. <input file name> <output file name>");
		return 3;
	}

	arrOfPersons* arr = malloc(sizeof(arrOfPersons));
	if (!arr) 
	{
		printf("Error. Not enough memory to store line in file");
		return 1;
	}
	
	int returnCode = readPersons(argv[1], arr);
	if (returnCode == 1)
	{
		printf("Error. Not enough memory to store line in file");
		return returnCode;
	}
	if (returnCode == 2)
	{
		printf("Error. Cannot open input file");
		return returnCode;
	}

	advancedQuicksort(0, arr->size, arr->data);
	
    returnCode = fileWriter(argv[2], arr->data, arr->size);
	if (returnCode)
	{
		printf("Error. Cannot open output file");
		return returnCode;
	}

	free(arr->data);
	free(arr);
	return 0;
}