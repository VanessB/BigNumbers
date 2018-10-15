#include <stdio.h>
#include "bn.h"

// Cложение двух вводимых чисел.
int addition_test()
{
	while (1)
	{
		printf("Addition test.\n\n");
		char String[1024];

		bn *BN1 = bn_new();

		printf("First big number (hex, 1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix_pow2(BN1, String, 16);

		printf("BN1: \n");
		bn_print_hex(BN1);


		printf("\n");


		bn *BN2 = bn_new();

		printf("Second big number (hex, 1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix_pow2(BN2, String, 16);

		printf("BN1: \n");
		bn_print_hex(BN2);

		// СЛОЖЕНИЕ.
		bn_add_to(BN1, BN2);

		printf("\n");
		bn_print_hex(BN1);
		printf("\n");
		printf("\n");

		bn_delete(BN1);
		bn_delete(BN2);
	}

	return(0);
}

// Нахождение N-ого элемента последовательности Фибоначчи.
int fibonacci_test()
{
	printf("Fibonacci test.\n\n");
	printf("Number of element (the first one is 1): ");
	long long unsigned int N = 0;
	scanf("%llu", &N);

	if (N <= 2) { printf("1\n"); }
	else
	{
		bn *BN1 = bn_new();
		bn_init_int(BN1, 1);
		bn *BN2 = bn_new();
		bn_init_int(BN2, 1);

		bn *TEMP = bn_new();

		for (unsigned int i = 2; i < N; ++i)
		{
			bn_copy(TEMP, BN2);
			bn_add_to(BN2, BN1);
			bn_copy(BN1, TEMP);
		}

		printf("\n");

		bn_print_formula(BN2);

		bn_delete(BN1);
		bn_delete(BN2);
		bn_delete(TEMP);
	}

	return(0);
}

// Умножение большого числа на int.
int mul_to_int_test()
{
	while (1)
	{
		printf("Multiplication to int test.\n\n");
		char String[1024];

		bn *BN = bn_new();
		int Integer = 0;

		printf("Big number (hex, 1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix_pow2(BN, String, 16);

		printf("BN: \n");
		bn_print_formula(BN);


		printf("\n");


		printf("Integer: ");
		scanf("%d", &Integer);

		// УМНОЖЕНИЕ.
		bn_mul_to_int(BN, Integer);

		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_delete(BN);
	}

	return(0);
}

// Умножение больших чисел.
int multiplication_test()
{
	while (1)
	{
		printf("Multiplication test.\n\n");
		char String[1024];

		bn *BN1 = bn_new();

		printf("First big number (hex, 1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix_pow2(BN1, String, 16);

		printf("BN1: \n");
		bn_print_hex(BN1);


		printf("\n");


		bn *BN2 = bn_new();

		printf("Second big number (hex, 1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix_pow2(BN2, String, 16);

		printf("BN1: \n");
		bn_print_hex(BN2);

		// Умножение.
		bn_mul_to(BN1, BN2);

		printf("\n");
		bn_print_hex(BN1);
		printf("\n");
		printf("\n");

		bn_delete(BN1);
		bn_delete(BN2);
	}

	return(0);
}

// Нахождение факториала N.
int factorial_test()
{
	printf("Factorial test.\n\n");
	printf("Enter N: ");
	long long unsigned int N = 0;
	scanf("%llu", &N);

	bn *Step = bn_new();
	bn_init_int(Step, 1);

	bn *CurrN = bn_new();
	bn_init_int(CurrN, 2);

	bn *Factorial = bn_new();
	bn_init_int(Factorial, 1);

	for (long long unsigned int i = 1; i < N; ++i)
	{
		bn_mul_to(Factorial, CurrN);
		bn_add_to(CurrN, Step);
	}

	bn_print_hex(Factorial);

	bn_delete(Step);
	bn_delete(CurrN);
	bn_delete(Factorial);

	return(BN_OK);
}




int main()
{
	//addition_test();
	//fibonacci_test();

	//multiplication_test();
	factorial_test();

	return(0);
}