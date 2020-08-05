# ITMO-C-labs
2nd term. 'C' programming language labs.

# Лабораторная работа 1: Перекодировка
Программа должна выполнять перекодировку текстового файла.
Аргументы программе передаются через командную строку:
lab1 <имя_входного_файла> <имя_выходного_файла> <выходная_кодировка>
где <выходная_кодировка>:
* 0 – UTF-8 без BOM;
* 1 – UTF-8 с BOM;
* 2 – UTF-16 Little Endian;
* 3 – UTF-16 Big Endian;
* 4 – UTF-32 Little Endian;
* 5 – UTF-32 Big Endian.
Входная кодировка: любая из перечисленных, должна определяться программой автоматически.
Неправильные UTF-8 байты кодировать/декодировать как символы из диапазона 0xDC80..0xDCFF. UTF-8 -> другая кодировка -> UTF-8 должно точно возвращать исходный файл (плюс/минус BOM).

# Лабораторная работа 2: Сортировка
Программа должна сортировать (регистрозависимо, однобайтовая кодировка символов) строки файла в формате:
<фамилия><пробел><имя><пробел><отчество><пробел><телефон>
где:
<фамилия>, <имя>, <отчество> – строки не длиннее 20 символов, не содержат пробелов, табуляции, переводов строк или ноль-символов;
<телефон> – целое неотрицательное число, меньшее 1011.
В порядке фамилия->(если совпадает)имя->отчество->телефон.
Каждый элемент строки должен храниться отдельным полем структуры. Порядок сортировки должно быть возможно изменить (например, первым учитывать имя) незначительными изменениями в программе.

# Лабораторная работа 3: Решение системы уравнений
* Программа должна находить решение системы линейных уравнений.
* Входной файл в первой строке содержит одно число: размер системы уравнений N, после чего идут N строк по N+1 числу, которые соответствуют коэффициентам при N переменных и свободному члену.
Выходной файл должен содержать:
 * найденные значения переменных в формате одно число на каждой строке если решение единственно;
 * только фразу “many solutions” если решение не единственно;
 * только фразу “no solution” при отсутствии решений.
