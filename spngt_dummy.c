#include "spngt.h"

void spngt_print_version_dummy(void)
{
    printf("dummy 1.0\n");
}

int spngt_decode_dummy(struct spngt_params *params)
{
    return SPNGT_ENOTSUPP;
}

int spngt_encode_dummy(struct spngt_params *params)
{
    return SPNGT_ENOTSUPP;
}
