#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bn.h"

// Ввод числа в системе счисления radix.
int radix_test()
{
	while (1)
	{
		printf("Radix test.\n\n");
		char String[1024];

		unsigned int Radix = 2;
		printf("Radix: ");
		scanf("%u", &Radix);

		bn *BN = bn_new();
		printf("Big number (1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix(BN, String, Radix);
		
		printf("\n");
		bn_print_hex(BN);
		printf("\n");
		printf("\n");

		bn_delete(BN);
	}

	return(0);
}

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

// Нахождение номера последней пары соседних элементов модифицированной последовательности Фибоначчи, имеющих разный знак.
// Гипотеза: для двух стартовых элементов, отношение которых близко к золотому сечению, искомый номер растёт линейно с ростом
// длины (логарифма в случае целого) числа.
int custom_fibonacci_sign_test()
{
	printf("Custom fibonacci test.\n\n");

	char *String1 = calloc(2048, sizeof(char));
	char *String2 = calloc(2048, sizeof(char));

	bn *BN1 = bn_new();
	printf("The first element: ");
	scanf("%s", String1);

	printf("\n");

	bn *BN2 = bn_new();
	printf("The second element: ");
	scanf("%s", String2);

	printf("\n");

	size_t Str1Len = strlen(String1);
	size_t Str2Len = strlen(String2);

	printf("Max number of elements: ");
	long long unsigned int MaxN = 0;
	scanf("%llu", &MaxN);

	bn *TEMP = bn_new();

	for (size_t j = 0; (j < Str1Len) && (j < Str2Len); j++)
	{
		String1[Str1Len - j] = '\0';
		String2[Str2Len - j] = '\0';

		bn_init_string_radix_pow2(BN1, String1, 16);
		bn_init_string_radix_pow2(BN2, String2, 16);

		bn_init_int(TEMP, 0);

		unsigned int i = 0;

		for (i = 0; i < MaxN; ++i)
		{
			if (bn_sign(BN1)*bn_sign(BN2) >= 0)
			{
				break;
			}

			bn_copy(TEMP, BN2);
			bn_add_to(BN2, BN1);
			bn_copy(BN1, TEMP);
		}

		printf("\n%zu : %u", Str1Len - j, i);
	}

	bn_delete(BN1);
	bn_delete(BN2);
	bn_delete(TEMP);

	free(String1);
	free(String2);

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

// Умножение больших чисел.
int division_test()
{
	while (1)
	{
		printf("Division test.\n\n");
		char String[1024];

		unsigned int Radix = 2;
		printf("Radix: ");
		scanf("%u", &Radix);

		bn *BN1 = bn_new();

		printf("First big number (1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix(BN1, String, Radix);

		printf("BN1: \n");
		bn_print_formula(BN1);


		printf("\n");


		bn *BN2 = bn_new();

		printf("Second big number (1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix(BN2, String, Radix);

		printf("BN1: \n");
		bn_print_formula(BN2);

		// Деление.
		bn_div_to(BN1, BN2);

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

// Проверка функции сдвига числа.
int shift_test()
{
	while (1)
	{
		printf("Shift test.\n\n");
		char String[1024];

		unsigned int Radix = 2;
		printf("Radix: ");
		scanf("%u", &Radix);

		bn *BN = bn_new();
		printf("Big number (1024 symbols max): ");
		scanf("%s", String);
		bn_init_string_radix(BN, String, Radix);

		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_shift(BN, 1);
		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_shift(BN, 2);
		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_shift(BN, -1);
		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_shift(BN, -3);
		printf("\n");
		bn_print_formula(BN);
		printf("\n");
		printf("\n");

		bn_delete(BN);
	}

	return(0);
}




int main()
{
	//addition_test();
	//fibonacci_test();

	//multiplication_test();
	//factorial_test();
	//custom_fibonacci_sign_test();

	//radix_test();

	division_test();

	return(0);
}