#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

static int
qsort_check(unsigned int *table, unsigned int n)
{
	int i;

	for (i = 0; i < n - 1; i++) {
		if (table[i] > table[i + 1]) {
printf("invalid result on positions %d, %d\n", i, i+1);
printf("<%u, %u>\n", table[i], table[i + 1]);
			return (i);
		}
	}
	return (0);
}

static void
quicksort_int(unsigned int *table, int l, int r)
{
	unsigned int p, v, t, a;
	unsigned int i, j;

	if (l >= r || l < 0)
		return;

	/* Use simple "in the middle" for pivot point */
	p = l + (r - l) / 2;
	v = table[p];
	/* Swap pivot value with last element */
	t = table[r];
	table[r] = v;
	table[p] = v;

	a = l;
	for (i = l; i < r; i++) {
		if (table[i] < v) {
			t = table[i];
			table[i] = table[a];
			table[a] = t;
			a++;
		}
	}

	t = table[a];
	table[a] = table[r];
	table[r] = t;

	quicksort_int(table, l, a - 1);
	quicksort_int(table, a + 1, r);
}

static void
quicksort(unsigned int *table, int n)
{

	quicksort_int(table, 0, n - 1);
}

int main(int argc, char *argv[])
{
	int error, id, nrd, size, n;
	unsigned int *data, index = 1;

	n = 2048;

	size = n * sizeof(unsigned int);

	data = malloc(size);
	if (data == NULL)
		return (ENOMEM);

	for (id = 1; ;) {
		nrd = read(STDIN_FILENO, data, size);
		if (nrd < 0) {
			error = errno;
			break;
		}
		if (nrd == 0) {
			error = 0;
			break;
		}

		n = min(nrd, n);
goto nouniform;
		for (nrd = 0; nrd < n; nrd++)
			data[nrd] = data[nrd] % 64;
nouniform:
		quicksort(data, n);

		error = qsort_check(data, n);
		if (error != 0) {
			for (nrd = 0; nrd < n; nrd++) {
				if (nrd == error)
					printf("|||");
				printf("%u ", ((uint32_t *)data)[nrd]);
			}
			printf("invalid result\n");
			break;
		}
		printf("test #%u: result ok\n", index);
		index++;
	}
	
	free(data);

	return (0);
}
