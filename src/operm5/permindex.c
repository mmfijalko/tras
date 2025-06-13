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

#if 0
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
#endif

static void
operm5_index_to_perm(uint32_t index, uint32_t *perm, int n)
{
	uint32_t fac, val;
	int i, j, idx, div;

	for (i = 0, fac = 1; i < n; i++) {
		if (i > 0)
			fac = fac * i;
		perm[i] = i + 1;
	}

	for (i = 0; i < n; i++) {
		idx = index / fac;
		index = index - idx * fac;
		div = (n - 1 - i);
		div = (div != 0) ? div : 1;
		fac = fac / div;
		val = perm[i + idx];
		for (j = i + idx; j > i; j--)
			perm[j] = perm[j - 1];
		perm[i] = val;
	}
}

static void
operm5_int_to_perm(uint32_t *perm, int n, uint32_t value)
{
	int i;

	for (i = n - 1; i >= 0; i--) {
		perm[i] = value % 10;
		value = value / 10;
	}	
}

static int
operm5_perm_valid(uint32_t *perm, int n)
{
	int i, j;

	for (i = 0; i < n; i++) {
		if (perm[i] < 1 || perm[i] > n)
			return (0);
		for (j = i + 1; j < n; j++) {
			if (perm[i] == perm[j])
				return (0);
		}
	}
	return (1);
}

int main(void)
{
	uint32_t perm[5], index, index2;
	uint32_t idx, n = 5, i;

	for (i = 1; i < 55556; i++) {
		operm5_int_to_perm(perm, n, i);
		if (!operm5_perm_valid(perm, n))
			continue;
		printf("permutation := [");
		for (idx = 0; idx < n; idx++)
			printf("%d%s", perm[idx], (idx == (n - 1)) ? "], " : " ");
		index = operm5_perm_to_index(perm, n);
		printf("index = %d, ", index);
		operm5_index_to_perm(index, perm, n);
		printf("reverted permutation := [");
		for (idx = 0; idx < n; idx++)
			printf("%d%s", perm[idx], (idx == (n - 1)) ? "]\n, " : " ");
		index2 = operm5_perm_to_index(perm, n);
		if (index != index2) {
			printf("failed with permutation, index = %d, index2 = %d\n", index, index2);
			break;
		}
	}

	return (0);
}

