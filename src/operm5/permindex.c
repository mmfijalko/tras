#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


static int
operm5_perm_to_index(uint32_t *perm, int n)
{
	uint32_t fac, dsc, index;
	int i, j;

	index = 0;
	for (i = 0, fac = n - 1; i < n - 1; i++, fac--) {
		for (j = i, dsc = 0; j < n - 1; j++) {
			if (perm[i] > perm[j + 1])
				dsc++;
		}
		index = (index + dsc) * fac;
	}
	return (index);
}

static void
operm5_index_to_perm(uint32_t *perm, int n, uint32_t index)
{
	uint32_t fac, div;
	int i, j;

	for (i = 1, fac = 1; i < n; i++)
		fac = fac * i;

	div = n - 2;
	for (i = 0; i < n; i++) {
		j = index / fac;
		printf("j = %d\n", j);
		index = index - j * fac;
		printf("index = %d\n", index);
		if (j == 0)
			perm[i] = i;
		else
			perm[i] = j;
		fac = fac / ((div == 0) ? 1 : div);
		div--;
	}
}

int main(void)
{
	uint32_t perm[5] = {1, 2, 3, 4, 5}, index;
	uint32_t idx, n = 5, i;

	for (idx = 0; idx < 120; idx++) {
		operm5_index_to_perm(perm, 5, idx);
		index = operm5_perm_to_index(perm, 5);
		printf("(");
		for (i = 0; i < 5; i++) {
			if (i != 0)
				printf(", ");
			printf("%d", perm[i]);
		}
		printf("), index = %d\n", index);
	}

	return (0);
}

