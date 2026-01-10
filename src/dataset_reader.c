#include <stdio.h>
#include <stdlib.h>
#include "../include/dataset_reader.h"

/**
 * Reads integers from a file separated by commas (e.g., "10, 20, 30").
 * * @param filename The path to the file.
 * @param count_out A pointer to an integer where the function will write the array size.
 * @return A pointer to the allocated array, or NULL if failed.
 */
int *read_ints_from_file(const char *filename, int *count_out) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    int temp_val;
    int count = 0;
    char ch;

    // --- PASS 1: Count the numbers ---
    // We try to read an integer. If successful, we increment count.
    // Then we consume the next character (comma or newline) to prepare for the next read.
    while (fscanf(fp, "%d", &temp_val) == 1) {
        count++;
        // Consume the separator (comma) and any whitespace following it
        // We Loop until we hit a digit, EOF, or something that isn't a separator
        while ((ch = fgetc(fp)) != EOF && (ch == ',' || ch == ' ' || ch == '\n' || ch == '\r'));
        
        // If we hit a digit or something else, put it back so fscanf can read it next
        if (ch != EOF) {
            ungetc(ch, fp);
        }
    }

    // Allocate memory exactly for the number of integers found
    int *array = (int*)malloc(count * sizeof(int));
    if (array == NULL) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // --- PASS 2: Read the data ---
    rewind(fp); // Go back to the start of the file
    int index = 0;
    
    while (fscanf(fp, "%d", &array[index]) == 1) {
        index++;
        // Skip separators again just like pass 1
        while ((ch = fgetc(fp)) != EOF && (ch == ',' || ch == ' ' || ch == '\n' || ch == '\r'));
        if (ch != EOF) {
            ungetc(ch, fp);
        }
    }

    fclose(fp);
    
    // Save the count so the caller knows the array size
    *count_out = count;
    
    return array;
}

#ifdef READER_TEST
int main() {
    // 1. Setup a test file programmatically (for demonstration)
    FILE *f = fopen("data.txt", "w");
    if (f) {
        fprintf(f, "10, 25, 30, 4, 100");
        fclose(f);
    }

    // 2. Call the function
    int size = 0;
    int *my_array = read_ints_from_file("data.txt", &size);

    // 3. Verify output
    if (my_array != NULL) {
        printf("Successfully read %d integers:\n", size);
        printf("[ ");
        for (int i = 0; i < size; i++) {
            printf("%d ", my_array[i]);
        }
        printf("]\n");

        // Always free dynamically allocated memory
        free(my_array);
    }

    return 0;
}
#endif
