#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "verifier.h"

/*
    TODO add a way to add keys from command line
    TODO add a way to create .vfy file from command line
    TODO dynamically update defines at install
*/

#define MAX_LINE 256
#define ADDITIONAL_SPACE 64
#define MAX_HOME_PATH 128

#define CMD_SIZE (ADDITIONAL_SPACE + MAX_HOME_PATH + TMP_FOLDER_SIZE)

static char* errStr[] = {
    "Invalid header",
    "Unknown author",
    "File altered",
    "Invalid .vfy file",
    "OpenSSL error"
};
#define INVALID_HEADER errStr[0]
#define UNKNOWN_AUTH errStr[1]
#define FILE_ALTERED errStr[2]
#define INVALID_VFY errStr[3]
#define OPENSSL_ERR errStr[4]

#define STR_SEP ","

/**
 ** Verifies is the passed file is what it really is
 ** @return 0 -> OK, signature ok
 ** @return 1 -> KO, signature mismatch
 ** @return 2 -> invalid file
 **
 ** If 0 is returned, out is a pointer to the name of the file created
 **/
static unsigned char
verifyFile(char *base, char *header, char *signature, char **out);

/**
 ** Checks if the given auth has a trusted public key
 ** Fills keyPath with its path if it exists and return true
 ** Else returns false
 **/
static bool
isAuthKnown(char *auth, size_t len, char **keyPath);

/**
 ** Checks if the data is well signed with OpenSSL
 **/
static bool
messageWellSigned(char *sign, char *key, char **out, size_t len);

/**
 ** Returns true if the given file is accessible
 **/
static bool
fileExists(char *path);


unsigned char
prepareFile(char *in, size_t len, char **out)
{
    char tmpFolder[PATH_MAX] = {0};
    char headerFile[PATH_MAX] = {0};
    char signatureFile[PATH_MAX] = {0};
    char baseName[PATH_MAX] = {0};
    char *cmd = NULL;
    size_t tmpSize = 0;
    size_t baseSize = 0;
    unsigned char ret = 0;

    if (!fileExists(in)) {
        *out = strerror(errno);
        return V_FERROR;
    }
   
    strncpy(tmpFolder, getenv("HOME"), MAX_HOME_PATH);
    strncat(tmpFolder, TMP_FOLDER, TMP_FOLDER_SIZE);

    if (mkdir(tmpFolder, S_IRWXU) && errno != EEXIST) {
        *out = strerror(errno);
        return V_FERROR;
    }

    cmd = calloc(len + CMD_SIZE, sizeof (char));
    tmpSize = strlen(tmpFolder);
    
    strncpy(cmd, "tar -zxf ", 9);
    strncat(cmd, in, len);
    strncat(cmd, " -C ", 4);
    strncat(cmd, tmpFolder, tmpSize);
    strncat(cmd, " > /dev/null 2>&1", 17);
    if (system(cmd)) {
        *out = INVALID_VFY;
        free(cmd);
        return V_FERROR;
    }
    free(cmd);

    strncpy(baseName, tmpFolder, tmpSize);
    // in may be modified by basename, but since its not used anymore, its fine
    strncat(baseName, basename(in), len); // len is upper bound of basename(in)
    baseSize = strlen(baseName);

    strncpy(signatureFile, baseName, baseSize);
    strncat(signatureFile, ".sgn", 4);

    strncpy(headerFile, baseName, baseSize);
    strncat(headerFile, ".hdr", 4);

    if (!fileExists(signatureFile) || !fileExists(headerFile)) {
        *out = INVALID_VFY;
        return V_FERROR;
    }

    ret = verifyFile(baseName, headerFile, signatureFile, out);

    cmd = calloc(ADDITIONAL_SPACE + baseSize, sizeof (char));
    strncpy(cmd, "rm ", 3);
    strncat(cmd, baseName, baseSize);
    strncat(cmd, "* > /dev/null 2>&1", 18);

    system(cmd);
    
    free(cmd);

    return ret;
}

static bool
fileExists(char *path)
{
    return (access(path, F_OK) != -1);
}

static unsigned char
verifyFile(char *base, char *header, char *signature, char **out)
{
    FILE *hdr = NULL;
    char *key = NULL;
    char line[MAX_LINE] = {0};
    char baseOut[PATH_MAX] = {0};
    char *auth = NULL;
    char *ext = NULL;
    char *last = NULL;
    size_t extLen = 0;
    size_t baseLen = strlen(base);
    size_t authLen = 0;
    size_t outLen = 0;

    hdr = fopen(header, "r");
    if (hdr == NULL) {
        *out = strerror(errno);
        return V_FERROR;
    }

    fgets(line, sizeof (line), hdr);
    fclose(hdr);
    auth = strtok(line, STR_SEP);
    ext = strtok(NULL, STR_SEP);
    last = strtok(NULL, STR_SEP);

    if (auth == NULL || ext == NULL || last == NULL
    || (last != NULL && last[0] != '\n')) {
        *out = INVALID_HEADER;
        return V_FERROR;
    }

    authLen = strlen(auth);

    if (!isAuthKnown(auth, authLen, &key)) {
        *out = UNKNOWN_AUTH;
        return V_INVALID;
    }

    extLen = strlen(ext);

    *out = calloc(baseLen + extLen + authLen + ADDITIONAL_SPACE,
                    sizeof (char));
    strncpy(*out, base, baseLen);
    strncat(*out, ".", 1);
    strncat(*out, ext, extLen);

    if (!fileExists(*out)) {
        free(key);
        free(*out);
        *out = INVALID_VFY;
        return V_FERROR;
    }

    outLen = strlen(*out);

    if (!messageWellSigned(signature, key, out, outLen)) {
        free(key);
        return V_INVALID;
    }

    // *out and basename(*out) overlap, so copy
    // basename may modify out, but we reset it after so its fine
    strncpy(baseOut, basename(*out), outLen); // outLen is upper bound of basename(*out)

    rename(*out, baseOut); // moves *out file to the current dir

    strncpy(*out, baseOut, outLen); // outLen is used to pad with \0
    strncat(*out, " (from ", 7);
    strncat(*out, auth, authLen);
    strncat(*out, ")", 1);

    free(key);
    return V_VALID;
}

static bool
isAuthKnown(char *auth, size_t len, char **keyPath)
{
    char path[PATH_MAX] = {0};
    FILE *key = NULL;
    size_t pathLen = 0;

    strncpy(path, getenv("HOME"), MAX_HOME_PATH);
    strncat(path, TRUSTED_FOLDER, TRUSTED_FOLDER_SIZE);
    strncat(path, auth, len);

    key = fopen(path, "r");
    if (key != NULL) {
        pathLen = strlen(path);
        *keyPath = calloc(pathLen + 1, sizeof (char));
        strncpy(*keyPath, path, pathLen);
        fclose(key);
        return true;
    }
    return false;
}

static bool
messageWellSigned(char *sign, char *key, char **out, size_t len)
{
    char output[MAX_LINE] = {0};
    size_t sLen = strlen(sign);
    size_t kLen = strlen(key);
    char *cmd = calloc(sLen + sLen + ADDITIONAL_SPACE, sizeof (char));
    FILE *pipe = NULL;

    // command to decode signature from base64
    // the temp file will be deleted in the func prepareFile()
    strncpy(cmd, "openssl base64 -d -in ", 22);
    strncat(cmd, sign, sLen);
    strncat(cmd, " -out ", 6);
    strncat(cmd, sign, sLen);
    strncat(cmd, ".sha256", 7);
    
    if (system(cmd)) {
        free(cmd);
        free(*out);
        *out = OPENSSL_ERR;
        return false;
    }

    free(cmd);
    cmd = calloc(kLen + sLen + len + ADDITIONAL_SPACE, sizeof (char));

    // command that will output data telling if the signature is correct
    strncpy(cmd, "openssl dgst -sha256 -verify ", 29); 
    strncat(cmd, key, kLen);
    strncat(cmd, " -signature ", 12);
    strncat(cmd, sign, sLen);
    strncat(cmd, ".sha256 ", 8);
    strncat(cmd, *out, len);

    pipe = popen(cmd, "r");
    free(cmd);

    if (pipe == NULL) {
        free(*out);
        *out = OPENSSL_ERR;
        return false;
    }

    if (fgets(output, sizeof (output), pipe) == NULL) {
        pclose(pipe);
        free(*out);
        *out = OPENSSL_ERR;
        return false;
    }

    if (strcmp(output, "Verified OK\n")) {
        pclose(pipe);
        free(*out);
        *out = FILE_ALTERED;
        return false;
    }

    pclose(pipe);
    return true;
}

