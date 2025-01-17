/*
 * The checkpoint map entry functions
 *
 * Copyright (C) 2018-2023, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined( _LIBFSAPFS_CHECKPOINT_MAP_ENTRY_H )
#define _LIBFSAPFS_CHECKPOINT_MAP_ENTRY_H

#include <common.h>
#include <types.h>

#include "libfsapfs_libcerror.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libfsapfs_checkpoint_map_entry libfsapfs_checkpoint_map_entry_t;

struct libfsapfs_checkpoint_map_entry
{
	/* The object identifier
	 */
	uint64_t object_identifier;

	/* The physical address
	 */
	uint64_t physical_address;
};

int libfsapfs_checkpoint_map_entry_initialize(
     libfsapfs_checkpoint_map_entry_t **checkpoint_map_entry,
     libcerror_error_t **error );

int libfsapfs_checkpoint_map_entry_free(
     libfsapfs_checkpoint_map_entry_t **checkpoint_map_entry,
     libcerror_error_t **error );

int libfsapfs_checkpoint_map_entry_read_data(
     libfsapfs_checkpoint_map_entry_t *checkpoint_map_entry,
     const uint8_t *data,
     size_t data_size,
     libcerror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif /* !defined( _LIBFSAPFS_CHECKPOINT_MAP_ENTRY_H ) */

