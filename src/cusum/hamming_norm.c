#include <stdint.h>
#include <stdio.h>

const uint8_t hamming8[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

int main(void)
{
	uint8_t c;
	int i, b, h, j, k;
	int hmax, hmin, habs;

	for (i = 0; i < 256; i++) {
		if (i != 0 && ((i % 16) == 0))
			printf("\n");
		c = (uint8_t)i;
		h = 0;
		for (j = 7; j >= 0; j--)
			h += (c & (1 << j)) ? 1 : -1;
		if (h >= 0)
			printf(" ");
		printf("%d,", h);
		if (((i + 1) % 16) != 0)
			printf(" ");
	}

	printf("\n");
	printf("cusum max\n");
	for (i = 0; i < 256; i++) {
		if (i != 0 && ((i % 16) == 0))
			printf("\n");
		for (k = 1; k <= 8; k++) {
			hmax = 0;
			h = 0;
			c = (uint8_t)i;
			for (j = 7; j >= 7 - k + 1; j--) {
				h += (c & (1 << j)) ? 1 : -1;
			}
			if (h > hmax)
				hmax = h;
		}
		if (hmax >= 0)
			printf(" ");
		printf("%d,", hmax);
		if (((i + 1) % 16) != 0)
			printf(" ");
	}
	printf("\n");
	printf("cusum min\n");
	for (i = 0; i < 256; i++) {
		if (i != 0 && ((i % 16) == 0))
			printf("\n");
		for (k = 1; k <= 8; k++) {
			hmin = 0;
			h = 0;
			c = (uint8_t)i;
			for (j = 7; j >= 7 - k + 1; j--) {
				h += (c & (1 << j)) ? 1 : -1;
			}
			if (h < hmin)
				hmin = h;
		}
		if (hmin >= 0)
			printf(" ");
		printf("%d,", hmin);
		if (((i + 1) % 16) != 0)
			printf(" ");

	}

#define	abs(a)		(((a) < 0) ? -(a) : (a))

	printf("\n");
	printf("cusum abs\n");
	for (i = 0; i < 256; i++) {
		if (i != 0 && ((i % 16) == 0))
			printf("\n");
		for (k = 1; k <= 8; k++) {
			habs = 0;
			h = 0;
			c = (uint8_t)i;
			for (j = 7; j >= 7 - k + 1; j--) {
				h += (c & (1 << j)) ? 1 : -1;
			}
			h = abs(h);
			if (h > habs)
				habs = h;
		}
		if (hmin >= 0)
			printf(" ");
		printf("%d,", habs);
		if (((i + 1) % 16) != 0)
			printf(" ");

	}

	return (0);
}
