class Voxel {
public:
	Voxel(int i, int j, int k);

#ifdef DEBUG
	char *str();
	char dbstr[64];
#endif

	int i;
	int j;
	int k;
};


