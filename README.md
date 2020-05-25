# BigNumbers
Библиотека для работы с большими числами (проект первого семестра ФУПМ МФТИ, осень 2018).

## Спецификация
[FUPM_HomeTask1.pdf](https://www.babichev.org/mipt/FUPM_HomeTask1.pdf) - спецификация операций над большими числами.

## Начало работы
### Построение
Для построения проекта требуется CMake версии не ниже 3.7.2. Листинг команд, используемых для построения проекта из директории, находящейся на два уровня ниже файла CMakeLists.txt:
```
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```