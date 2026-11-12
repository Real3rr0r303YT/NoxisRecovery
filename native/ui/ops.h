/* Noxis Recovery (native) - operations
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_OPS_H
#define NOXIS_OPS_H

#include <stddef.h>
#include <stdint.h>
#include "device.h"

/* Called as work proceeds. total may be 0 when the size is not known. */
typedef void (*nx_progress_fn)(void *ud, const char *stage,
                               uint64_t done, uint64_t total);

typedef enum { NX_FS_EXT4 = 1, NX_FS_F2FS } nx_fs_type;

/* Writes image to the partition. Refuses when the image is larger than the
 * partition. Returns 0, or -1 with err filled. */
int nx_op_flash_image(const char *image, const nx_partition *target,
                      nx_progress_fn cb, void *ud, char *err, size_t errn);

/* Copies the partition to dest_dir/<name>.img. Refuses when the destination
 * has less free space than the partition holds. */
int nx_op_backup_partition(const nx_partition *src, const char *dest_dir,
                           nx_progress_fn cb, void *ud, char *err, size_t errn);

/* Makes a new filesystem. Pick the type the system expects: Android wants
 * f2fs on userdata, ext4 elsewhere. */
int nx_op_format(const nx_partition *target, nx_fs_type fs, char *err, size_t errn);

/* Zeroes the start of the partition so nothing reads a stale superblock. */
int nx_op_wipe(const nx_partition *target, char *err, size_t errn);

/* Sets which slot boots next ("a", "b", "_a", "_b"). */
int nx_op_set_active_slot(const char *slot, char *err, size_t errn);

#endif /* NOXIS_OPS_H */
