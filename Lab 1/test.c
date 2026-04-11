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
    
    // Логические выражения
    int isGreater = (a > b);
    int isEqual = (a == b);
    int logicalAnd = (a < b) && (result > 0);
    
    // Условный оператор
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
    
    // Цикл for
    printf("Цикл for (1 до 5): ");
    for (i = 1; i <= 5; i++) {
        printf("%d ", i);
    }
    printf("\n");
    
    // Вызов функции
    int num = 4;
    int fact = factorial(num);
    printf("Факториал %d = %d\n", num, fact);
    
    // Сложное логическое выражение в условии
    int x = 15, y = 20, z = 25;
    if ((x < y) && (y < z) || (x == y)) {
        printf("Условие выполнено: %d < %d < %d\n", x, y, z);
    }
    
    // Арифметика с присваиванием
    int val = 10;
    val += 5;      // val = 15
    val *= 2;      // val = 30
    val -= 8;      // val = 22
    
    printf("Результат вычислений: result = %d, val = %d\n", result, val);
    
    return 0;
}

/* Определение функции
    для вычисление факториала 
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