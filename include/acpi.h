#ifndef __ACPI_H
#define __ACPI_H

struct RSDPtr{
    char signature[8];
    char checksum;
    char OEMID[6];
    char revision;
    int *rsdtAddress;
};

struct FACP{
    char signature[4];
    int length;
    char unended1[40-8];
    int *DSDT;
    char unended[48-40];
    int *SMI_CMD;
    char ACPI_enable;
    char ACPI_disable;
    char unended3[64-54];
    int *PM1a_CNT_BLK;
    int *PM1b_CNT_BLK;
    char unended4[89-72];
    char PM1_CNT_LEN;
};

#endif

