#ifndef	DEVICE_TREE_H
#define	DEVICE_TREE_H

int get_device_tree_adr(void);
void fdt_traverse(char* req_node, char* req_prop, 
                  void (*req_callback)(char*, unsigned int));

#endif /* DEVICE_TREE_H */