#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <float.h>


typedef struct Matrix
{
    int rk;
    int rkex;
    float * matr;
    int width;
    int height;
} Matrix;


const float eps = 0.000001f;

int equal(float a, float b)
{
    return fabsf(a - b) <= eps ? 1 : 0;
}

void swap(Matrix * this, int i, int j)
{
    for (int k = 0; k < this->width; k++)
	{
        float temp = this->matr[i* this->width + k];
        this->matr[i * this->width + k] = this->matr[j* this->width + k];
        this->matr[j * this->width + k] = temp;
    }
}

float * findSolutions(Matrix * this)
{
    float * ans = malloc(sizeof(int) * this->height);
    for (int i = this->height - 1; i > - 1; i--)
	{
        float temp = 0.0f;
        for (int j = i + 1; j < this->height; j++)
		{
            temp += this->matr[i* this->width + j] * ans[j];
        }
        ans[i] = (this->matr[i* this->width + this->width - 1] - temp) / this->matr[i* this->width + i];
    }
    return ans;
}

void add(Matrix * this, int fixi, int fixj)
{
    for (int i = fixi + 1; i < this->height; i++)
	{
        float k = (this->matr[i* this->width + fixj] / this->matr[fixi* this->width + fixj]);
        for (int j = 0; j < this->width; j++)
		{
            this->matr[i* this->width + j] = this->matr[i* this->width + j] - this->matr[fixi* this->width + j] * k;
        }
    }
}


void getExtendedRank(Matrix * this)
{
    this->rkex = this->rk;
    for (int i = 0; i < this->height - this->rk; i++)
	{
        if (!equal(this->matr[(this->height - 1 - i) * this->width + this->width - 1], 0))
		{
            this->rkex++;
            break;
        }
    }
}

void calcDown(Matrix * this)
{
    int i = 0;
    int j = 0;
    while (i < this->height && j < this->height)
	{
        if (equal(this->matr[i* this->width + j], 0))
		{
            for (int k = i + 1; k < this->height; k++)
			{
                if (!equal(this->matr[k* this->width + j], 0))
				{
                    swap(this, k, i);
                    break;
                }
            }
        }
        if (!equal(this->matr[i* this->width + j], 0))
		{
            add(this, i++, j++);
        } else 
		{
            this->rk--;
            j++;
        }
    }
    getExtendedRank(this);
}


int outputSolutions(char * filename, float * ans, int n) 
{
    FILE * f = fopen(filename, "w");
    if (!f) 
	{
        return 0;
    }
    for (int i = 0; i < n; i++) 
	{
        fprintf(f, "%f\n", ans[i]);
    }
    fclose(f);
    return 1;
}

int outputWord(char * filename, char * str)
{
    FILE * f = fopen(filename, "w");
    if (!f)
	{
        return 0;
    }
    fprintf(f, "%s", str);
    fclose(f);
    return 1;
}

int input(char * filename, Matrix * this)
{
    FILE * f = fopen(filename, "r");
    if (!f) {
        return -1; // Input file cannot be opened
    }

    if (fscanf(f, "%i", &this->height) <= 0) {
        return -3; // First is not a number
    }

    this->width = this->height + 1;
    this->rk = this->height;
    this->rkex = this->height;
    this->matr = malloc(sizeof(float) * this->height * this->width);
    if (!this->matr)
	{
        //Matrix can't be stored
        return -2;
    }

    for (int i = 0; i < this->height; i++)
	{
        for (int j = 0; j < this->width; j++)
		{
            if (fscanf(f, "%f", &this->matr[i* this->width + j]) <= 0)
			{
                return -3; // error in input file format
            }

        }
    }
    fclose(f);
    return 1;
}

int main(int argc, char ** argv)
{
    if (argc != 3)
	{
        printf("Need 2 arguments: input file name, output file name");
        return -1;
    }
    Matrix * this = malloc(sizeof(Matrix));

    int code;
    if ((code = input(argv[1], this)) < 0)
	{
        switch (code)
		{
            case -1:
                printf("Input file not found");
                return code;
            case -2:
                printf("Can't store matrix in memory");
                return code;
            case -3:
                printf("Input file format error. First line should be n. Then n lines");
                return code;

        }
    }

    calcDown(this);

    if (this->rk != this->rkex)
	{
        // verdict No Solution
        outputWord(argv[2], "no solution");
    } else if (this->rk == this->rkex && this->rk == this->height)
	{
        // one solution
        float * ans = findSolutions(this);
        outputSolutions(argv[2], ans, this->height);
        free(ans);
    } else
	{
        // many solutions
        outputWord(argv[2], "many solutions");
    }

    free(this->matr);
    free(this);
    return 0;
}