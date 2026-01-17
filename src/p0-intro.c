/*
 * CS 261 PA0: Intro project
 *
 * Name:
 */

#include "p0-intro.h"

#include <math.h>
#include <stdlib.h>

int add_abs (int num1, int num2)
{
    return abs(num1) + abs(num2);
}

void add_ptr (int num1, int num2, int *ans)
{
    if(ans != NULL)
    {
    *ans = num1 + num2;
    }
}

int factorial (int num)
{
    if(num <= 1){
        return 1;
    }
    return num * factorial(num - 1);
}

bool is_prime (int num)
{
    if(num <= 1)
    {
        return false;
    } else if(num == 2)
    {
        return true;
    } else if(num % 2 == 0)
    {
        return false;
    }
    for(int i = 3; i * i <= num; i+=2)
    {
        if(num % i == 0)
        {
            return false;
        }
    }
    return true;
}

void add_vec (vector_t v1, vector_t v2, vector_t *result)
{
    double newx = v1.x + v2.x;
    double newy = v1.y + v2.y;
    result->x = newx;
    result->y = newy;
}

double dot_prod_vec (vector_t v1, vector_t v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

int sum_array (int *nums, size_t n)
{
    int sum = 0;
    for(int i = 0; i < n; i++)
    {
        sum += (*(nums + i));
    }
    return sum;
}

void sort_array (int *nums, size_t n)
{
    if(n > 1)
    {
        for(int i = 0; i < n - 1; i++)
        {
            for(int j = 0; j < n - i - 1; j++)
            {
                if(*(nums+j) > (*(nums+j+1)))
                {
                    int temp = *(nums+j+1);
                    *(nums+j+1) = *(nums+j);
                    *(nums+j) = temp;
                }
            }
        }
    }
}

bool read_line (FILE *f, char *buffer, size_t size)
{
    if(f == NULL || buffer == NULL)
    {
        return false;
    }
    if(fgets(buffer, size, f) == NULL)
    {
        return false;
    }
    return true;
}
