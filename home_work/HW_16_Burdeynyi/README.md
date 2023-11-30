[**Назад**](https://github.com/BurdichxD4r/Cpp_Lessons/tree/master)
## Task_1

- [**Частичная специализация шаблона**](https://ravesli.com/urok-179-chastichnaya-spetsializatsiya-shablona/)

## Частичная специализация шаблона
**Частичная специализация шаблона позволяет выполнить специализацию шаблона класса (но не функции!), где некоторые (но не все) параметры шаблона явно определены**. Для нашей вышеприведенной задачи идеальное решение заключается в том, чтобы шаблон функции print() работал со StaticArray типа char, но при этом размер массива не являлся фиксированным значением, а мог варьироваться.

Вот наш шаблон функции print(), который принимает частично специализированный шаблон класса StaticArray:
```cpp
// Шаблон функции print() с частично специализированным шаблоном класса StaticArray<char, size> в качестве параметра
template <int size> // size по-прежнему является non-type параметром
void print(StaticArray<char, size> & array){ // мы здесь явно указываем тип char
	for (int count = 0; count < size; ++count)
		std::cout << array[count];
}
```
Как вы можете видеть, мы здесь явно указали тип char, но `size` оставили не фиксированным, поэтому функция print() будет работать с массивами типа char любого размера. Вот и всё!

Полный код программы:
```cpp
#include <iostream>
#include <cstring>
 
template <class T, int size> // size является non-type параметром шаблона
class StaticArray{
private:
	// Параметр size отвечает за длину массива
	T m_array[size];
 
public:
	T * getArray(){return m_array;}
 
	T & operator[](int index){
		return m_array[index];
	}
};
 
template <typename T, int size>
void print(StaticArray<T, size> & array){
	for (int count = 0; count < size; ++count)
		std::cout << array[count] << ' ';
}
 
// Шаблон функции print() с частично специализированным шаблоном класса StaticArray<char, size> в качестве параметра
template <int size>
void print(StaticArray<char, size> & array){
	for (int count = 0; count < size; ++count)
		std::cout << array[count];
}
 
int main(){
	// Объявляем массив типа char длиной 14
	StaticArray<char, 14> char14;
 
	strcpy_s(char14.getArray(), 14, "Hello, world!");
 
	// Выводим элементы массива
	print(char14);
 
	// Теперь объявляем массив типа char длиной 12
	StaticArray<char, 12> char12;
 
	strcpy_s(char12.getArray(), 12, "Hello, dad!");
 
	// Выводим элементы массива
	print(char12);
 
	return 0;
}
```
Результат:

`Hello, world! Hello, dad!`

Как и ожидалось.

Обратите внимание, начиная с C++14 частичная специализация шаблона может использоваться только с классами, но не с отдельными функциями (для функций используется только полная специализация шаблона). Наш пример `void print(StaticArray<char, size> & array)` работает только потому, что шаблон функции print() принимает в качестве параметра шаблон класса, который, в свою очередь, частично специализирован.
## Частичная специализация шаблонов методов
Ограничение частичной специализации для функций может привести к некоторым проблемам при работе с методами класса. Например, что, если бы мы определили StaticArray следующим образом:
```cpp
template <class T, int size> // size является non-type параметром шаблона
class StaticArray{
private:
    // Параметр size отвечает за длину массива
    T m_array[size];
 
public:
    T * getArray(){return m_array;}
	
    T & operator[](int index){
        return m_array[index];
    }
 
    void print(){
        for (int i = 0; i < size; i++)
            std::cout << m_array[i] << ' ';
        std::cout << "\n";
	}
};
```
Функция print() является методом класса `StaticArray<T, int>`. Что произойдет, если мы захотим частично специализировать шаблон функции print(), чтобы метод работал по-другому? Мы можем попробовать сделать следующее:
```cpp
// Не сработает
template <int size>
void StaticArray<double, size>::print(){
	for (int i = 0; i < size; i++)
		std::cout << std::scientific << m_array[i] << " ";
	std::cout << "\n";
}
```
К сожалению, это не сработает, так как мы пытаемся частично специализировать шаблон функции, что делать запрещено.

Как же это можно обойти? Одним из очевидных решений является частичная специализация шаблона всего класса:
```cpp
#include <iostream>
 
template <class T, int size> // size является non-type параметром шаблона
class StaticArray{
private:
	// Параметр size отвечает за длину массива
	T m_array[size];
 
public:
	T * getArray(){return m_array;}
 
	T & operator[](int index){
		return m_array[index];
	}

	void print(){
		for (int i = 0; i < size; i++)
			std::cout << m_array[i] << ' ';
		std::cout << "\n";
	}
};

template <int size> // size является non-type параметром шаблона
class StaticArray<double, size>{
private:
	// Параметр size отвечает за длину массива
	double m_array[size];
 
public:
	double * getArray(){return m_array;}
 
	double & operator[](int index){
		return m_array[index];
	}

	void print(){
		for (int i = 0; i < size; i++)
			std::cout << std::scientific << m_array[i] << ' ';
		std::cout << "\n";
	}
};
 
int main(){
	// Объявляем целочисленный массив длиной 5
	StaticArray<int, 5> intArray;
 
	// Заполняем массив, а затем выводим его
	for (int count = 0; count < 5; ++count)
		intArray[count] = count;
	intArray.print();
 
	// Объявляем массив типа double длиной 4
	StaticArray<double, 4> doubleArray;
 
	for (int count = 0; count < 4; ++count)
		doubleArray[count] = (4.0 + 0.1 * count);
	doubleArray.print();
 
	return 0;
}
```
Результат:

`0 1 2 3 4`<br>
`4.000000e+00 4.100000e+00 4.200000e+00 4.300000e+00`

Хотя это работает, но это не самый лучший вариант, так как у нас теперь куча дублированного кода из `StaticArray<T, size>` в `StaticArray<double, size>`.

Если бы можно было использовать код из `StaticArray<T, size>` в `StaticArray<double, size>` без дублирования. Ничего вам это не напоминает? Как по мне, то это звучит, как отличный вариант для применения **наследования**!

Вы можете начать с:
```cpp
template <int size> // size является non-type параметром шаблона
class StaticArray<double, size>: public StaticArray< // а затем что?
```
Но как мы можем ссылаться на StaticArray? Никак, но, к счастью, есть обходной путь с использованием общего родительского класса:
```cpp
#include <iostream>
 
template <class T, int size> // size является non-type параметром шаблона
class StaticArray_Base{
protected:
	// Параметр size отвечает за длину массива
	T m_array[size];
public:
	T * getArray(){return m_array;}
 
	T & operator[](int index){
		return m_array[index];
	}

	virtual void print(){
		for (int i = 0; i < size; i++)
			std::cout << m_array[i] << ' ';
		std::cout << "\n";
	}
};
 
template <class T, int size> // size является non-type параметром шаблона
class StaticArray: public StaticArray_Base<T, size>{
public:
	StaticArray(){}
};
 
template <int size> // size является non-type параметром шаблона
class StaticArray<double, size>: public StaticArray_Base<double, size>{
public:
	virtual void print() override{
		for (int i = 0; i < size; i++)
			std::cout << std::scientific << this->m_array[i] << " ";
// Примечание: Префикс this-> на вышеприведенной строке необходим. Почему? Читайте здесь - https://stackoverflow.com/a/6592617
		std::cout << "\n";
	}
};
 
int main(){
	// Объявляем целочисленный массив длиной 5
	StaticArray<int, 5> intArray;
 
	// Заполняем его, а затем выводим
	for (int count = 0; count < 5; ++count)
		intArray[count] = count;
	intArray.print();
	
	// Объявляем массив типа double длиной 4
	StaticArray<double, 4> doubleArray;
 
	// Заполняем его, а затем выводим
	for (int count = 0; count < 4; ++count)
		doubleArray[count] = (4. + 0.1 * count);
	doubleArray.print();
 
	return 0;
}
```
Результат тот же, что и в примере, приведенном выше, но дублированного кода меньше.