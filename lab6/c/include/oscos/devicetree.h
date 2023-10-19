#ifndef OSCOS_DEVICETREE_H
#define OSCOS_DEVICETREE_H

#include <stdbool.h>
#include <stdint.h>

#include "oscos/libc/string.h"
#include "oscos/utils/align.h"
#include "oscos/utils/control-flow.h"
#include "oscos/utils/endian.h"

/// \brief Flattened devicetree header.
typedef struct {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
} fdt_header_t;

/// \brief An entry of the memory reservation block.
typedef struct {
  uint64_t address;
  uint64_t size;
} fdt_reserve_entry_t;

/// \brief An item in a node in a flattened devicetree blob.
typedef struct {
  uint32_t token;
  char payload[];
} fdt_item_t;

/// \brief Flattened devicetree property; whatever follows the FDT_PROP token.
typedef struct {
  uint32_t len;
  uint32_t nameoff;
  char value[];
} fdt_prop_t;

#define FDT_BEGIN_NODE ((uint32_t)0x00000001)
#define FDT_END_NODE ((uint32_t)0x00000002)
#define FDT_PROP ((uint32_t)0x00000003)
#define FDT_NOP ((uint32_t)0x00000004)
#define FDT_END ((uint32_t)0x00000009)

/// \brief Initializes the device tree.
/// \param dtb_start The loading address of the devicetree blob.
/// \return Whether or not the initialization succeeds.
bool devicetree_init(const void *dtb_start);

/// \brief Returns whether or not the devicetree has been successfully
///        initialized.
bool devicetree_is_init(void);

/// \brief The start of the memory reservation block of the devicetree blob.
#define FDT_START_MEM_RSVMAP                                                   \
  ((const fdt_reserve_entry_t                                                  \
        *)(fdt_get_start() +                                                   \
           rev_u32(((const fdt_header_t *)fdt_get_start())->off_mem_rsvmap)))

/// \brief The start of the strings block of the devicetree blob.
#define FDT_START_STRINGS                                                      \
  (fdt_get_start() +                                                           \
   rev_u32(((const fdt_header_t *)fdt_get_start())->off_dt_strings))

/// \brief The starting token of an item in the structure block.
#define FDT_TOKEN(ITEM) (rev_u32((ITEM)->token))

/// \brief The name of an node in the structure block.
#define FDT_NODE_NAME(NODE) ((NODE)->payload)

/// \brief The name of a property in the structure block.
#define FDT_PROP_NAME(PROP) (FDT_START_STRINGS + rev_u32((PROP)->nameoff))

/// \brief The value of a property in the structure block.
#define FDT_PROP_VALUE(PROP) ((PROP)->value)

/// \brief The length of the of a property in the structure block.
#define FDT_PROP_VALUE_LEN(PROP) (rev_u32((PROP)->len))

#define FDT_ITEMS_START(NODE)                                                  \
  ((const fdt_item_t *)ALIGN(                                                  \
      (uintptr_t)((NODE)->payload) + strlen(FDT_NODE_NAME(NODE)) + 1, 4))
#define FDT_ITEM_IS_END(ITEM) (FDT_TOKEN(ITEM) == FDT_END_NODE)

/// \brief Expands to a for loop that loops over each item in the given node.
///
/// \param NODE The pointer to the node.
/// \param ITEM_NAME The name of the variable for the item.
#define FDT_FOR_ITEM(NODE, ITEM_NAME)                                          \
  for (const fdt_item_t *ITEM_NAME = FDT_ITEMS_START(NODE);                    \
       !FDT_ITEM_IS_END(ITEM_NAME); ITEM_NAME = fdt_next_item(ITEM_NAME))

/// \brief Expands to a for loop that loops over each item in the given node.
///
/// This is a variant of FDT_FOR_ITEM that does not declare the variable within
/// the for loop. This is useful, e. g., if one wants to obtain the pointer to
/// the FDT_END_NODE token.
///
/// \param NODE The pointer to the node.
/// \param ITEM_NAME The name of the variable for the item. Must be a declared
///                  variable.
#define FDT_FOR_ITEM_(NODE, ITEM_NAME)                                         \
  for (ITEM_NAME = FDT_ITEMS_START(NODE); !FDT_ITEM_IS_END(ITEM_NAME);         \
       ITEM_NAME = fdt_next_item(ITEM_NAME))

/// \brief Gets the starting address of the devicetree blob.
const char *fdt_get_start(void);

/// \brief Gets the ending address of the devicetree blob.
const char *fdt_get_end(void);

/// \brief Gets the pointer to the next item of the given item in the same node.
/// \param item The item. Must not point to the FDT_END_NODE token.
const fdt_item_t *fdt_next_item(const fdt_item_t *item);

/// \brief A node of the parent node linked list.
typedef struct fdt_traverse_parent_list_node_t {
  const fdt_item_t *node;
  const struct fdt_traverse_parent_list_node_t *parent;
} fdt_traverse_parent_list_node_t;

/// \brief Callback for void fdt_traverse(fdt_traverse_callback_t *, void *).
typedef control_flow_t
fdt_traverse_callback_t(void *, const fdt_item_t *,
                        const fdt_traverse_parent_list_node_t *);

/// \brief Performs in-order traversal of the devicetree.
///
/// When the traversal process encounters a node, \p callback is called with
/// \p arg, the pointer to the current node, and the linked list of the parent
/// nodes of the current node.
///
/// \param callback The callback that is called on each node.
/// \param arg The first argument that will be passed to \p callback.
void fdt_traverse(fdt_traverse_callback_t *callback, void *arg);

/// \brief The #address-cells and the #size-cells properties.
typedef struct {
  uint32_t n_address_cells, n_size_cells;
} fdt_n_address_size_cells_t;

/// \brief Gets the #address-cells and the #size-cells properties of a node.
///
/// \param node The pointer to the node.
/// \return The value of the #address-cells and the #size-cells properties, or
///         their respective defaults if these properties are missing.
fdt_n_address_size_cells_t fdt_get_n_address_size_cells(const fdt_item_t *node);

/// \brief The reg property.
typedef struct {
  uintmax_t address, size;
} fdt_reg_t;

typedef struct {
  fdt_reg_t value;
  bool address_overflow, size_overflow;
} fdt_read_reg_result_t;

/// \brief Reads the reg property.
///
/// \param prop The pointer to the reg property.
/// \param n_cells The #address-cells and the #size-cells properties of the
///                parent node.
fdt_read_reg_result_t fdt_read_reg(const fdt_prop_t *prop,
                                   fdt_n_address_size_cells_t n_cells);

#endif
