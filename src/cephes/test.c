#include <stdio.h>
#include <igamc.h>

int main(void)
{
	int i;
	double y;

	for (i = 0; i < 20; i++) {
		y = igamc(1.5, 10.0/ ( i + 1));

	printf("y = %.16f\n", y);
}
	return (0);
}
