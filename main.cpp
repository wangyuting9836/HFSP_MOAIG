#include "PaperAlgorithm.h"

int main(int argc, char** argv)
{

	parameter_setting();
	experimental_ls_big_instance();
	experimental_ls_small_instance();
	experimental_small_instance(0.003, 0.3, 3);
	experimental_big_instance(0.003, 0.3, 3);
	experimental_Carlier_benchmark(0.003, 0.3, 3);
	return 0;
}
