#ifndef DATASET_READER
#define DATASET_READER
/**
 * @brief Reads integers from a text file separated by commas.
 * * This function parses a file containing integers separated by delimiters 
 * (such as commas, spaces, or newlines). It allocates memory dynamically 
 * to store the integers. The caller is responsible for freeing the allocated memory.
 * * @param filename The path to the file to be read.
 * @param count_out A pointer to an integer where the function will write the total number of elements read.
 * @return A pointer to the dynamically allocated array of integers, or NULL if the file could not be opened or memory allocation failed.
 */
int *read_ints_from_file(const char *filename, int *count_out);
#endif
