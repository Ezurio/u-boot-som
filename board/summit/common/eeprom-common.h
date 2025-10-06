#ifndef EEPROM_COMMON_H
#define EEPROM_COMMON_H

#include <linux/types.h>

int nvmem_cell_rw(const char *name, bool write, void *p, size_t size);

#endif /* EEPROM_COMMON_H */
