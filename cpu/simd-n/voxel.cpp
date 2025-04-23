#include <stdio.h>

#include "voxel.h"

Voxel::Voxel(int i, int j, int k) : i(i), j(j), k(k) {}

#ifdef DEBUG
char *Voxel::str() {
	sprintf(dbstr, "(%d, %d, %d)", i, j, k);
	return dbstr;
}
#endif
