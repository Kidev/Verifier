#ifndef HEADER_VERIFY
#define HEADER_VERIFY

// This is where the trusted keys are stored, relative to $HOME
#define TRUSTED_FOLDER "/.verify/trusted/"
#define TRUSTED_FOLDER_SIZE 17
// This is the folder where temporary files are stored, relative to $HOME
#define TMP_FOLDER "/.verify/tmp/"
#define TMP_FOLDER_SIZE 13

#define V_VALID 0
#define V_INVALID 1
#define V_FERROR 2

/**
 ** Checks if the given tarball is safe, and if so,
 ** Puts its data in the current folder
 ** *out is filled with meaningful data about the operation
 **/
unsigned char
prepareFile(char *in, size_t len, char **out);

#endif /* HEADER_VERIFY */

