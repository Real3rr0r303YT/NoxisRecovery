/* Noxis Recovery (native) - device probe
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_DEVICE_H
#define NOXIS_DEVICE_H

#include <stddef.h>
#include <stdint.h>

#define NX_MAX_PARTITIONS 128

typedef struct {
    char     name[64];
    char     path[192];
    uint64_t size;
} nx_partition;

/* Active A/B slot ("_a" / "_b"). -1 when the device has no slots. */
int  nx_dev_slot(char *out, size_t n);

/* Fills out[] from /dev/block/by-name. Returns how many were found. */
int  nx_dev_partitions(nx_partition *out, int max);

const nx_partition *nx_dev_find(const nx_partition *parts, int n, const char *name);
const nx_partition *nx_dev_find_slotted(const nx_partition *parts, int n,
                                        const char *name, const char *slot);

int  nx_dev_free_space(const char *mount_point, uint64_t *free_bytes);
void nx_dev_human_size(uint64_t bytes, char *out, size_t n);
int  nx_dev_system_name(char *out, size_t n);

#endif /* NOXIS_DEVICE_H */
