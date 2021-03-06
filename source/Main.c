#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bn.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#define TEST_RADIX
#define TEST_RADIX_OUT
#define TEST_ADD_TO_INT
#define TEST_ADDITION
#define TEST_SUBSTITUTION
#define TEST_FIBONACCI
#define TEST_CUSTOM_FIBONACCI_SIGN
#define TEST_MULTIPLICATION
#define TEST_POWER
#define TEST_ROOT
#define TEST_FACTORIAL
#define TEST_DIV_MOD_TO_UINT
#define TEST_DIVISION
#define TEST_MOD
#define TEST_SHIFT

// Ввод числа в системе счисления radix.
int radix()
{
    printf("Radix test.\n\n");
    char String[1024];

    unsigned int Radix = 2;
    printf("Radix: ");
    scanf("%u", &Radix);

    bn *BN = bn_new();
    printf("Big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix(BN, String, Radix);

    printf("\n");
    bn_print_hex(BN);
    printf("\n");
    printf("\n");

    bn_delete(BN);

    return 0;
}

// Вывод числа в системе счисления radix.
int radix_out()
{
    printf("Radix test.\n\n");
    char String[1024];

    unsigned int Radix = 2;
    printf("Radix: ");
    scanf("%u", &Radix);

    bn *BN = bn_new();
    printf("Big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN, String);

    const char *str = bn_to_string(BN, Radix);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);
    bn_delete(BN);

    //_CrtDumpMemoryLeaks();

    return 0;
}

// Сложение большого числа и int.
int add_to_int()
{
    printf("Addition to int test.\n\n");
    char String[1024];

    bn *BN = bn_new();
    int Integer = 0;

    printf("Big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix(BN, String, 10);

    printf("\n");

    printf("Integer: ");
    scanf("%d", &Integer);

    // СЛОЖЕНИЕ.
    bn_add_to_int(BN, Integer);

    const char *str = bn_to_string(BN, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);

    bn_delete(BN);

    return 0;
}

// Cложение двух вводимых чисел.
int addition()
{
    printf("Addition test.\n\n");
    char String[1024];

    bn *BN1 = bn_new();

    printf("First big number (hex, 1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix_pow2(BN1, String, 16);

    printf("BN1: \n");
    bn_print_hex(BN1);


    printf("\n");


    bn *BN2 = bn_new();

    printf("Second big number (hex, 1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix_pow2(BN2, String, 16);

    printf("BN2: \n");
    bn_print_hex(BN2);

    // СЛОЖЕНИЕ.
    bn_add_to(BN1, BN2);

    printf("\n");
    bn_print_hex(BN1);
    printf("\n");
    printf("\n");

    bn_delete(BN1);
    bn_delete(BN2);

    return 0;
}

// Вычитание двух вводимых чисел.
int substitution()
{
    printf("Substitution test.\n\n");
    char String[1024];

    bn *BN1 = bn_new();

    printf("First big number 1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN1, String);

    printf("BN1: \n");
    bn_print_hex(BN1);


    printf("\n");


    bn *BN2 = bn_new();

    printf("Second big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN2, String);

    printf("BN2: \n");
    bn_print_hex(BN2);

    // ВЫЧИТАНИЕ.
    bn_sub_to(BN1, BN2);

    const char *str = bn_to_string(BN1, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);

    bn_delete(BN1);
    bn_delete(BN2);

    return 0;
}

// Нахождение N-ого элемента последовательности Фибоначчи.
int fibonacci()
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

    return 0;
}

// Нахождение номера последней пары соседних элементов модифицированной последовательности Фибоначчи, имеющих разный знак.
// Гипотеза: для двух стартовых элементов, отношение которых близко к золотому сечению, искомый номер растёт линейно с ростом
// длины (логарифма в случае целого) числа.
int custom_fibonacci_sign()
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

    return 0;
}

// Умножение большого числа на int.
int mul_to_int()
{
    printf("Multiplication to int test.\n\n");
    char String[1024];

    bn *BN = bn_new();
    int Integer = 0;

    printf("Big number (hex, 1023 symbols max): ");
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

    return 0;
}

// Деление (с остатком) большого числа на unsigned int.
int div_mod_to_uint()
{
    printf("Division to unsigned int test.\n\n");
    char String[1024];

    bn *BN = bn_new();
    int Integer = 0;

    printf("Big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN, String);

    printf("BN: \n");
    bn_print_formula(BN);

    printf("\n");

    printf("Integer: ");
    scanf("%d", &Integer);

    // ДЕЛЕНИЕ.
    bn_div_mod_to_uint(BN, Integer, 0);

    printf("\n");
    bn_print_hex(BN);
    printf("\n");
    printf("\n");

    bn_init_string(BN, String);

    // ОСТАТОК.
    bn_div_mod_to_uint(BN, Integer, 1);

    printf("\n");
    bn_print_hex(BN);
    printf("\n");
    printf("\n");

    bn_delete(BN);

    return 0;
}

// Умножение больших чисел.
int multiplication()
{
    printf("Multiplication test.\n\n");
    char String[1024];

    bn *BN1 = bn_new();

    printf("First big number (hex, 1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix_pow2(BN1, String, 16);

    printf("BN1: \n");
    bn_print_hex(BN1);


    printf("\n");


    bn *BN2 = bn_new();

    printf("Second big number (hex, 1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix_pow2(BN2, String, 16);

    printf("BN2: \n");
    bn_print_hex(BN2);

    // УМНОЖЕНИЕ.
    bn_mul_to(BN1, BN2);

    printf("\n");
    bn_print_hex(BN1);
    printf("\n");
    printf("\n");

    bn_delete(BN1);
    bn_delete(BN2);

    return 0;
}

// Возведение в степень.
int power()
{
    printf("Power to test.\n\n");
    char String[1024];

    bn *BN1 = bn_new();

    printf("First big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN1, String);

    printf("\n");

    int power = 1;
    printf("Power: ");
    scanf("%d", &power);

    // ВОЗВЕДЕНИЕ В СТЕПЕНЬ.
    bn_pow_to(BN1, power);

    const char *str = bn_to_string(BN1, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);
    bn_delete(BN1);

    return 0;
}

// Нахождение корня.
int root()
{
    printf("Root to test.\n\n");
    char String[1024];

    bn *BN1 = bn_new();

    printf("Big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string(BN1, String);

    printf("\n");

    int reciprocal = 1;
    printf("Reciprocal: ");
    scanf("%d", &reciprocal);

    // НАХОЖДЕНИЯ КОРНЯ.
    bn_root_to(BN1, reciprocal);

    const char *str = bn_to_string(BN1, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);
    bn_delete(BN1);

    return 0;
}

// Нахождение факториала N.
int factorial()
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

// Деление больших чисел.
int division()
{
    printf("Division test.\n\n");
    char String1[1024];
    char String2[1024];

    bn *BN1 = bn_new();

    printf("First big number (1023 symbols max): ");
    scanf("%s", String1);
    bn_init_string(BN1, String1);

    printf("\n");

    bn *BN2 = bn_new();

    printf("Second big number (1023 symbols max): ");
    scanf("%s", String2);
    bn_init_string(BN2, String2);

    // ДЕЛЕНИЕ.
    bn_div_to(BN1, BN2);

    const char *str = bn_to_string(BN1, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    bn_init_string(BN1, String1);

    // ДЕЛЕНИЕ С ОСТАТКОМ.
    bn_mod_to(BN1, BN2);

    str = bn_to_string(BN1, 10);

    printf("\n");
    printf("%s", str);
    printf("\n");
    printf("\n");

    free((char*)str);

    bn_delete(BN1);
    bn_delete(BN2);

    return 0;
}

// Деление с остатком больших чисел.
int mod()
{
    printf("Mod test.\n\n");
    char String[1024];

    unsigned int Radix = 2;
    printf("Radix: ");
    scanf("%u", &Radix);

    bn *BN1 = bn_new();

    printf("First big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix(BN1, String, Radix);

    printf("BN1: \n");
    bn_print_formula(BN1);


    printf("\n");


    bn *BN2 = bn_new();

    printf("Second big number (1023 symbols max): ");
    scanf("%s", String);
    bn_init_string_radix(BN2, String, Radix);

    printf("BN2: \n");
    bn_print_formula(BN2);

    // Деление с остатком.
    bn_mod_to(BN1, BN2);

    printf("\n");
    bn_print_hex(BN1);
    printf("\n");
    printf("\n");

    bn_delete(BN1);
    bn_delete(BN2);

    return 0;
}

// Проверка функции сдвига числа.
int shift()
{
    printf("Shift test.\n\n");
    char String[1024];

    unsigned int Radix = 2;
    printf("Radix: ");
    scanf("%u", &Radix);

    bn *BN = bn_new();
    printf("Big number (1023 symbols max): ");
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

    return 0;
}




int main()
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    #ifdef TEST_RADIX
    radix();
    #endif

    #ifdef TEST_RADIX_OUT
    radix_out();
    #endif

    #ifdef TEST_ADD_TO_INT
    add_to_int();
    #endif

    #ifdef TEST_ADDITION
    addition();
    #endif

    #ifdef TEST_SUBSTITUTION
    substitution();
    #endif

    #ifdef TEST_FIBONACCI
    fibonacci();
    #endif

    #ifdef TEST_CUSTOM_FIBONACCI_SIGN
    custom_fibonacci_sign();
    #endif

    #ifdef TEST_MULTIPLICATION
    multiplication();
    #endif

    #ifdef TEST_POWER
    power();
    #endif

    #ifdef TEST_ROOT
    root();
    #endif

    #ifdef TEST_FACTORIAL
    factorial();
    #endif

    #ifdef TEST_DIV_MOD_TO_UINT
    div_mod_to_uint();
    #endif

    #ifdef TEST_DIVISION
    division();
    #endif

    #ifdef TEST_MOD
    mod();
    #endif

    #ifdef TEST_SHIFT
    shift();
    #endif

    return 0;
}
