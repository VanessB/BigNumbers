#include "bn.h"
#include <stddef.h>
#include <string.h> // Для memcpy.
#include <stdlib.h> // Для abs().
#include <stdio.h>

const int UINT_BITS = sizeof(unsigned int) * 8;
const double POW2_32 = 4294967296.0; // 2^32

struct bn_s
{
	unsigned int *Body; // Тело большого числа.
	size_t BodySize; // Размер массива Body (32 бита на кусок, чтобы можно было выполнять операции с двумя кусками в 64-битных переменных).
	signed char Sign; // Знак числа.
};

// ВСПОМОГАТЕЛЬНОЕ

// Изменение размера Body:
int bn_resize(bn *OperatedBN, const size_t NewBodySize)
{
	if ((OperatedBN == NULL) || ((OperatedBN->Body == NULL))) { return(BN_NULL_OBJECT); }
	if (NewBodySize != OperatedBN->BodySize)
	{
		size_t Size = sizeof(unsigned int) * NewBodySize;
		unsigned int *NewPointer = realloc(OperatedBN->Body, sizeof(unsigned int) * NewBodySize);
		if (NewPointer == NULL) { return(BN_NO_MEMORY); }
		OperatedBN->Body = NewPointer;
		// Если NewBodySize < OperatedBN->BodySize, произойдет "урезание" верхних разрядов.

		// TODO: заменить memset'ом.
		for (size_t i = OperatedBN->BodySize; i < NewBodySize; ++i)
		{
			OperatedBN->Body[i] = 0;
		}

		OperatedBN->BodySize = NewBodySize;
	}

	return(BN_OK);
}

// Обрезание верхних нулевых блоков.
int bn_shrink(bn *OperatedBN)
{
	if ((OperatedBN == NULL) || ((OperatedBN->Body == NULL))) { return(BN_NULL_OBJECT); }

	size_t i = OperatedBN->BodySize; // OperatedBN->BodySize > 0;
	while(--i > 0)
	{
		if (OperatedBN->Body[i]) { break; } // Попался первый ненулевой блок.
	}

	int Result = BN_OK;
	// Действительно ли придётся обрезать?
	if (++i < OperatedBN->BodySize) { Result = bn_resize(OperatedBN, i); }
	// Если нулевой размер с нулевыми блоками, знак = 0.
	if ((i == 1) && !(OperatedBN->Body[0])) { OperatedBN->Sign = 0; }

	return(Result);
}

// Копирование.
int bn_copy(bn *dest, bn const *orig)
{
	if ((dest == NULL) || ((dest->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((orig == NULL) || ((orig->Body == NULL))) { return(BN_NULL_OBJECT); }

	bn_resize(dest, orig->BodySize);
	dest->Sign = orig->Sign;

	memcpy(dest->Body, orig->Body, orig->BodySize * sizeof(unsigned int));

	return(BN_OK);
}

// Копирование значения, сдвинутого на shift блоков в сторону старших разрядов.
int bn_copy_shift(bn *dest, bn const *orig, long long int shift)
{
	if ((dest == NULL) || ((dest->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((orig == NULL) || ((orig->Body == NULL))) { return(BN_NULL_OBJECT); }

	if (!shift)
	{
		return(bn_copy(dest, orig));
	}

	size_t OrigSize = orig->BodySize;
	if (-shift >= (long long int)OrigSize)
	{
		// Нечего копировать с таким большим по модулю отрицательным сдвигом.

		bn_resize(dest, 1);
		dest->Body[0] = 0;
		dest->Sign = 0;

		return(BN_OK);
	}

	// С этого момента orig->BodySize + shift >= 1. Иначе уже произошёл выход из функции.

	// Изменяем размер dest, заполняем нулями. Знак можно не обнулять, он позже скопируется.
	{
		size_t NewSize = (size_t)((long long int)orig->BodySize + shift);
		unsigned int *NewPointer = realloc(dest->Body, sizeof(unsigned int) * NewSize);
		if (NewPointer == NULL) { return(BN_NO_MEMORY); }
		dest->Body = NewPointer;
		dest->BodySize = NewSize;

		memset(dest->Body, 0, dest->BodySize * sizeof(unsigned int));
	}

	dest->Sign = orig->Sign;

	unsigned int *DestPtr = dest->Body;
	unsigned int *OrigPtr = orig->Body;
	if (shift > 0) { DestPtr += shift; } // DestPtr = max(dest->Body, dest->Body + shift).
	else
	{
		OrigSize += shift; // OrigSize = min(orig->BodySize, orig->BodySize + shift).
		OrigPtr -= shift; // OrigPtr = max(orig->Body, orig->Body - shift).
	}

	memcpy(DestPtr, OrigPtr, OrigSize * sizeof(unsigned int));

	return(BN_OK);
}

// Сдвиг на shift блоков в сторону старших разрядов.
int bn_shift(bn *t, long long int shift)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	if (!shift)
	{
		return(BN_OK);
	}

	size_t OrigSize = t->BodySize;
	if (-shift >= (long long int)OrigSize)
	{
		// Нечего делать с таким большим по модулю отрицательным сдвигом.

		bn_resize(t, 1);
		t->Body[0] = 0;
		t->Sign = 0;

		return(BN_OK);
	}

	// С этого момента t->BodySize + shift >= 1. Иначе уже произошёл выход из функции.

	// Изменяем размер t, если shift > 0.
	if (shift > 0)
	{
		size_t NewSize = (size_t)((long long int)t->BodySize + shift);
		unsigned int *NewPointer = realloc(t->Body, sizeof(unsigned int) * NewSize);
		if (NewPointer == NULL) { return(BN_NO_MEMORY); }
		t->Body = NewPointer;
		t->BodySize = NewSize;
	}

	unsigned int *DestPtr = t->Body;
	unsigned int *OrigPtr = t->Body;
	if (shift > 0)
	{
		DestPtr += shift; // DestPtr = max(t->Body, t->Body + shift).
	}
	else
	{
		OrigSize += shift; // OrigSize = min(t->BodySize, t->BodySize + shift).
		OrigPtr -= shift; // OrigPtr = max(t->Body, t->Body - shift).
	}

	memmove(DestPtr, OrigPtr, OrigSize * sizeof(unsigned int));

	// Заполняем нулями пустые блоки.
	if (shift > 0)
	{
		memset(t->Body, 0, shift * sizeof(unsigned int));
	}
	else // Изменяем размер t, если shift < 0.
	{
		size_t NewSize = (size_t)((long long int)t->BodySize + shift);
		unsigned int *NewPointer = realloc(t->Body, sizeof(unsigned int) * NewSize);
		if (NewPointer == NULL) { return(BN_NO_MEMORY); }
		t->Body = NewPointer;
		t->BodySize = NewSize;
	}

	return(BN_OK);
}


// Создать новое BN
bn *bn_new()
{
	bn *NewBN = malloc(sizeof(bn));
	if (NewBN == NULL) { return(NULL); }
	NewBN->BodySize = 1;
	NewBN->Sign = 0;

	NewBN->Body = malloc(sizeof(unsigned int) * NewBN->BodySize);
	if (NewBN->Body == NULL)
	{
		free(NewBN);
		return(NULL);
	}
	NewBN->Body[0] = 0;
	return(NewBN);
}

// Создать копию существующего BN
bn *bn_init(bn const *orig)
{
	bn *NewBN = malloc(sizeof(bn));
	if (NewBN == NULL) { return(NULL); }
	NewBN->BodySize = orig->BodySize;
	NewBN->Sign = orig->Sign;

	size_t Size = sizeof(unsigned int) * NewBN->BodySize;
	NewBN->Body = malloc(Size);
	if (NewBN->Body == NULL)
	{
		free(NewBN);
		return(NULL);
	}
	memcpy(NewBN->Body, orig->Body, Size);
	return(NewBN);
}

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	int Error = BN_OK;

	Error = bn_resize(t, 1);
	if (Error) { return(Error); }
	t->Body[0] = 0;
	t->Sign = 0;

	size_t StringLength = 0;
	signed char Sign = 1; // Храним тут знак до того, как убедимся, что строка не нулевая.
	if (init_string[0] == '-')
	{
		++init_string; //Сдвигаем указатель с минуса на первую цифру числа.
		Sign = -1;
	}

	char FilledWithNulls = 1;
	while (init_string[StringLength] != '\0')
	{
		if (FilledWithNulls && (init_string[StringLength] != '0')) { FilledWithNulls = 0; } // Проверка на зополненность одними нулями.
		++StringLength; // Ищем длину строки.
	}

	if (!StringLength || FilledWithNulls) { return(BN_OK); } // С пустой строкой нечего делать.

	//Error = bn_shrink(t);
	//if (Error) { return(Error); }

	bn *Power = bn_new();
	if (Power == NULL) { return(BN_NO_MEMORY); }

	bn *PoweredDigit = bn_new();
	if (PoweredDigit == NULL) { return(BN_NO_MEMORY); }

	Error = bn_init_int(Power, 1);
	if (Error) { return(Error); }

	unsigned char Digit = 0; // Знак разряда.
	for (long long int i = StringLength; --i >= 0;)
	{
		Digit = init_string[i];

		// Преводим из символа в число:
		if (Digit >= 'a') { Digit += 10 - 'a'; }
		else if (Digit >= 'A') { Digit += 10 - 'A'; }
		else { Digit -= '0'; }

		int Error = bn_init_int(PoweredDigit, Digit);
		if (Error) { return(Error); }

		Error = bn_mul_to(PoweredDigit, Power);
		if (Error) { return(Error); }

		Error = bn_add_to(t, PoweredDigit);
		if (Error) { return(Error); }

		Error = bn_mul_to_uint(Power, radix);
		if (Error) { return(Error); }
	}

	Error = bn_delete(Power);
	if (Error) { return(Error); }

	t->Sign = Sign; // Для непустой строки ставим знак.

	Error = bn_delete(PoweredDigit);
	return(Error);
}

// Инициализировать значение BN представлением строки
// в системе счисления radix = 2^k.
int bn_init_string_radix_pow2(bn *t, const char *init_string, int radix)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	memset(t->Body, 0, t->BodySize);
	t->Sign = 0;

	size_t StringLength = 0;
	signed char Sign = 1; // Храним тут знак до того, как убедимся, что строка не нулевая.
	if (init_string[0] == '-')
	{
		++init_string; //Сдвигаем указатель с минуса на первую цифру числа.
		Sign = -1;
	}

	char FilledWithNulls = 1;
	while (init_string[StringLength] != '\0')
	{
		if (FilledWithNulls && (init_string[StringLength] != '0')) { FilledWithNulls = 0; } // Проверка на зополненность одними нулями.
		++StringLength; // Ищем длину строки.
	}

	if (!StringLength || FilledWithNulls) { return(BN_OK); } // С пустой строкой нечего делать.
	t->Sign = Sign; // Для непустой строки ставим знак.

	// Так как система счисления имеет базу - степень двойки:
	int RadixPower = 1;
	for (int tempRadix = radix; tempRadix > 2; tempRadix >>= 1, ++RadixPower); // Смотрим, какой именно степенью двойки является radix.
	// Теперь RadixPower = log_2(radix)

	size_t NewSize = (StringLength * RadixPower) / (UINT_BITS) + ((StringLength * RadixPower) % (UINT_BITS) != 0);
	//            ceil( (~log(N)) * log_2(radix) / BitsInBlock ) - Верхняя оценка количества требуемых блококв.

	// Изменяем размер.
	int Error = bn_resize(t, NewSize);
	if (Error) { return(Error); }

	size_t BlockPos = 0;
	long long unsigned int Block = 0;

	unsigned char Power = 0; // Степень двойки при знаке.
	unsigned char Digit = 0; // Знак разряда.
	for (long long int i = StringLength; --i >= 0;)
	{		
		Digit = init_string[i];

		// Преводим из символа в число:
		if (Digit >= 'a') { Digit += 10 - 'a'; }
		else if (Digit >= 'A') { Digit += 10 - 'A'; }
		else { Digit -= '0'; }

		Block += (long long unsigned int)Digit << Power; // Аналогично (+= Digit * 2^Power)
		Power += RadixPower; // Переходим к следующему знаку - повышаем степень.

		if (Power >= UINT_BITS) // Теперь новый прочитаный знак не может повлиять на текущий блок Body.
		{
			t->Body[BlockPos] = (Block & UINT_MAX); //Первые 32 бита блока записываем в текущий блок Body.
			Block >>= UINT_BITS - 1; //Остальные биты сдвигаем на 32 позиции, так как готовимся писать в следующий блок.
			Block >>= 1; //Там была какая-то дичь со сдвигом на 32 бита за раз. Надо проверить.
			++BlockPos; //Переходим к следующему блоку в Body.
			Power -= UINT_BITS; //Уменьшаем степень.
		}
	}

	if (Block) { t->Body[BlockPos] = (Block & UINT_MAX); } //Записываем недописанный блок.

	return(BN_OK);
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	int Error = bn_resize(t, 1);
	if (Error) { return(Error); }
	bn_resize(t, 1);
	t->Body[0] = abs(init_int);
	t->Sign = (0 < init_int) - (init_int < 0); // -1 - отрицательное. 0 - ноль. 1 - положительное.

	return(BN_OK);
}

// Уничтожить BN (освободить память)
int bn_delete(bn *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	free(t->Body);
	free(t);
	return(BN_OK);
}




// Беззнаковое сложение.
int bn_unsigned_add_to(bn *left, bn const *right, char mode)
// mode = 0 - выход за пределы последнего блока влечет смену знака на положительный - симуляция сложения с доп кодом.
// mode = 1 - выход за пределы последнего блока влечет создание нового и перенос туда старших разрядов, получившихся после сложения предыдущего блока.
{
	if ((left == NULL) || ((left->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((right == NULL) || ((right->Body == NULL))) { return(BN_NULL_OBJECT); }
	
	long long unsigned int Block = 0;
	if (left->BodySize < right->BodySize) // Если в левое число сумма точно не поместится, ...
	{
		bn_resize(left, right->BodySize); // ... расширяем левое число до длмны правого.
	}

	size_t i = 0;
	for (i = 0; i < right->BodySize; ++i) // Поблочно суммируем с переносами излишков.
	{
		Block += (long long unsigned int)left->Body[i] + (long long unsigned int)right->Body[i];
		left->Body[i] = (Block & UINT_MAX);

		Block >>= 31; // INTEL!!!  D:<
		Block >>= 1;
	}

	for (; i < left->BodySize; ++i)
	{
		if (!Block) { break; } // Нечего переносить в следующий разряд...

		Block += (long long unsigned int)left->Body[i];
		left->Body[i] = (Block & UINT_MAX);

		Block >>= 31; // INTEL!!!  D:<
		Block >>= 1;
	}

	if (Block) // Число закончилось, но еще есть, что перенести в старшие разряды.
	{
		switch (mode)
		{
			case 0:
			{
				bn_neg(left); // Прибавили больше или равно, чем модуль left - меняем знак.
				bn_shrink(left); // Убираем нулевые блоки. Знак автоматом сбросится до нуля, если весь left = 0;
				break;
			}
			case 1:
			{
				if (left->BodySize == right->BodySize) { bn_resize(left, left->BodySize + 1); } // Добавляем еще один разряд.
				left->Body[left->BodySize - 1] = (Block & UINT_MAX); // И прибавляем к нему, что осталось.
				break;
			}
		}
	}

	return(BN_OK);
}
// Обращение числа в дополнительный код.
int bn_twos_complement(bn *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	size_t i = 0;
	for (i = 0; i < t->BodySize; ++i)
	{
		t->Body[i] = ~(t->Body[i]);
		++(t->Body[i]);
		if (t->Body[i]) { ++i;  break; } // Прибавление единицы в текущий блок не повлекло выход за пределы текущего разряда.
	}
	for (; i < t->BodySize; ++i)
	{
		t->Body[i] = ~(t->Body[i]);
	}

	return(BN_OK);
}
// Возвращение числа из доп кода обратно в нормальный вид.
int bn_reverse_twos_complement(bn *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	size_t i = 0;
	for (i = 0; i < t->BodySize; ++i)
	{
		--(t->Body[i]);
		t->Body[i] = ~(t->Body[i]);
		if (t->Body[i]) // Если t->Body[i] == 0, то до двух предыдущих операций он тоже был равен 0.
		{
			// Убавление единицы не повлекло уменьшение следующего блока.
			++i;
			break;
		}
	}
	for (; i < t->BodySize; ++i)
	{
		t->Body[i] = ~(t->Body[i]);
	}

	return(BN_OK);
}

// Сложение с.
int bn_add_to(bn *t, bn const *right)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((right == NULL) || ((right->Body == NULL))) { return(BN_NULL_OBJECT); }
	
	if (t->Sign)
	{
		if (right->Sign)
		{
			if (t->BodySize < right->BodySize) // Если в левое число сумма точно не поместится, ...
			{
				bn_resize(t, right->BodySize); // ... расширяем левое число до длины правого.
			}

			// Если знаки разные, переводим в доп код левое число (неважно, положительно или отрицательно оно).
			if (!(t->Sign + right->Sign))
			{
				bn_twos_complement(t);
			}

			long long unsigned int Block = 0;
			size_t i = 0;
			for (i = 0; i < right->BodySize; ++i) // Поблочно суммируем с переносами излишков.
			{
				Block += (long long unsigned int)t->Body[i] + (long long unsigned int)right->Body[i];
				t->Body[i] = (Block & UINT_MAX);

				Block >>= 31; // INTEL!!!  D:<
				Block >>= 1;
			}

			for (; i < t->BodySize; ++i)
			{
				if (!Block) { break; } // Нечего переносить в следующий разряд...

				Block += (long long unsigned int)t->Body[i];
				t->Body[i] = (Block & UINT_MAX);

				Block >>= 31; // INTEL!!!  D:<
				Block >>= 1;
			}

			if (Block) // Число закончилось, но еще есть, что перенести в старшие разряды.
			{
				// Для чисел одного знака просто добавляем еще один блок (если надо) и излишки переносим туда.
				if (t->Sign + right->Sign)
				{
					// Проверяем, хватит ли текущего количества блоков.
					if (t->BodySize == i)
					{
						// Не хватило, так как дошли до последнего (i == t->BodySize).
						bn_resize(t, t->BodySize + 1);
					} 
					t->Body[t->BodySize - 1] = (Block & UINT_MAX); // Прибавляем к последнему блоку, что осталось.
				}
				else
				{
					// Числа разных знаков. Вспоминаем, что левое было записано в доп коде.
					// Тогда выход за пределы текущих блоков возможен только тогда, когда при сложении левое число должно поменять знак.
					bn_neg(t); // Меняем знак.
					bn_shrink(t); // Убираем нулевые блоки. Знак автоматом сбросится до нуля, если весь t = 0;
				}
			}

			// Если знаки все еще разные, значит, левое всё еще в доп коде. Переводим в нормальную форму.
			if (!(t->Sign + right->Sign))
			{
				bn_reverse_twos_complement(t);
				bn_shrink(t); // Убираем нулевые блоки. Знак автоматом сбросится до нуля, если весь t = 0;
			}

			return(BN_OK);

			// Старая реализация через отдельную функцию беззнакового сложения.
			/*
			if (t->Sign + right->Sign) // Выполняется, если одинаковые знаки.
			{
				return(bn_unsigned_add_to(t, right, 1));
			}
			else
			{
				if (t->Sign == -1)
				{
					bn_twos_complement(t);
					int Result = bn_unsigned_add_to(t, right, 0);

					// Если в результате отрицательное число, переводим в нормальную форму.
					if (t->Sign == -1) { bn_reverse_twos_complement(t); }
				}
				else
				{
					// Какой ужас! Правое значение надо переписать в доп код, а оно константно!
					// А кто сказал, что доп код - это только для отрицательных чисел?
					// Теперь положительное число записываем в доп коде.
					// Если результат тоже окажется положительным, то он будет в доп коде.

					bn_twos_complement(t);
					int Result = bn_unsigned_add_to(t, right, 0);
					if (t->Sign == 1) { bn_reverse_twos_complement(t); } // Если в результате положительное число, переводим в нормальную форму.
				}
			}
			*/
		}
		else
		{
			// Сложение с нулём ничего не даёт.
			return(BN_OK);
		}
	}
	else
	{
		// Если t == 0, то после сложения должно быть t == right.
		return(bn_copy(t, right));
	}

	return(BN_OK);
}
// Вычитание из.
int bn_sub_to(bn *t, bn const *right)
{
	// -(-a + b) = a-b.
	bn_neg(t);
	int Error = bn_add_to(t, right);
	bn_neg(t);
	return(Error);
}
// Умножение на (ИСТОРИЧЕСКАЯ ФИГНЯ!).
int bn_mul_to(bn *t, bn const *right)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((right == NULL) || ((right->Body == NULL))) { return(BN_NULL_OBJECT); }

	// Квадратичный алгоритм.

	bn *Temp = bn_new();
	bn *Result = bn_new();

	// Чтобы не делать бесконечных rellocate, зарезервируем память для всего Result.
	int Error = bn_resize(Result, t->BodySize + right->BodySize);
	if (Error) { return(Error); }
	//int Error = 0;

	for (size_t i = 0; i < t->BodySize; ++i)
	{
		Error = bn_copy_shift(Temp, right, i); // Копирование с умножением блоков на 32^i.
		if (Error) { return(Error); }

		bn_mul_to_uint(Temp, t->Body[i]);
		bn_add_to(Result, Temp);
	}

	bn_copy(t, Result);
	bn_delete(Temp);
	bn_delete(Result);

	return(BN_OK);
}
// Деление на (ИСТОРИЧЕСКАЯ ФИГНЯ!!!).
int bn_div_mod_to(bn *t, bn const *right, int mode)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((right == NULL) || ((right->Body == NULL))) { return(BN_NULL_OBJECT); }

	// Квадратичный алгоритм?

	if (!right->Sign)
	{
		return(BN_DIVIDE_BY_ZERO); // Деление на ноль.
	}
	if (!t->Sign)
	{
		return(BN_OK); // Деление нуля.
	}

	if (bn_abs_cmp(right, t) > 0)
	{
		bn_resize(t, 1);
		t->Body[0] = 0;
		t->Sign = 0;

		return(BN_OK);
	}

	unsigned long long int Digit = 0; // Текущий блок, который мы собираемся записать в соотвествующий разряд частного.

	bn *ShiftedRight = bn_new(); // Делитель (right) с учетом текущего разряда в алгоритме со столбиком.
	bn_copy_shift(ShiftedRight, right, t->BodySize - right->BodySize);
	ShiftedRight->Sign = 1; // Но модуль.

	bn *MuledShiftedRight = bn_new(); // ShiftedRight умножить на оценку Digit.

	bn *Num = bn_init(t); // Для вычитания в алгоритме деления столбиком (инициализируется делимым (t)).
	Num->Sign = 1; // Но модуль.

	bn *Result = bn_new(); // Результат.
	bn_init_int(Result, t->Sign * right->Sign); // Инициализируем знаком.

	for (size_t i = 0; i <= t->BodySize - right->BodySize; ++i)
	{
		Digit = 0;

		if (bn_cmp(Num, ShiftedRight) > 0) // Избегаем ситуации, когда Num меньше Temp.
		{
			// Два (один, если размер равен одному) старших блока Num).
			double DNUM = 0.0;

			// Умноженный на 2^32 старший блок Num.
			long long unsigned int ShiftedNumB1 = (long long unsigned int)(Num->Body[Num->BodySize - 1]);
			ShiftedNumB1 <<= 31;
			ShiftedNumB1 <<= 1;

			DNUM = (double)ShiftedNumB1;

			if (Num->BodySize > 1)
			{
				DNUM += (double)(Num->Body[Num->BodySize - 2]); // Еще один блок Num.
			}

			// Аналогично для ShiftedRight.
			double DSHIFTEDRIGHT = 0.0;

			// Умноженный на 2^32 старший блок ShiftedRight.
			long long unsigned int ShiftedRightB1 = (long long unsigned int)(ShiftedRight->Body[ShiftedRight->BodySize - 1]);
			ShiftedRightB1 <<= 31;
			ShiftedRightB1 <<= 1;

			DSHIFTEDRIGHT = (double)ShiftedRightB1;

			if (Num->BodySize > 1)
			{
				DSHIFTEDRIGHT += (double)(ShiftedRight->Body[ShiftedRight->BodySize - 2]); // Еще один блок ShiftedRight.
			}

			if (Num->BodySize > ShiftedRight->BodySize)
			{
				// На самом деле, возможен только вариант с разницой размеров в единицу.
				DSHIFTEDRIGHT /= POW2_32;
			}

			Digit = (long long unsigned int)(DNUM / DSHIFTEDRIGHT);

			//printf("Digit: %llu \n", Digit);
			if (Digit > UINT_MAX)
			{
				Digit = UINT_MAX; // Просто неточность оценки.
				printf("ALERT!!!\n");
			}

			bn_copy(MuledShiftedRight, ShiftedRight);
			bn_mul_to_uint(MuledShiftedRight, (unsigned int)Digit);

			//printf("\n MSR: \n");
			//bn_print_formula(MuledShiftedRight);
			bn_sub_to(Num, MuledShiftedRight);
			//printf("New Num: \n");
			//bn_print_formula(Num);
			//printf("SR: \n");
			//bn_print_formula(ShiftedRight);

			while (Num->Sign < 0)
			{
				bn_add_to(Num, ShiftedRight);
				--Digit;
			}

			while (bn_cmp(Num, ShiftedRight) > 0)
			{
				bn_sub_to(Num, ShiftedRight);
				++Digit;
			}

			//printf("Corrected Num: \n");
			//bn_print_formula(Num);
		}

		Result->Body[0] = (unsigned int)Digit;
		bn_shift(Result, 1);
		bn_shift(ShiftedRight, -1);
	}

	bn_shrink(Result);
	// Деление.
	if (mode == 0)
	{
		bn_copy_shift(t, Result, -1); // Мой дебильный алгоритм дает ответ с лишним сдвигом на один блок.
	}
	// Остаток
	else if (mode == 1)
	{
		bn_copy(t, Num);
		// Надо еще подумать, как в этом случае лучше избежать подсчета Result...
	}

	bn_delete(ShiftedRight);
	bn_delete(MuledShiftedRight);
	bn_delete(Num);
	bn_delete(Result);

	return(BN_OK);
}
int bn_div_to(bn *t, bn const *right)
{
	return(bn_div_mod_to(t, right, 0));
	/*
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	if ((right == NULL) || ((right->Body == NULL))) { return(BN_NULL_OBJECT); }

	// Квадратичный алгоритм?

	if (!right->Sign)
	{
		return(BN_DIVIDE_BY_ZERO); // Деление на ноль.
	}
	if (!t->Sign)
	{
		return(BN_OK); // Деление нуля.
	}

	if (bn_abs_cmp(right, t) > 0)
	{
		bn_resize(t, 1);
		t->Body[0] = 0;
		t->Sign = 0;

		return(BN_OK);
	}

	unsigned long long int Digit = 0; // Текущий блок, который мы собираемся записать в соотвествующий разряд частного.

	bn *ShiftedRight = bn_new(); // Делитель (right) с учетом текущего разряда в алгоритме со столбиком.
	bn_copy_shift(ShiftedRight, right, t->BodySize - right->BodySize);
	ShiftedRight->Sign = 1; // Но модуль.

	bn *MuledShiftedRight = bn_new(); // ShiftedRight умножить на оценку Digit.

	bn *Num = bn_init(t); // Для вычитания в алгоритме деления столбиком (инициализируется делимым (t)).
	Num->Sign = 1; // Но модуль.

	bn *Result = bn_new(); // Результат.
	bn_init_int(Result, t->Sign * right->Sign); // Инициализируем знаком.

	for (size_t i = 0; i <= t->BodySize - right->BodySize; ++i)
	{
		Digit = 0;

		if (bn_cmp(Num, ShiftedRight) > 0) // Избегаем ситуации, когда Num меньше Temp.
		{
			// Два (один, если размер равен одному) старших блока Num).
			double DNUM = 0.0;

			// Умноженный на 2^32 старший блок Num.
			long long unsigned int ShiftedNumB1 = (long long unsigned int)(Num->Body[Num->BodySize - 1]);
			ShiftedNumB1 <<= 31;
			ShiftedNumB1 <<= 1;

			DNUM = (double)ShiftedNumB1;

			if (Num->BodySize > 1)
			{
				DNUM += (double)(Num->Body[Num->BodySize - 2]); // Еще один блок Num.
			}

			// Аналогично для ShiftedRight.
			double DSHIFTEDRIGHT = 0.0;

			// Умноженный на 2^32 старший блок ShiftedRight.
			long long unsigned int ShiftedRightB1 = (long long unsigned int)(ShiftedRight->Body[ShiftedRight->BodySize - 1]);
			ShiftedRightB1 <<= 31;
			ShiftedRightB1 <<= 1;

			DSHIFTEDRIGHT = (double)ShiftedRightB1;

			if (Num->BodySize > 1)
			{
				DSHIFTEDRIGHT += (double)(ShiftedRight->Body[ShiftedRight->BodySize - 2]); // Еще один блок ShiftedRight.
			}

			if (Num->BodySize > ShiftedRight->BodySize)
			{
				// На самом деле, возможен только вариант с разницой размеров в единицу.
				DSHIFTEDRIGHT /= POW2_32;
			}

			Digit = (unsigned int)(DNUM / DSHIFTEDRIGHT);

			printf("Digit: %u \n", Digit);
			if (Digit > UINT_MAX)
			{
				printf("ALERT!!!\n");
			}

			bn_copy(MuledShiftedRight, ShiftedRight);
			bn_mul_to_uint(MuledShiftedRight, (unsigned int)Digit);

			//printf("MSR: \n");
			//bn_print_formula(MuledShiftedRight);
			bn_sub_to(Num, MuledShiftedRight);
			//printf("New Num: \n");
			//bn_print_formula(Num);

			while (Num->Sign < 0)
			{
				bn_add_to(Num, ShiftedRight);
				--Digit;
			}

			while (bn_cmp(Num, ShiftedRight) > 0)
			{
				bn_sub_to(Num, ShiftedRight);
				++Digit;
			}

			//printf("Corrected Num: \n");
			//bn_print_formula(Num);
		}

		Result->Body[0] = (unsigned int)Digit;
		bn_shift(Result, 1);
		bn_shift(ShiftedRight, -1);
	}
	
	bn_shrink(Result);
	bn_copy_shift(t, Result, -1); // Мой дебильный алгоритм дает ответ с лишним сдвигом на один блок.

	bn_delete(ShiftedRight);
	bn_delete(MuledShiftedRight);
	bn_delete(Num);
	bn_delete(Result);

	return(BN_OK);
	*/
}
int bn_mod_to(bn *t, bn const *right)
{
	return(bn_div_mod_to(t, right, 1));
}

// Умножение на маленькое число.
int bn_mul_to_uint(bn *t, unsigned int multipler)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	// Тривиальные случаи.
	if (multipler == 0)
	{
		int Error = bn_resize(t, 1);
		if (Error) { return(Error); }
		t->Body[0] = 0;
		t->Sign = 0;

		return(BN_OK);
	}
	if (multipler == 1)
	{
		return(BN_OK);
	}

	long long unsigned int Block = 0;
	size_t i = 0;
	for (i = 0; i < t->BodySize; ++i) // Поблочно умножаем с переносами излишков.
	{
		Block += (long long unsigned int)t->Body[i] * multipler;
		t->Body[i] = (Block & UINT_MAX);

		Block >>= 31; // INTEL!!!  D:<
		Block >>= 1;
	}

	if (Block) // Число закончилось, но еще есть, что перенести в старшие разряды.
	{
		// Добавляем еще один блок (если надо) и излишки переносим туда.
		// Проверяем, хватит ли текущего количества блоков.
		// Не хватило, так как дошли до последнего (i == t->BodySize).
		bn_resize(t, t->BodySize + 1);
		t->Body[t->BodySize - 1] = (Block & UINT_MAX); // Прибавляем к последнему блоку, что осталось.
	}

	return(BN_OK);
}
int bn_mul_to_int(bn *t, int multipler)
{
	//if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	// Смотрим на знак.
	if (multipler < 0)
	{
		// Умножение на отрицательное число.
		int Error = bn_neg(t); // Меняем знак.
		if (Error) { return(Error); }
		multipler = -multipler; // Вычисляем модуль множителя.
	}

	// Умножаем на модуль.
	return(bn_mul_to_uint(t, (unsigned int)multipler));
}

// Возвести число в степень degree
int bn_pow_to(bn *t, int degree)
{
	if ((t == NULL) || (t->Body == NULL)) { return(BN_NULL_OBJECT); }
	if (degree == 1)
	{
		return(BN_OK);
	}
	else if (degree == 0)
	{
		return(bn_init_int(t, 1));
	}

	bn *orig = bn_init(t);
	if (orig == NULL) { return(BN_NO_MEMORY); }

	int Error = BN_OK;
	if (degree > 1)
	{
		int Error = BN_OK;
		for (int i = 0; i < degree; ++i)
		{
			bn_mul_to(t, orig);
			Error = bn_mul_to(t, orig);
			if (Error) { return(Error); }
		}
	}
	else
	{
		// TODO: реализовать после реализации деления.
	}

	Error = bn_delete(orig);
	return(Error);
}


// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
// TODO: нехорошая вещь может случиться при сравнении чисел с зарезервированной пустой памятью (размер Body не соответствует числу).
int bn_cmp(bn const *left, bn const *right)
{
	if (left->BodySize || right->BodySize)
	{
		if (left->Sign + right->Sign)
		{
			if (left->BodySize == right->BodySize)
			{
				for (size_t i = 0; i < left->BodySize; ++i)
				{
					if (left->Body[left->BodySize - i - 1] > right->Body[right->BodySize - i - 1]) { return(left->Sign); }
					else if (left->Body[left->BodySize - i - 1] == right->Body[right->BodySize - i - 1]) { continue; }
					else { return(-left->Sign); }
				}
				return(0);
			}
			else
			{
				return((left->BodySize > right->BodySize) - (right->BodySize > left->BodySize));
			}
		}
		else
		{
			return((left->Sign - right->Sign) / 2); // Хочу, чтобы было не -2 и 2, а -1 и 1.
		}
	}
	else
	{
		return(0);
	}
}
// Сравнение по модулю.
int bn_abs_cmp(bn const *left, bn const *right)
{
	if (left->BodySize || right->BodySize)
	{
		if (left->BodySize == right->BodySize)
		{
			for (size_t i = 0; i < left->BodySize; ++i)
			{
				if (left->Body[left->BodySize - i - 1] > right->Body[right->BodySize - i - 1]) { return(1); }
				else if (left->Body[left->BodySize - i - 1] == right->Body[right->BodySize - i - 1]) { continue; }
				else { return(-1); }
			}
			return(0);
		}
		else
		{
			return((left->BodySize > right->BodySize) - (right->BodySize > left->BodySize));
		}
	}
	else
	{
		return(0);
	}
}
// Изменить знак на противоположный
int bn_neg(bn *t)
{
	if (t == NULL) { return(BN_NULL_OBJECT); }
	t->Sign = -(t->Sign); //~t->Sign + 1;
	return(BN_OK);
}
// Взять модуль
int bn_abs(bn *t)
{
	if (t == NULL) { return(BN_NULL_OBJECT); }
	t->Sign = ~(t->Sign); // Переворачиваем биты.
	return(BN_OK);
}
//-1 если t<0; 0 если t = 0, 1 если t>0
int bn_sign(bn const *t)
{
	return(t->Sign);
}



int bn_print_blocks(bn const *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }
	
	printf("Number of blocks: %zu\n", t->BodySize);
	for (size_t i = 0; i < t->BodySize; ++i)
	{
		printf("%u ", t->Body[i]);
	}

	printf("\n");
	
	return(BN_OK);
}
int bn_print_formula(bn const *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	printf("Number of blocks: %zu\n", t->BodySize);

	printf("%hhd*(", t->Sign);
	for (size_t i = 0; i < t->BodySize - 1; ++i)
	{
		printf("%u*2^%zu + ", t->Body[i], i*32);
	}

	printf("%u*2^%zu) \n", t->Body[t->BodySize-1], (t->BodySize - 1) * 32);

	return(BN_OK);
}
int bn_print_hex(bn const *t)
{
	if ((t == NULL) || ((t->Body == NULL))) { return(BN_NULL_OBJECT); }

	if (!(t->Sign)) { printf("0x0"); return(0); }
	if (t->Sign < 0) { printf("-"); }

	printf("0x");
	unsigned int Mask = 0xf;
	char Digit = 0;
	char NumStarted = 0; // Закончились ли нули в начале записи?

	for (long long int i = t->BodySize; --i >= 0;)
	{
		for (long long int j = UINT_BITS - 4; j >= 0; j -= 4)
		{
			Digit = (t->Body[i] >> j) & Mask;

			if (NumStarted || Digit)
			{
				NumStarted = 1;
				if (Digit > 9) { Digit += 'A' - 10; }
				else { Digit += '0'; }
				printf("%c", Digit);
			}
		}
	}

	printf("\n");

	return(BN_OK);
}