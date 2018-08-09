#include<stdio.h>
//#include<iostream>
int main(){
	int a = 1,b = 2,c = 3;
	printf("%d %d %d %d\n", a + b + c, b == 2 ? ++a : 0, b++, c = (c * 2));

	return 0;

}
