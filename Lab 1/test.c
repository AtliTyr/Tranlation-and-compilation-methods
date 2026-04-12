#include <stdio.h>

// Объявление функции
int factorial(int n);

/* Основная функция */
int main() {
    // Объявление переменных
    int a = 5;
    int b = 10;
    int result = 0;
    int i;
    
    // Присваивание
    a = a + 2; 
    
    // Арифметические выражения
    result = a * b - 3;

    if (a > b) {
        printf("a больше b\n");
        result = a;
    } else if (a < b) {
        printf("a меньше b\n");
        result = b;
    } else {
        printf("a равно b\n");
        result = a + b;
    }
    // Вызов функции
    int num = 4;
    int fact = factorial(num);
    printf("Факториал %d = %d\n", num, fact);

    return 0;
}

/* Определение функции
    для вычисления факториала 
*/
int factorial(int n) {
    int fact = 1;
    int i;
    
    if (n < 0) {
        return -1;  // ошибка: факториал отрицательного числа не определен
    }
    
    for (i = 1; i <= n; i++) {
        fact = fact * i;
    }
    
    return fact;
}
/*