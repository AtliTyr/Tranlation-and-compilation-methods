#include <stdio.h>


int factorial(int n);


int main() {
    
    int a = 5;
    int b = 10;
    int result = 0;
    int i;
    
    
    a = a + 2; 
    
    
    result = a * b - 3;
    
    
    int isGreater = (a > b);
    int isEqual = (a == b);
    int logicalAnd = (a < b) && (result > 0);
    
    
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
    
    
    printf("Цикл for (1 до 5): ");
    for (i = 1; i <= 5; i++) {
        printf("%d ", i);
    }
    printf("\n");
    
    
    int num = 4;
    int fact = factorial(num);
    printf("Факториал %d = %d\n", num, fact);
    
    
    int x = 15, y = 20, z = 25;
    if ((x < y) && (y < z) || (x == y)) {
        printf("Условие выполнено: %d < %d < %d\n", x, y, z);
    }
    
    
    int val = 10;
    val += 5;      
    val *= 2;      
    val -= 8;      
    
    printf("Результат вычислений: result = %d, val = %d\n", result, val);
    
    return 0;
}


int factorial(int n) {
    int fact = 1;
    int i;
    
    if (n < 0) {
        return -1;  
    }
    
    for (i = 1; i <= n; i++) {
        fact = fact * i;
    }
    
    return fact;
}