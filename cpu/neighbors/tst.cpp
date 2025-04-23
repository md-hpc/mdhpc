#include <stdio.h>

#include "offset.h"

int main() {
	for (Offset ofst = Offset(); !ofst.done(); ofst.inc()) {
		printf("%d %d %d\n", ofst.i, ofst.j, ofst.k);
	}
	return 0;
}
