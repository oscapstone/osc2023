/// \file include/oscos/initrd.h
/// \brief Initial ramdisk.
///
/// Before reading the initial ramdisk, it must be validated by calling
/// initrd_is_valid(void).

#ifndef OSCOS_INITRD_H
#define OSCOS_INITRD_H

#include <stdbool.h>
#include <stdint.h>

#include "oscos/align.h"
#include "oscos/libc/string.h"

#define INITRD_ADDR ((void *)0x8000000)

/// \brief New ASCII Format CPIO archive header.
typedef struct {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
} cpio_newc_header_t;

#define CPIO_NEWC_MODE_FILE_TYPE_MASK ((uint32_t)0170000)
#define CPIO_NEWC_MODE_FILE_TYPE_LNK ((uint32_t)0120000)
#define CPIO_NEWC_MODE_FILE_TYPE_REG ((uint32_t)0100000)
#define CPIO_NEWC_MODE_FILE_TYPE_DIR ((uint32_t)0040000)

/// \brief New ASCII Format CPIO archive file entry.
typedef struct {
  cpio_newc_header_t header;
  char payload[];
} cpio_newc_entry_t;

/// \brief Pointer to the first file entry of the initial ramdisk.
#define INITRD_HEAD ((const cpio_newc_entry_t *)INITRD_ADDR)

/// \brief The value of the given header of the file entry.
#define CPIO_NEWC_HEADER_VALUE(ENTRY, HEADER)                                  \
  cpio_newc_parse_header_field((ENTRY)->header.c_##HEADER)

/// \brief The pathname of the file entry.
#define CPIO_NEWC_PATHNAME(ENTRY) ((const char *)(ENTRY)->payload)

/// \brief The file data of the file entry.
#define CPIO_NEWC_FILE_DATA(ENTRY)                                             \
  ((const char *)ALIGN((uintptr_t)(ENTRY)->payload +                           \
                           CPIO_NEWC_HEADER_VALUE(ENTRY, namesize),            \
                       4))

/// \brief The filesize of the file entry.
#define CPIO_NEWC_FILESIZE(ENTRY) CPIO_NEWC_HEADER_VALUE(ENTRY, filesize)

/// \brief Determines if the file entry is the final sentinel entry.
#define CPIO_NEWC_IS_ENTRY_LAST(ENTRY)                                         \
  (strcmp(CPIO_NEWC_PATHNAME(ENTRY), "TRAILER!!!") == 0)

/// \brief Returns the next file entry of the given file entry.
///
/// The given file entry must not be the final sentinel entry.
#define CPIO_NEWC_NEXT_ENTRY(ENTRY)                                            \
  (const cpio_newc_entry_t *)ALIGN(                                            \
      (uintptr_t)CPIO_NEWC_FILE_DATA(ENTRY) + CPIO_NEWC_FILESIZE(ENTRY), 4)

/// \brief Expands to a for loop that loops over each file entry in the initial
///        ramdisk.
///
/// \param ENTRY_NAME The name of the variable for the file entry.
#define INITRD_FOR_ENTRY(ENTRY_NAME)                                           \
  for (const cpio_newc_entry_t *ENTRY_NAME = INITRD_HEAD;                      \
       !CPIO_NEWC_IS_ENTRY_LAST(ENTRY_NAME);                                   \
       ENTRY_NAME = CPIO_NEWC_NEXT_ENTRY(ENTRY_NAME))

/// \brief Determines if the initial ramdisk is a valid New ASCII format CPIO
///        archive.
bool initrd_is_valid(void);

/// \brief Parses the given header field of a New ASCII format CPIO archive.
///
/// The given header field must be valid.
///
/// \see CPIO_NEWC_HEADER_VALUE
uint32_t cpio_newc_parse_header_field(const char field[static 8]);

/// \brief Finds the file entry in the initial ramdisk corresponding to the
///        given pathname.
/// \param pathname The pathname.
/// \return The pointer to the file entry if the entry is found, or NULL
///         otherwise.
const cpio_newc_entry_t *initrd_find_entry_by_pathname(const char *pathname);

#endif
