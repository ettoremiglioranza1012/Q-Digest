// This docs are the my temporary notes regarding the functions' purpose. To be updated and added
// to the actual .c file for clear documentations. 

/*
 * 
 * node_and_sibbling_count:   return the aggregated count for a node and its siblings.
 * 
 * delete_node_if_needed:     a tree node which has a count of 0 can be eliminated if it has no children. 
 *                            Returns 'true' or 'false' depending on wheter it deleted the node 'n' from the tree. 
 * 
 * compress:                  perform compaction. Specifically, ensure that no node is too small, i.e. apart form
 *                            the root node, try to see if we can compress counts from a set of 3 nodes 
 *                            (i.e. a node, its parent and its sibling) and promote them to the parent node. 
 * printTree:                 print the tree structure with end user representation.
 *
 * expandTree:                Expand the range of the tree to include numbers in the range [0..ub).
 *                         
 * _insert_node:              Insert the equivalent of the values present in node 'n' into the current tree 
 *                            This will either create new nodes along the way and then create the final node 
 *                            or will update the count in the tree. No compaction is attempted after the new
 *                            node is inserted since the function is assumed to be called by the deserialization routine.
 *
*
 * compact_if_needed:         Perform a post-order traversal of the tree and fetch the element at rank 'req_rank' 
 *                            starting from the smallest element in the structure. 
 *
 * preorder_toString:         Perform a pre-order traversal of the tree and serialize all the nodes with a 
 *                            non-zero count. Separates each node with a newline (\n).
 *                            Usage: 
 *                            FILE *file = fopen("output.txt", "w");
 *                            preorder_toString(root, file);
 *                            fclose(file);
 *
 * QDigest_Create:            Create a QDigest data structure and return the pointer to it.
 *
 * moveQDigest:               Move the ownership of QDigest instance's resource to another instance;
 *                            Probably not necessary in C; 
 * 
 * percentile:                returns the approximate 100p'th percentile element in the structure.
 *                            i.e. passing 0.7 will return the 70th percentile element (which is the 
 *                            70th percentile element starting from the smallest element). 
 *
 * toString:                  Serialized format consists of newline separated entries which
 *                            are tripples of the form: (LB, UB, COUNT).
 *
 *                            That means that we have a node which has COUNT elements which
 *                            have values in the range [LB..UB]. Only non-empty ranges will
 *                            be serialized (i.e. the serialized tree will be sparse). Also,
 *                            the ranges will be serialized in pre-order tree traversal so
 *                            that re-construction is easy.
 *
 * fromString:                Deserialize the tree from the serialized version in the string 
 *                            'ser'. The serialized version is obtained by calling toString().
 *
 * QDigest_add:               ADD to the current Qdigest another by merging them inplace of the first. 
 *
 * printTree:                 print current all data structure by printing trees recursively. 
 *                            Use as 'printTree(stdout);'.
 *
 * QDigest_print:             stdout representation of Qdigest Data Structure.
 *                            Equivalent of python __str__.
 *
 * */
