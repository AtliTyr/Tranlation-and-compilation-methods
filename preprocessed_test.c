#include <stdio.h>
int factorial(int n);
string helloWorld()
{
return "Hello World!\n";
}
int main() {
int a = 5;
int b = 10;
int result = 0;
int i;
string var = helloWorld();
a = a + 2;
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
int num = 4;
int fact = factorial(num);
printf("Факториал %d = %d\n", num, fact);
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