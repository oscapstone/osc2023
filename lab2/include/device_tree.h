#ifndef	DEVICE_TREE_H
#define	DEVICE_TREE_H

int get_device_tree_adr(void);
/*
 * req_callback(node_name, prop_name, value_ptr, value_length) returns 1 on
 * success.
 */
void fdt_traverse(int (*req_callback)(char*, char*, char*, unsigned int));

#endif /* DEVICE_TREE_H */