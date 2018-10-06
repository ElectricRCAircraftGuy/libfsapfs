/*
 * Info handle
 *
 * Copyright (C) 2018, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <file_stream.h>
#include <memory.h>
#include <narrow_string.h>
#include <system_string.h>
#include <types.h>
#include <wide_string.h>

#include "fsapfstools_libbfio.h"
#include "fsapfstools_libcerror.h"
#include "fsapfstools_libclocale.h"
#include "fsapfstools_libcnotify.h"
#include "fsapfstools_libcpath.h"
#include "fsapfstools_libfdatetime.h"
#include "fsapfstools_libfguid.h"
#include "fsapfstools_libfsapfs.h"
#include "info_handle.h"

#if !defined( LIBFSAPFS_HAVE_BFIO )

extern \
int libfsapfs_container_open_file_io_handle(
     libfsapfs_container_t *container,
     libbfio_handle_t *file_io_handle,
     int access_flags,
     libfsapfs_error_t **error );

#endif /* !defined( LIBFSAPFS_HAVE_BFIO ) */

#define INFO_HANDLE_NOTIFY_STREAM	stdout

/* Copies a string of a decimal value to a 64-bit value
 * Returns 1 if successful or -1 on error
 */
int fsapfstools_system_string_copy_from_64_bit_in_decimal(
     const system_character_t *string,
     size_t string_size,
     uint64_t *value_64bit,
     libcerror_error_t **error )
{
	static char *function              = "fsapfstools_system_string_copy_from_64_bit_in_decimal";
	size_t string_index                = 0;
	system_character_t character_value = 0;
	uint8_t maximum_string_index       = 20;
	int8_t sign                        = 1;

	if( string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid string.",
		 function );

		return( -1 );
	}
	if( string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( value_64bit == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid value 64-bit.",
		 function );

		return( -1 );
	}
	*value_64bit = 0;

	if( string[ string_index ] == (system_character_t) '-' )
	{
		string_index++;
		maximum_string_index++;

		sign = -1;
	}
	else if( string[ string_index ] == (system_character_t) '+' )
	{
		string_index++;
		maximum_string_index++;
	}
	while( string_index < string_size )
	{
		if( string[ string_index ] == 0 )
		{
			break;
		}
		if( string_index > (size_t) maximum_string_index )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_LARGE,
			 "%s: string too large.",
			 function );

			return( -1 );
		}
		*value_64bit *= 10;

		if( ( string[ string_index ] >= (system_character_t) '0' )
		 && ( string[ string_index ] <= (system_character_t) '9' ) )
		{
			character_value = (system_character_t) ( string[ string_index ] - (system_character_t) '0' );
		}
		else
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported character value: %" PRIc_SYSTEM " at index: %d.",
			 function,
			 string[ string_index ],
			 string_index );

			return( -1 );
		}
		*value_64bit += character_value;

		string_index++;
	}
	if( sign == -1 )
	{
		*value_64bit *= (uint64_t) -1;
	}
	return( 1 );
}

/* Creates an info handle
 * Make sure the value info_handle is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int info_handle_initialize(
     info_handle_t **info_handle,
     libcerror_error_t **error )
{
	static char *function = "info_handle_initialize";

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( *info_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid info handle value already set.",
		 function );

		return( -1 );
	}
	*info_handle = memory_allocate_structure(
	                info_handle_t );

	if( *info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create info handle.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *info_handle,
	     0,
	     sizeof( info_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear info handle.",
		 function );

		goto on_error;
	}
	if( libbfio_file_range_initialize(
	     &( ( *info_handle )->input_file_io_handle ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize input file IO handle.",
		 function );

		goto on_error;
	}
	if( libfsapfs_container_initialize(
	     &( ( *info_handle )->input_container ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize input container.",
		 function );

		goto on_error;
	}
	( *info_handle )->notify_stream = INFO_HANDLE_NOTIFY_STREAM;

	return( 1 );

on_error:
	if( *info_handle != NULL )
	{
		if( ( *info_handle )->input_file_io_handle != NULL )
		{
			libbfio_handle_free(
			 &( ( *info_handle )->input_file_io_handle ),
			 NULL );
		}
		memory_free(
		 *info_handle );

		*info_handle = NULL;
	}
	return( -1 );
}

/* Frees an info handle
 * Returns 1 if successful or -1 on error
 */
int info_handle_free(
     info_handle_t **info_handle,
     libcerror_error_t **error )
{
	static char *function = "info_handle_free";
	int result            = 1;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( *info_handle != NULL )
	{
		if( libfsapfs_container_free(
		     &( ( *info_handle )->input_container ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free input container.",
			 function );

			result = -1;
		}
		if( libbfio_handle_free(
		     &( ( *info_handle )->input_file_io_handle ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free input file IO handle.",
			 function );

			result = -1;
		}
		if( ( *info_handle )->user_password != NULL )
		{
			if( memory_set(
			     ( *info_handle )->user_password,
			     0,
			     ( *info_handle )->user_password_size ) == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_SET_FAILED,
				 "%s: unable to clear user password.",
				 function );

				result = -1;
			}
			memory_free(
			 ( *info_handle )->user_password );

			( *info_handle )->user_password      = NULL;
			( *info_handle )->user_password_size = 0;
		}
		memory_free(
		 *info_handle );

		*info_handle = NULL;
	}
	return( result );
}

/* Signals the info handle to abort
 * Returns 1 if successful or -1 on error
 */
int info_handle_signal_abort(
     info_handle_t *info_handle,
     libcerror_error_t **error )
{
	static char *function = "info_handle_signal_abort";

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	info_handle->abort = 1;

	if( info_handle->input_container != NULL )
	{
		if( libfsapfs_container_signal_abort(
		     info_handle->input_container,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to signal input container to abort.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Sets the password
 * Returns 1 if successful or -1 on error
 */
int info_handle_set_password(
     info_handle_t *info_handle,
     const system_character_t *string,
     libcerror_error_t **error )
{
	static char *function = "info_handle_set_password";
	size_t string_length  = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->user_password != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid info handle - user password value already set.",
		 function );

		return( -1 );
	}
	if( string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid string.",
		 function );

		return( -1 );
	}
	string_length = system_string_length(
	                 string );

	info_handle->user_password_size = string_length + 1;

	info_handle->user_password = system_string_allocate(
	                              info_handle->user_password_size );

	if( info_handle->user_password == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create user password.",
		 function );

		goto on_error;
	}
	if( system_string_copy(
	     info_handle->user_password,
	     string,
	     info_handle->user_password_size ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy user password.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( info_handle->user_password != NULL )
	{
		memory_free(
		 info_handle->user_password );

		info_handle->user_password = NULL;
	}
	info_handle->user_password_size = 0;

	return( -1 );
}

/* Sets the volume offset
 * Returns 1 if successful or -1 on error
 */
int info_handle_set_volume_offset(
     info_handle_t *info_handle,
     const system_character_t *string,
     libcerror_error_t **error )
{
	static char *function = "info_handle_set_volume_offset";
	size_t string_length  = 0;
	uint64_t value_64bit  = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	string_length = system_string_length(
	                 string );

	if( fsapfstools_system_string_copy_from_64_bit_in_decimal(
	     string,
	     string_length + 1,
	     &value_64bit,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy string to 64-bit decimal.",
		 function );

		return( -1 );
	}
	info_handle->volume_offset = (off64_t) value_64bit;

	return( 1 );
}

/* Opens the input
 * Returns 1 if successful or -1 on error
 */
int info_handle_open_input(
     info_handle_t *info_handle,
     const system_character_t *filename,
     libcerror_error_t **error )
{
	static char *function  = "info_handle_open_input";
	size_t filename_length = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	filename_length = system_string_length(
	                   filename );

#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
	if( libbfio_file_range_set_name_wide(
	     info_handle->input_file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
#else
	if( libbfio_file_range_set_name(
	     info_handle->input_file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
#endif
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open set file name.",
		 function );

		return( -1 );
	}
	if( libbfio_file_range_set(
	     info_handle->input_file_io_handle,
	     info_handle->volume_offset,
	     0,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open set volume offset.",
		 function );

		return( -1 );
	}
	if( libfsapfs_container_open_file_io_handle(
	     info_handle->input_container,
	     info_handle->input_file_io_handle,
	     LIBFSAPFS_OPEN_READ,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open input container.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Closes the input
 * Returns the 0 if succesful or -1 on error
 */
int info_handle_close_input(
     info_handle_t *info_handle,
     libcerror_error_t **error )
{
	static char *function = "info_handle_close_input";

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( libfsapfs_container_close(
	     info_handle->input_container,
	     error ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_CLOSE_FAILED,
		 "%s: unable to close input container.",
		 function );

		return( -1 );
	}
	return( 0 );
}

/* Prints a POSIX value
 * Returns 1 if successful or -1 on error
 */
int info_handle_posix_time_value_fprint(
     info_handle_t *info_handle,
     const char *value_name,
     uint64_t value_64bit,
     libcerror_error_t **error )
{
	system_character_t date_time_string[ 32 ];

	libfdatetime_posix_time_t *posix_time = NULL;
	static char *function                 = "info_handle_posix_time_fprint";
	int result                            = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( value_64bit == 0 )
	{
		fprintf(
		 info_handle->notify_stream,
		 "%s: Not set (0)\n",
		 value_name );
	}
	else
	{
		if( libfdatetime_posix_time_initialize(
		     &posix_time,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create POSIX time.",
			 function );

			goto on_error;
		}
		if( libfdatetime_posix_time_copy_from_64bit(
		     posix_time,
		     value_64bit,
		     LIBFDATETIME_POSIX_TIME_VALUE_TYPE_NANO_SECONDS_64BIT_SIGNED,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to copy POSIX time from 64-bit.",
			 function );

			goto on_error;
		}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfdatetime_posix_time_copy_to_utf16_string(
			  posix_time,
			  (uint16_t *) date_time_string,
			  32,
			  LIBFDATETIME_STRING_FORMAT_TYPE_ISO8601 | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
			  error );
#else
		result = libfdatetime_posix_time_copy_to_utf8_string(
			  posix_time,
			  (uint8_t *) date_time_string,
			  32,
			  LIBFDATETIME_STRING_FORMAT_TYPE_ISO8601 | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to copy POSIX time to string.",
			 function );

			goto on_error;
		}
		fprintf(
		 info_handle->notify_stream,
		 "%s: %" PRIs_SYSTEM "Z\n",
		 value_name,
		 date_time_string );

		if( libfdatetime_posix_time_free(
		     &posix_time,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free POSIX time.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( posix_time != NULL )
	{
		libfdatetime_posix_time_free(
		 &posix_time,
		 NULL );
	}
	return( -1 );
}

/* Prints an UUID value
 * Returns 1 if successful or -1 on error
 */
int info_handle_uuid_value_fprint(
     info_handle_t *info_handle,
     const char *value_name,
     const uint8_t *uuid_data,
     libcerror_error_t **error )
{
	system_character_t uuid_string[ 48 ];

	libfguid_identifier_t *uuid = NULL;
	static char *function       = "info_handle_uuid_value_fprint";
	int result                  = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( libfguid_identifier_initialize(
	     &uuid,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create UUID.",
		 function );

		goto on_error;
	}
	if( libfguid_identifier_copy_from_byte_stream(
	     uuid,
	     uuid_data,
	     16,
	     LIBFGUID_ENDIAN_BIG,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy byte stream to UUID.",
		 function );

		goto on_error;
	}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfguid_identifier_copy_to_utf16_string(
		  uuid,
		  (uint16_t *) uuid_string,
		  48,
		  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
		  error );
#else
	result = libfguid_identifier_copy_to_utf8_string(
		  uuid,
		  (uint8_t *) uuid_string,
		  48,
		  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
		  error );
#endif
	if( result != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy UUID to string.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "%s: %" PRIs_SYSTEM "\n",
	 value_name,
	 uuid_string );

	if( libfguid_identifier_free(
	     &uuid,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free UUID.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( uuid != NULL )
	{
		libfguid_identifier_free(
		 &uuid,
		 NULL );
	}
	return( -1 );
}

/* Prints a file entry value
 * Returns 1 if successful, 0 if not or -1 on error
 */
int info_handle_file_entry_value_fprint(
     info_handle_t *info_handle,
     libfsapfs_file_entry_t *file_entry,
     libcerror_error_t **error )
{
	char file_mode_string[ 11 ]         = { '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', 0 };

	system_character_t *file_entry_name = NULL;
	static char *function               = "info_handle_file_entry_value_fprint";
	size_t file_entry_name_size         = 0;
	uint64_t access_time                = 0;
	uint64_t creation_time              = 0;
	uint64_t identifier                 = 0;
	uint64_t inode_change_time          = 0;
	uint64_t modification_time          = 0;
	uint32_t group_identifier           = 0;
	uint32_t owner_identifier           = 0;
	uint16_t file_mode                  = 0;
	int result                          = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( libfsapfs_file_entry_get_identifier(
	     file_entry,
	     &identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve identifier.",
		 function );

		goto on_error;
	}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfsapfs_file_entry_get_utf16_name_size(
	          file_entry,
	          &file_entry_name_size,
	          error );
#else
	result = libfsapfs_file_entry_get_utf8_name_size(
	          file_entry,
	          &file_entry_name_size,
	          error );
#endif
	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file entry name string size.",
		 function );

		goto on_error;
	}
	if( ( result == 1 )
	 && ( file_entry_name_size > 0 ) )
	{
		file_entry_name = system_string_allocate(
		                   file_entry_name_size );

		if( file_entry_name == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create file entry name string.",
			 function );

			goto on_error;
		}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfsapfs_file_entry_get_utf16_name(
		          file_entry,
		          (uint16_t *) file_entry_name,
		          file_entry_name_size,
		          error );
#else
		result = libfsapfs_file_entry_get_utf8_name(
		          file_entry,
		          (uint8_t *) file_entry_name,
		          file_entry_name_size,
		          error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve file entry name string.",
			 function );

			goto on_error;
		}
	}
	if( libfsapfs_file_entry_get_creation_time(
	     file_entry,
	     &creation_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve creation time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_modification_time(
	     file_entry,
	     &modification_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve modification time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_inode_change_time(
	     file_entry,
	     &inode_change_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve inode change time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_access_time(
	     file_entry,
	     &access_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve access time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_owner_identifier(
	     file_entry,
	     &owner_identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve owner identifier.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_group_identifier(
	     file_entry,
	     &group_identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve group identifier.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_file_mode(
	     file_entry,
	     &file_mode,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file mode.",
		 function );

		goto on_error;
	}
/* TODO move into function */
	if( ( file_mode & 0x0001 ) != 0 )
	{
		file_mode_string[ 9 ] = 'x';
	}
	if( ( file_mode & 0x0002 ) != 0 )
	{
		file_mode_string[ 8 ] = 'w';
	}
	if( ( file_mode & 0x0004 ) != 0 )
	{
		file_mode_string[ 7 ] = 'r';
	}
	if( ( file_mode & 0x0008 ) != 0 )
	{
		file_mode_string[ 6 ] = 'x';
	}
	if( ( file_mode & 0x0010 ) != 0 )
	{
		file_mode_string[ 5 ] = 'w';
	}
	if( ( file_mode & 0x0020 ) != 0 )
	{
		file_mode_string[ 4 ] = 'r';
	}
	if( ( file_mode & 0x0040 ) != 0 )
	{
		file_mode_string[ 3 ] = 'x';
	}
	if( ( file_mode & 0x0080 ) != 0 )
	{
		file_mode_string[ 2 ] = 'w';
	}
	if( ( file_mode & 0x0100 ) != 0 )
	{
		file_mode_string[ 1 ] = 'r';
	}
	switch( file_mode & 0xf000 )
	{
		case 0x1000:
			file_mode_string[ 0 ] = 'p';
			break;

		case 0x2000:
			file_mode_string[ 0 ] = 'c';
			break;

		case 0x4000:
			file_mode_string[ 0 ] = 'd';
			break;

		case 0x6000:
			file_mode_string[ 0 ] = 'b';
			break;

		case 0xa000:
			file_mode_string[ 0 ] = 'l';
			break;

		case 0xc000:
			file_mode_string[ 0 ] = 's';
			break;

		default:
			break;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\tIdentifier\t\t: %" PRIu64 "\n",
	 identifier );

	if( file_entry_name != NULL )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\tName\t\t\t: %" PRIs_SYSTEM "\n",
		 file_entry_name );
	}
	if( info_handle_posix_time_value_fprint(
	     info_handle,
	     "\tCreation time\t\t",
	     creation_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print POSIX time value.",
		 function );

		goto on_error;
	}
	if( info_handle_posix_time_value_fprint(
	     info_handle,
	     "\tModification time\t",
	     modification_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print POSIX time value.",
		 function );

		goto on_error;
	}
	if( info_handle_posix_time_value_fprint(
	     info_handle,
	     "\tInode change time\t",
	     inode_change_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print POSIX time value.",
		 function );

		goto on_error;
	}
	if( info_handle_posix_time_value_fprint(
	     info_handle,
	     "\tAccess time\t\t",
	     access_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print POSIX time value.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\tOwner identifier\t: %" PRIu32 "\n",
	 owner_identifier );

	fprintf(
	 info_handle->notify_stream,
	 "\tGroup identifier\t: %" PRIu32 "\n",
	 group_identifier );

	fprintf(
	 info_handle->notify_stream,
	 "\tFile mode\t\t: %s (%07" PRIo16 ")\n",
	 file_mode_string,
	 file_mode );

	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );

		file_entry_name = NULL;
	}
	return( 1 );

on_error:
	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );
	}
	return( -1 );
}

/* Prints file entry information as part of the file system hierarchy
 * Returns 1 if successful or -1 on error
 */
int info_handle_file_system_hierarchy_fprint_file_entry(
     info_handle_t *info_handle,
     libfsapfs_file_entry_t *file_entry,
     const system_character_t *path,
     int level,
     libcerror_error_t **error )
{
	libfsapfs_file_entry_t *sub_file_entry = NULL;
	system_character_t *file_entry_name    = NULL;
	system_character_t *sub_path           = NULL;
	static char *function                  = "info_handle_file_system_hierarchy_fprint_file_entry";
	size_t file_entry_name_size            = 0;
	size_t path_length                     = 0;
	size_t sub_path_size                   = 0;
	int number_of_sub_file_entries         = 0;
	int result                             = 0;
	int sub_file_entry_index               = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( path == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid path.",
		 function );

		return( -1 );
	}
	if( libfsapfs_file_entry_get_number_of_sub_file_entries(
	     file_entry,
	     &number_of_sub_file_entries,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of sub file entries.",
		 function );

		goto on_error;
	}
	path_length = system_string_length(
	               path );

	if( level > 0 )
	{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfsapfs_file_entry_get_utf16_name_size(
		          file_entry,
		          &file_entry_name_size,
		          error );
#else
		result = libfsapfs_file_entry_get_utf8_name_size(
		          file_entry,
		          &file_entry_name_size,
		          error );
#endif
		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve file entry name string size.",
			 function );

			goto on_error;
		}
		if( ( result == 1 )
		 && ( file_entry_name_size > 0 ) )
		{
			file_entry_name = system_string_allocate(
			                   file_entry_name_size );

			if( file_entry_name == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				 "%s: unable to create file entry name string.",
				 function );

				goto on_error;
			}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfsapfs_file_entry_get_utf16_name(
			          file_entry,
			          (uint16_t *) file_entry_name,
			          file_entry_name_size,
			          error );
#else
			result = libfsapfs_file_entry_get_utf8_name(
			          file_entry,
			          (uint8_t *) file_entry_name,
			          file_entry_name_size,
			          error );
#endif
			if( result != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve file entry name string.",
				 function );

				goto on_error;
			}
		}
	}
	fprintf(
	 info_handle->notify_stream,
	 "%" PRIs_SYSTEM "",
	 path );

	if( file_entry_name != NULL )
	{
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIs_SYSTEM "",
		 file_entry_name );
	}
	fprintf(
	 info_handle->notify_stream,
	 "\n" );

	if( number_of_sub_file_entries > 0 )
	{
		sub_path_size = path_length + file_entry_name_size + 1;

		sub_path = system_string_allocate(
		            sub_path_size );

		if( sub_path == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create sub path.",
			 function );

			goto on_error;
		}
		if( system_string_copy(
		     sub_path,
		     path,
		     path_length ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy path to sub path.",
			 function );

			goto on_error;
		}
		if( file_entry_name != NULL )
		{
			if( system_string_copy(
			     &( sub_path[ path_length ] ),
			     file_entry_name,
			     file_entry_name_size - 1 ) == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
				 "%s: unable to copy file entry name to sub path.",
				 function );

				goto on_error;
			}
			sub_path[ sub_path_size - 2 ] = (system_character_t) '/';
		}
		sub_path[ sub_path_size - 1 ] = (system_character_t) 0;

		for( sub_file_entry_index = 0;
		     sub_file_entry_index < number_of_sub_file_entries;
		     sub_file_entry_index++ )
		{
			if( libfsapfs_file_entry_get_sub_file_entry_by_index(
			     file_entry,
			     sub_file_entry_index,
			     &sub_file_entry,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve sub file entry: %d.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
			if( info_handle_file_system_hierarchy_fprint_file_entry(
			     info_handle,
			     sub_file_entry,
			     sub_path,
			     level + 1,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print file entry: %d information.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
			if( libfsapfs_file_entry_free(
			     &sub_file_entry,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free sub file entry: %d.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
		}
	}
	if( sub_path != NULL )
	{
		memory_free(
		 sub_path );

		sub_path = NULL;
	}
	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );

		file_entry_name = NULL;
	}
	return( 1 );

on_error:
	if( sub_file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &sub_file_entry,
		 NULL );
	}
	if( sub_path != NULL )
	{
		memory_free(
		 sub_path );
	}
	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );
	}
	return( -1 );
}

/* Prints the file system hierarchy information
 * Returns 1 if successful or -1 on error
 */
int info_handle_file_system_hierarchy_fprint(
     info_handle_t *info_handle,
     libcerror_error_t **error )
{
	system_character_t uuid_string[ 48 ];
	uint8_t uuid_data[ 16 ];

	libfguid_identifier_t *uuid        = NULL;
	libfsapfs_file_entry_t *file_entry = NULL;
	libfsapfs_volume_t *volume         = NULL;
	static char *function              = "info_handle_file_system_hierarchy_fprint";
	int number_of_volumes              = 0;
	int result                         = 0;
	int volume_index                   = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "Apple File System (APFS) information:\n\n" );

	fprintf(
	 info_handle->notify_stream,
	 "File system hierarchy:\n" );

	if( libfsapfs_container_get_number_of_volumes(
	     info_handle->input_container,
	     &number_of_volumes,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of volumes.",
		 function );

		goto on_error;
	}
	for( volume_index = 0;
	     volume_index < number_of_volumes;
	     volume_index++ )
	{
		if( libfsapfs_container_get_volume_by_index(
		     info_handle->input_container,
		     volume_index,
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( info_handle->user_password != NULL )
		{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
			if( libfsapfs_volume_set_utf16_password(
			     volume,
			     (uint16_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#else
			if( libfsapfs_volume_set_utf8_password(
			     volume,
			     (uint8_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#endif
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set password.",
				 function );

				goto on_error;
			}
/* TODO call unlock volume */
		}
		if( libfsapfs_volume_get_identifier(
		     volume,
		     uuid_data,
		     16,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume identifier.",
			 function );

			goto on_error;
		}
		if( libfguid_identifier_initialize(
		     &uuid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create UUID.",
			 function );

			goto on_error;
		}
		if( libfguid_identifier_copy_from_byte_stream(
		     uuid,
		     uuid_data,
		     16,
		     LIBFGUID_ENDIAN_BIG,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy byte stream to UUID.",
			 function );

			goto on_error;
		}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfguid_identifier_copy_to_utf16_string(
			  uuid,
			  (uint16_t *) &( uuid_string[ 1 ] ),
			  48 - 1,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE | LIBFGUID_STRING_FORMAT_FLAG_USE_SURROUNDING_BRACES,
			  error );
#else
		result = libfguid_identifier_copy_to_utf8_string(
			  uuid,
			  (uint8_t *) &( uuid_string[ 1 ] ),
			  48 - 1,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE | LIBFGUID_STRING_FORMAT_FLAG_USE_SURROUNDING_BRACES,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy UUID to string.",
			 function );

			goto on_error;
		}
		uuid_string[ 0 ]  = (system_character_t) '/';
		uuid_string[ 39 ] = (system_character_t) '/';
		uuid_string[ 40 ] = (system_character_t) 0;

		if( libfguid_identifier_free(
		     &uuid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free UUID.",
			 function );

			goto on_error;
		}
		if( libfsapfs_volume_get_root_directory(
		     volume,
		     &file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve root directory file entry from volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( info_handle_file_system_hierarchy_fprint_file_entry(
		     info_handle,
		     file_entry,
		     uuid_string,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print root directory file entry information.",
			 function );

			goto on_error;
		}
		if( libfsapfs_file_entry_free(
		     &file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file entry.",
			 function );

			goto on_error;
		}
		if( libfsapfs_volume_free(
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free volume.",
			 function );

			goto on_error;
		}
	}
	fprintf(
	 info_handle->notify_stream,
	 "\n" );

	return( 1 );

on_error:
	if( file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &file_entry,
		 NULL );
	}
	if( uuid != NULL )
	{
		libfguid_identifier_free(
		 &uuid,
		 NULL );
	}
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

/* Prints file entry information as part of the inode information
 * Returns 1 if successful, 0 if not or -1 on error
 */
int info_handle_inode_fprint_file_entry(
     info_handle_t *info_handle,
     libfsapfs_volume_t *volume,
     int volume_index,
     uint64_t inode_number,
     libcerror_error_t **error )
{
	libfsapfs_file_entry_t *file_entry = NULL;
	static char *function              = "info_handle_inode_fprint_file_entry";
	int result                         = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	result = libfsapfs_volume_get_file_entry_by_identifier(
	          volume,
	          inode_number,
	          &file_entry,
	          error );

	if( result == -1 )
	{
		if( ( error != NULL )
		 && ( *error != NULL ) )
		{
			libcnotify_print_error_backtrace(
			 *error );
		}
		libcerror_error_free(
		 error );

		fprintf(
		 info_handle->notify_stream,
		 "Error reading inode: %" PRIu64 "\n\n",
		 inode_number );

		return( 0 );
	}
	else if( result == 0 )
	{
		return( 0 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "Inode: %" PRIu64 ":\n",
	 inode_number );

	if( info_handle_file_entry_value_fprint(
	     info_handle,
	     file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print file entry.",
		 function );

		return( -1 );
	}
	if( libfsapfs_file_entry_free(
	     &file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free file entry.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\n" );

	return( 1 );

on_error:
	if( file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &file_entry,
		 NULL );
	}
	return( -1 );
}

/* Prints the inode information
 * Returns 1 if successful, 0 if not or -1 on error
 */
int info_handle_inode_fprint(
     info_handle_t *info_handle,
     uint64_t inode_number,
     libcerror_error_t **error )
{
	libfsapfs_volume_t *volume = NULL;
	static char *function      = "info_handle_inode_fprint";

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
/* TODO add multi volume support */
	if( libfsapfs_container_get_volume_by_index(
	     info_handle->input_container,
	     0,
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume: %d.",
		 function,
		 0 );

		goto on_error;
	}
	if( info_handle->user_password != NULL )
	{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		if( libfsapfs_volume_set_utf16_password(
		     volume,
		     (uint16_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#else
		if( libfsapfs_volume_set_utf8_password(
		     volume,
		     (uint8_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#endif
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set password.",
			 function );

			goto on_error;
		}
/* TODO call unlock volume */
	}
	fprintf(
	 info_handle->notify_stream,
	 "Apple File System (APFS) information:\n\n" );

	if( info_handle_inode_fprint_file_entry(
	     info_handle,
	     volume,
	     0,
	     inode_number,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print inode: %" PRIu64 ".",
		 function,
		 inode_number );

		return( -1 );
	}
	if( libfsapfs_volume_free(
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free volume.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

/* Prints the inodes information
 * Returns 1 if successful or -1 on error
 */
int info_handle_inodes_fprint(
     info_handle_t *info_handle,
     libcerror_error_t **error )
{
	libfsapfs_volume_t *volume          = NULL;
	static char *function               = "info_handle_inodes_fprint";
	uint64_t inode_number               = 0;
	uint64_t next_file_entry_identifier = 0;
	int result                          = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
/* TODO add multi volume support */
	if( libfsapfs_container_get_volume_by_index(
	     info_handle->input_container,
	     0,
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume: %d.",
		 function,
		 0 );

		goto on_error;
	}
	if( info_handle->user_password != NULL )
	{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		if( libfsapfs_volume_set_utf16_password(
		     volume,
		     (uint16_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#else
		if( libfsapfs_volume_set_utf8_password(
		     volume,
		     (uint8_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#endif
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set password.",
			 function );

			goto on_error;
		}
/* TODO call unlock volume */
	}
	if( libfsapfs_volume_get_next_file_entry_identifier(
	     volume,
	     &next_file_entry_identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve next file entry identifier.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "Apple File System (APFS) information:\n\n" );

	for( inode_number = 0;
	     inode_number < next_file_entry_identifier;
	     inode_number++ )
	{
		result = info_handle_inode_fprint_file_entry(
		          info_handle,
		          volume,
		          0,
		          inode_number,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print inode: %" PRIu64 ".",
			 function,
			 inode_number );

			return( -1 );
		}
	}
	if( libfsapfs_volume_free(
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free volume.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

/* Prints the file entry information
 * Returns 1 if successful or -1 on error
 */
int info_handle_file_entry_fprint(
     info_handle_t *info_handle,
     const system_character_t *path,
     libcerror_error_t **error )
{
	libfsapfs_file_entry_t *file_entry = NULL;
	libfsapfs_volume_t *volume         = NULL;
	static char *function              = "info_handle_file_entry_fprint";
	size_t path_length                 = 0;
	int result                         = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
/* TODO add multi volume support */
	if( libfsapfs_container_get_volume_by_index(
	     info_handle->input_container,
	     0,
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume: %d.",
		 function,
		 0 );

		goto on_error;
	}
	if( info_handle->user_password != NULL )
	{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		if( libfsapfs_volume_set_utf16_password(
		     volume,
		     (uint16_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#else
		if( libfsapfs_volume_set_utf8_password(
		     volume,
		     (uint8_t *) info_handle->user_password,
		     info_handle->user_password_size - 1,
		     error ) != 1 )
#endif
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set password.",
			 function );

			goto on_error;
		}
/* TODO call unlock volume */
	}
	path_length = system_string_length(
	               path );

#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfsapfs_volume_get_file_entry_by_utf16_path(
	          volume,
	          (uint16_t *) path,
	          path_length,
	          &file_entry,
	          error );
#else
	result = libfsapfs_volume_get_file_entry_by_utf8_path(
	          volume,
	          (uint8_t *) path,
	          path_length,
	          &file_entry,
	          error );
#endif
	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file entry.",
		 function );

		goto on_error;
	}
	else if( result == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: file entry not found.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "Apple File System (APFS) information:\n\n" );

	fprintf(
	 info_handle->notify_stream,
	 "File entry:\n" );

	fprintf(
	 info_handle->notify_stream,
	 "\tPath\t\t\t: %" PRIs_SYSTEM "\n",
	 path );

	if( info_handle_file_entry_value_fprint(
	     info_handle,
	     file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print file entry.",
		 function );

		return( -1 );
	}
	if( libfsapfs_file_entry_free(
	     &file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free file entry.",
		 function );

		goto on_error;
	}
	if( libfsapfs_volume_free(
	     &volume,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free volume.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\n" );

	return( 1 );

on_error:
	if( file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &file_entry,
		 NULL );
	}
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

/* Prints the volume information
 * Returns 1 if successful or -1 on error
 */
int info_handle_volume_fprint(
     info_handle_t *info_handle,
     libfsapfs_volume_t *volume,
     int volume_index,
     libcerror_error_t **error )
{
	uint8_t uuid_data[ 16 ];

	system_character_t *volume_name = NULL;
	static char *function           = "info_handle_volume_fprint";
	size_t volume_name_size         = 0;
	int result                      = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "\nVolume: %d information:\n",
	 volume_index );

	if( libfsapfs_volume_get_identifier(
	     volume,
	     uuid_data,
	     16,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume identifier.",
		 function );

		goto on_error;
	}
	if( info_handle_uuid_value_fprint(
	     info_handle,
	     "\tIdentifier\t\t",
	     uuid_data,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print UUID value.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\tName\t\t\t: " );

#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfsapfs_volume_get_utf16_name_size(
	          volume,
	          &volume_name_size,
	          error );
#else
	result = libfsapfs_volume_get_utf8_name_size(
	          volume,
	          &volume_name_size,
	          error );
#endif
	if( result != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume name size.",
		 function );

		goto on_error;
	}
	if( volume_name_size > 0 )
	{
		volume_name = system_string_allocate(
		               volume_name_size );

		if( volume_name == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create volume name string.",
			 function );

			goto on_error;
		}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfsapfs_volume_get_utf16_name(
		          volume,
		          (uint16_t *) volume_name,
		          volume_name_size,
		          error );
#else
		result = libfsapfs_volume_get_utf8_name(
		          volume,
		          (uint8_t *) volume_name,
		          volume_name_size,
		          error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume name.",
			 function );

			goto on_error;
		}
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIs_SYSTEM "",
		 volume_name );

		memory_free(
		 volume_name );

		volume_name = NULL;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\n" );

/* TODO print additional volume information such as size */

	fprintf(
	 info_handle->notify_stream,
	 "\n" );

	return( 1 );

on_error:
	if( volume_name != NULL )
	{
		memory_free(
		 volume_name );
	}
	return( -1 );
}

/* Prints the container information
 * Returns 1 if successful or -1 on error
 */
int info_handle_container_fprint(
     info_handle_t *info_handle,
     libcerror_error_t **error )
{
	uint8_t uuid_data[ 16 ];

	libfsapfs_volume_t *volume = NULL;
	static char *function      = "info_handle_container_fprint";
	int number_of_volumes      = 0;
	int volume_index           = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "Apple File System (APFS) information:\n\n" );

	fprintf(
	 info_handle->notify_stream,
	 "Container information:\n" );

	if( libfsapfs_container_get_identifier(
	     info_handle->input_container,
	     uuid_data,
	     16,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve container identifier.",
		 function );

		goto on_error;
	}
	if( info_handle_uuid_value_fprint(
	     info_handle,
	     "\tIdentifier\t\t",
	     uuid_data,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print UUID value.",
		 function );

		goto on_error;
	}
/* TODO print additional container information such as size */

	if( libfsapfs_container_get_number_of_volumes(
	     info_handle->input_container,
	     &number_of_volumes,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of volumes.",
		 function );

		goto on_error;
	}
	fprintf(
	 info_handle->notify_stream,
	 "\tNumber of volumes\t: %d\n",
	 number_of_volumes );

	for( volume_index = 0;
	     volume_index < number_of_volumes;
	     volume_index++ )
	{
		if( libfsapfs_container_get_volume_by_index(
		     info_handle->input_container,
		     volume_index,
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( info_handle->user_password != NULL )
		{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
			if( libfsapfs_volume_set_utf16_password(
			     volume,
			     (uint16_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#else
			if( libfsapfs_volume_set_utf8_password(
			     volume,
			     (uint8_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#endif
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set password.",
				 function );

				goto on_error;
			}
/* TODO call unlock volume */
		}
		if( info_handle_volume_fprint(
		     info_handle,
		     volume,
		     volume_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( libfsapfs_volume_free(
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free volume.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

/* Prints file entry information as part of the bodyfile
 * Returns 1 if successful or -1 on error
 */
int info_handle_bodyfile_fprint_file_entry(
     info_handle_t *info_handle,
     libfsapfs_file_entry_t *file_entry,
     const system_character_t *path,
     int level,
     libcerror_error_t **error )
{
	char file_mode_string[ 11 ]            = { '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', 0 };

	libfsapfs_file_entry_t *sub_file_entry = NULL;
	system_character_t *file_entry_name    = NULL;
	system_character_t *sub_path           = NULL;
	static char *function                  = "info_handle_bodyfile_fprint_file_entry";
	size_t file_entry_name_size            = 0;
	size_t path_length                     = 0;
	size_t sub_path_size                   = 0;
	uint64_t access_time                   = 0;
	uint64_t creation_time                 = 0;
	uint64_t identifier                    = 0;
	uint64_t inode_change_time             = 0;
	uint64_t modification_time             = 0;
	uint32_t group_identifier              = 0;
	uint32_t owner_identifier              = 0;
	uint16_t file_mode                     = 0;
	int number_of_sub_file_entries         = 0;
	int result                             = 0;
	int sub_file_entry_index               = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( path == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid path.",
		 function );

		return( -1 );
	}
	if( libfsapfs_file_entry_get_number_of_sub_file_entries(
	     file_entry,
	     &number_of_sub_file_entries,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of sub file entries.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_identifier(
	     file_entry,
	     &identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve identifier.",
		 function );

		goto on_error;
	}
	path_length = system_string_length(
	               path );

	if( level > 0 )
	{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfsapfs_file_entry_get_utf16_name_size(
		          file_entry,
		          &file_entry_name_size,
		          error );
#else
		result = libfsapfs_file_entry_get_utf8_name_size(
		          file_entry,
		          &file_entry_name_size,
		          error );
#endif
		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve file entry name string size.",
			 function );

			goto on_error;
		}
		if( ( result == 1 )
		 && ( file_entry_name_size > 0 ) )
		{
			file_entry_name = system_string_allocate(
			                   file_entry_name_size );

			if( file_entry_name == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				 "%s: unable to create file entry name string.",
				 function );

				goto on_error;
			}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfsapfs_file_entry_get_utf16_name(
			          file_entry,
			          (uint16_t *) file_entry_name,
			          file_entry_name_size,
			          error );
#else
			result = libfsapfs_file_entry_get_utf8_name(
			          file_entry,
			          (uint8_t *) file_entry_name,
			          file_entry_name_size,
			          error );
#endif
			if( result != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve file entry name string.",
				 function );

				goto on_error;
			}
		}
	}
	if( libfsapfs_file_entry_get_creation_time(
	     file_entry,
	     &creation_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve creation time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_modification_time(
	     file_entry,
	     &modification_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve modification time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_inode_change_time(
	     file_entry,
	     &inode_change_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve inode change time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_access_time(
	     file_entry,
	     &access_time,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve access time.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_owner_identifier(
	     file_entry,
	     &owner_identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve owner identifier.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_group_identifier(
	     file_entry,
	     &group_identifier,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve group identifier.",
		 function );

		goto on_error;
	}
	if( libfsapfs_file_entry_get_file_mode(
	     file_entry,
	     &file_mode,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file mode.",
		 function );

		goto on_error;
	}
/* TODO move into function */
	if( ( file_mode & 0x0001 ) != 0 )
	{
		file_mode_string[ 9 ] = 'x';
	}
	if( ( file_mode & 0x0002 ) != 0 )
	{
		file_mode_string[ 8 ] = 'w';
	}
	if( ( file_mode & 0x0004 ) != 0 )
	{
		file_mode_string[ 7 ] = 'r';
	}
	if( ( file_mode & 0x0008 ) != 0 )
	{
		file_mode_string[ 6 ] = 'x';
	}
	if( ( file_mode & 0x0010 ) != 0 )
	{
		file_mode_string[ 5 ] = 'w';
	}
	if( ( file_mode & 0x0020 ) != 0 )
	{
		file_mode_string[ 4 ] = 'r';
	}
	if( ( file_mode & 0x0040 ) != 0 )
	{
		file_mode_string[ 3 ] = 'x';
	}
	if( ( file_mode & 0x0080 ) != 0 )
	{
		file_mode_string[ 2 ] = 'w';
	}
	if( ( file_mode & 0x0100 ) != 0 )
	{
		file_mode_string[ 1 ] = 'r';
	}
	switch( file_mode & 0xf000 )
	{
		case 0x1000:
			file_mode_string[ 0 ] = 'p';
			break;

		case 0x2000:
			file_mode_string[ 0 ] = 'c';
			break;

		case 0x4000:
			file_mode_string[ 0 ] = 'd';
			break;

		case 0x6000:
			file_mode_string[ 0 ] = 'b';
			break;

		case 0xa000:
			file_mode_string[ 0 ] = 'l';
			break;

		case 0xc000:
			file_mode_string[ 0 ] = 's';
			break;

		default:
			break;
	}
	/* Colums in a Sleuthkit 3.x and later bodyfile
	 * MD5|name|inode|mode_as_string|UID|GID|size|atime|mtime|ctime|crtime
	 */
	fprintf(
	 info_handle->notify_stream,
	 "0|%" PRIs_SYSTEM "",
	 path );

	if( file_entry_name != NULL )
	{
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIs_SYSTEM "",
		 file_entry_name );
	}
	fprintf(
	 info_handle->notify_stream,
	 "|%" PRIu64 "|%s|%" PRIu32 "|%" PRIu32 "|%f|%f|%f|%f\n",
	 identifier,
	 file_mode_string,
	 owner_identifier,
	 group_identifier,
	 (double) access_time / 1000000000,
	 (double) modification_time / 1000000000,
	 (double) inode_change_time / 1000000000,
	 (double) creation_time / 1000000000 );

	if( number_of_sub_file_entries > 0 )
	{
		sub_path_size = path_length + file_entry_name_size + 1;

		sub_path = system_string_allocate(
		            sub_path_size );

		if( sub_path == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create sub path.",
			 function );

			goto on_error;
		}
		if( system_string_copy(
		     sub_path,
		     path,
		     path_length ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy path to sub path.",
			 function );

			goto on_error;
		}
		if( file_entry_name != NULL )
		{
			if( system_string_copy(
			     &( sub_path[ path_length ] ),
			     file_entry_name,
			     file_entry_name_size - 1 ) == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
				 "%s: unable to copy file entry name to sub path.",
				 function );

				goto on_error;
			}
			sub_path[ sub_path_size - 2 ] = (system_character_t) '/';
		}
		sub_path[ sub_path_size - 1 ] = (system_character_t) 0;

		for( sub_file_entry_index = 0;
		     sub_file_entry_index < number_of_sub_file_entries;
		     sub_file_entry_index++ )
		{
			if( libfsapfs_file_entry_get_sub_file_entry_by_index(
			     file_entry,
			     sub_file_entry_index,
			     &sub_file_entry,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve sub file entry: %d.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
			if( info_handle_bodyfile_fprint_file_entry(
			     info_handle,
			     sub_file_entry,
			     sub_path,
			     level + 1,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print file entry: %d information.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
			if( libfsapfs_file_entry_free(
			     &sub_file_entry,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free sub file entry: %d.",
				 function,
				 sub_file_entry_index );

				goto on_error;
			}
		}
	}
	if( sub_path != NULL )
	{
		memory_free(
		 sub_path );

		sub_path = NULL;
	}
	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );

		file_entry_name = NULL;
	}
	return( 1 );

on_error:
	if( sub_file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &sub_file_entry,
		 NULL );
	}
	if( sub_path != NULL )
	{
		memory_free(
		 sub_path );
	}
	if( file_entry_name != NULL )
	{
		memory_free(
		 file_entry_name );
	}
	return( -1 );
}

/* Creates a bodyfile
 * Returns 1 if successful or -1 on error
 */
int info_handle_create_bodyfile(
     info_handle_t *info_handle,
     const system_character_t *path,
     libcerror_error_t **error )
{
	system_character_t uuid_string[ 48 ];
	uint8_t uuid_data[ 16 ];

	libfguid_identifier_t *uuid        = NULL;
	libfsapfs_file_entry_t *file_entry = NULL;
	libfsapfs_volume_t *volume         = NULL;
	static char *function              = "info_handle_create_bodyfile";
	int number_of_volumes              = 0;
	int result                         = 0;
	int volume_index                   = 0;

	if( info_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( libfsapfs_container_get_number_of_volumes(
	     info_handle->input_container,
	     &number_of_volumes,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of volumes.",
		 function );

		goto on_error;
	}
	for( volume_index = 0;
	     volume_index < number_of_volumes;
	     volume_index++ )
	{
		if( libfsapfs_container_get_volume_by_index(
		     info_handle->input_container,
		     volume_index,
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( info_handle->user_password != NULL )
		{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
			if( libfsapfs_volume_set_utf16_password(
			     volume,
			     (uint16_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#else
			if( libfsapfs_volume_set_utf8_password(
			     volume,
			     (uint8_t *) info_handle->user_password,
			     info_handle->user_password_size - 1,
			     error ) != 1 )
#endif
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set password.",
				 function );

				goto on_error;
			}
/* TODO call unlock volume */
		}
		if( libfsapfs_volume_get_identifier(
		     volume,
		     uuid_data,
		     16,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve volume identifier.",
			 function );

			goto on_error;
		}
		if( libfguid_identifier_initialize(
		     &uuid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create UUID.",
			 function );

			goto on_error;
		}
		if( libfguid_identifier_copy_from_byte_stream(
		     uuid,
		     uuid_data,
		     16,
		     LIBFGUID_ENDIAN_BIG,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy byte stream to UUID.",
			 function );

			goto on_error;
		}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfguid_identifier_copy_to_utf16_string(
			  uuid,
			  (uint16_t *) &( uuid_string[ 1 ] ),
			  48 - 1,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE | LIBFGUID_STRING_FORMAT_FLAG_USE_SURROUNDING_BRACES,
			  error );
#else
		result = libfguid_identifier_copy_to_utf8_string(
			  uuid,
			  (uint8_t *) &( uuid_string[ 1 ] ),
			  48 - 1,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE | LIBFGUID_STRING_FORMAT_FLAG_USE_SURROUNDING_BRACES,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy UUID to string.",
			 function );

			goto on_error;
		}
		uuid_string[ 0 ]  = (system_character_t) '/';
		uuid_string[ 39 ] = (system_character_t) '/';
		uuid_string[ 40 ] = (system_character_t) 0;

		if( libfguid_identifier_free(
		     &uuid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free UUID.",
			 function );

			goto on_error;
		}
		if( libfsapfs_volume_get_root_directory(
		     volume,
		     &file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve root directory file entry from volume: %d.",
			 function,
			 volume_index );

			goto on_error;
		}
		if( info_handle_bodyfile_fprint_file_entry(
		     info_handle,
		     file_entry,
		     uuid_string,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print root directory file entry information.",
			 function );

			goto on_error;
		}
		if( libfsapfs_file_entry_free(
		     &file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file entry.",
			 function );

			goto on_error;
		}
		if( libfsapfs_volume_free(
		     &volume,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free volume.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( file_entry != NULL )
	{
		libfsapfs_file_entry_free(
		 &file_entry,
		 NULL );
	}
	if( uuid != NULL )
	{
		libfguid_identifier_free(
		 &uuid,
		 NULL );
	}
	if( volume != NULL )
	{
		libfsapfs_volume_free(
		 &volume,
		 NULL );
	}
	return( -1 );
}

