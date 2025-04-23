#include "offset.h"

Offset::Offset() : i(-1), j(-1), k(-1) {}

void Offset::inc() {
	k+=1;
	if (k > 1) {
		j += 1;
		k = -1;
	}
	if (j > 1) {
		i += 1;
		j = -1;
	}
}

bool Offset::done() {
	return i > 1;
}
