#include "custom_def.h"

void test_div_sram(void)
{
//	printf("%s %p\n", __func__, test_div_sram);

	for(uint32_t i=0; i<TEST_LOOP_N; ++i)
	{
		__nop();
	}
}
