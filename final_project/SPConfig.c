#include "SPConfig.h"

#define MAX_SIZE 1024
#define VALID_BOOL(paramValue) \
    if (strcmp(paramValue, "true") && strcmp(paramValue, "false")) \
        return SP_CONFIG_INVALID_INTEGER;
#define VALID_INT(paramValue) \
    if (!isInt(paramValue) || atoi(paramValue) <= 0) \
        return SP_CONFIG_INVALID_INTEGER;
#define VALID_INT_RANGE(arr, value, size) \
    if (!isContainsInt(arr, value, size)) \
        return SP_CONFIG_INVALID_INTEGER;
#define PRE_GETTER(config, msg, valueOnError) \
    assert(msg != NULL); \
    if (!config){ \
        *msg = SP_CONFIG_INVALID_ARGUMENT; \
        return valueOnError; \
    } \
    *msg = SP_CONFIG_SUCCESS;
#define PRINT_ERROR(file, line, message) \
    printf("File: %s\n" \
           "Line: %d\n" \
           "Message: %s\n", file, line, message);

#define RELEASE_MEMORY_AFTER_ERROR \
    free(line); \
    fclose(file); \
    spConfigDestroy(config);

const int dimensionArray[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                              20, 21, 22, 23, 24, 25, 26, 27, 28};

const int splitMethodArray[] = {0, 1, 2};

const int loggerLevelArray[] = {1, 2, 3, 4};

const char *suffixArray[] = {".jpg" , ".png" , ".bmp" , ".gif"};

struct sp_config_t {
    int spPCADimension;
    char *spPCAFilename;
    int spNumOfFeatures;
    bool spExtractionMode;
    bool spMinimalGUI;
    int spNumOfSimilarImages;
    int spKNN;
    enum {RANDOM, MAX_SPREAD, INCREMENTAL} spKDTreeSplitMethod;
    int spLoggerLevel;
    char *spLoggerFilename;
    char *spImagesDirectory;
    char *spImagesPrefix;
    char *spImagesSuffix;
    int spNumOfImages;
};

void initializeStruct(SPConfig conf){
    conf->spPCADimension = 20;
    conf->spNumOfFeatures = 100;
    conf->spExtractionMode = true;
    conf->spMinimalGUI = false;
    conf->spNumOfSimilarImages = 1;
    conf->spKNN = 1;
    conf->spKDTreeSplitMethod = 1;
    conf->spLoggerLevel = 3;
    conf->spPCAFilename = (char *)malloc(MAX_SIZE);
    strcpy(conf->spPCAFilename, "pca.yml");
    conf->spLoggerFilename = (char *)malloc(MAX_SIZE);
    strcpy(conf->spLoggerFilename, "stdout");
    conf->spImagesSuffix = (char *)malloc(MAX_SIZE);
    conf->spImagesSuffix[0] = '\0';
    conf->spImagesPrefix = (char *)malloc(MAX_SIZE);
    conf->spImagesPrefix[0] = '\0';
    conf->spImagesDirectory = (char *)malloc(MAX_SIZE);
    conf->spImagesDirectory[0] = '\0';
    conf->spNumOfImages = -1;

}

bool isCommentOrBlankLine(char *line){
    char c;
    for (int i = 0; i < strlen(line); i++){
        if (isspace(line[i])) continue;
        if (line[i] == '#')
            return true;
        return false;
    }
    return true;
}

void strip(char *strValue, char *dest){
    int start = 0, end = 0, offset;
    int strLen = (int)strlen(strValue);
    for (int i = 0; i < strLen; i++){
        if (isspace(strValue[i])) continue;
        start = i;
        break;
    }
    for (int i = strLen - 1; i >= start; i--){
        if (isspace(strValue[i])) continue;
        end = i;
        break;
    }
    offset = end - start + 1;
    strncpy(dest, strValue + start, offset);
    dest[offset] = '\0';
}

bool isValidRow(char *paramName, char *paramValue){
    for (int i = 0; i < strlen(paramName); i++)
        if (isspace(paramName[i])) return false;
    for (int i = 0; i < strlen(paramValue); i++)
        if (isspace(paramValue[i])) return false;
    return true;
}

bool parseRow(char *line, char **paramName, char **paramValue){
    if (!strchr(line, '=')) return false;
    char *tmpValue, *tmpName;
    tmpValue = strdup(line);
    tmpName = strsep(&tmpValue, "=");
    if (tmpName==NULL || tmpValue == NULL) return false;
    strip(tmpName, *paramName);
    strip(tmpValue, *paramValue);
    free(tmpName);
    return isValidRow(*paramName, *paramValue);
}

bool isInt(char *s)
{
    while (*s)
        if (!isdigit(*s++)) return false;
    return true;
}

bool isContainsInt(const int *arr, int value, int size){
    int i;
    for (i=0; i < size; i++)
        if (arr[i] == value)
            return true;
    return false;
}

bool isContainsString(const char **arr, char *value, int size){
    int i;
    for (i=0; i < size; i++)
        if (!strcmp(arr[i], value))
            return true;
    return false;
}

SP_CONFIG_MSG setAttribute(SPConfig conf, char *paramName, char *paramValue){
    if (!strcmp(paramName, "spExtractionMode") ){
        VALID_BOOL(paramValue)
        conf->spExtractionMode = paramValue == "true" ? true : false;
    }
    if (!strcmp(paramName, "spMinimalGUI") ){
        VALID_BOOL(paramValue)
        conf->spMinimalGUI = paramValue == "true" ? true : false;;
    }
    if (!strcmp(paramName, "spPCAFilename")) strcpy(conf->spPCAFilename, paramValue);
    if (!strcmp(paramName, "spLoggerFilename")) strcpy(conf->spLoggerFilename, paramValue);
    if (!strcmp(paramName, "spImagesDirectory")) strcpy(conf->spImagesDirectory, paramValue);
    if (!strcmp(paramName, "spImagesPrefix")) strcpy(conf->spImagesPrefix, paramValue);
    if (!strcmp(paramName, "spImagesSuffix")) {
        if (!isContainsString(suffixArray, paramValue, 4))
            return SP_CONFIG_INVALID_STRING;
        strcpy(conf->spImagesSuffix, paramValue);
    }
    if (!strcmp(paramName, "spPCADimension")){
        VALID_INT(paramValue)
        int paramValueInt = atoi(paramValue);
        VALID_INT_RANGE(dimensionArray, paramValueInt, 19);
        conf->spPCADimension = paramValueInt;
    }
    if (!strcmp(paramName, "spKDTreeSplitMethod")){
        VALID_INT(paramValue)
        int paramValueInt = atoi(paramValue);
        VALID_INT_RANGE(splitMethodArray, paramValueInt, 3);
        conf->spKDTreeSplitMethod = paramValueInt;
    }
    if (!strcmp(paramName, "spNumOfFeatures")){
        VALID_INT(paramValue)
        conf->spNumOfFeatures = atoi(paramValue);
    }
    if (!strcmp(paramName, "spNumOfSimilarImages")){
        VALID_INT(paramValue)
        conf->spNumOfSimilarImages = atoi(paramValue);
    }
    if (!strcmp(paramName, "spKNN")){
        VALID_INT(paramValue)
        conf->spKNN = atoi(paramValue);
    }
    if (!strcmp(paramName, "spLoggerLevel")){
        VALID_INT(paramValue)
        int paramValueInt = atoi(paramValue);
        VALID_INT_RANGE(loggerLevelArray, paramValueInt, 4);
        conf->spLoggerLevel = paramValueInt;
    }
    if (!strcmp(paramName, "spNumOfImages")){
        VALID_INT(paramValue)
        conf->spNumOfImages = atoi(paramValue);
    }
    return SP_CONFIG_SUCCESS;
}


SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
    assert(msg != NULL);
    if (!filename){
        *msg = SP_CONFIG_INVALID_ARGUMENT;
        return NULL;
    }
    FILE *file = fopen(filename, "r");
    if (!file) {
        *msg = SP_CONFIG_CANNOT_OPEN_FILE;
        return NULL;
    }
    SPConfig config = malloc(sizeof(*(SPConfig) NULL));
    if (!config){
        *msg = SP_CONFIG_ALLOC_FAIL;
        return NULL;
    }
    initializeStruct(config);
    char *line = NULL;
    int i = 0;
    size_t len = 0;
    ssize_t read;
    char *paramName;
    char *paramValue;
    SP_CONFIG_MSG attributeMsg;
    while ((read = getline(&line, &len, file)) != -1) {
        i ++;
        if (isCommentOrBlankLine(line)) continue;
        paramName = (char *)malloc(MAX_SIZE);
        paramValue = (char *)malloc(MAX_SIZE);
        if (!parseRow(line, &paramName, &paramValue)){
            free(paramName);
            free(paramValue);
            RELEASE_MEMORY_AFTER_ERROR
            PRINT_ERROR(filename, i, "Invalid Configuration Line")
            return NULL;
        }
        attributeMsg = setAttribute(config, paramName, paramValue);
        free(paramName);
        free(paramValue);
        if (attributeMsg != SP_CONFIG_SUCCESS) {
            RELEASE_MEMORY_AFTER_ERROR
            *msg = attributeMsg;
            PRINT_ERROR(filename, i, "Invalid value - constraint not met")
            return NULL;
        }
    }
    free(line);
    fclose(file);
    if (!strlen(config->spImagesDirectory))
        PRINT_ERROR(filename, i, "Parameter spImagesDirectory is not set")
    else if (!strlen(config->spImagesPrefix))
        PRINT_ERROR(filename, i, "Parameter spImagesPrefix is not set")
    else if (!strlen(config->spImagesSuffix))
        PRINT_ERROR(filename, i, "Parameter spImagesSuffix is not set")
    else if (config->spNumOfImages == -1)
        PRINT_ERROR(filename, i, "Parameter spNumOfImages is not set")
    *msg = SP_CONFIG_SUCCESS;
    return config;
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg){
    PRE_GETTER(config, msg, false)
    return config->spExtractionMode;
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
    PRE_GETTER(config, msg, false)
    return config->spMinimalGUI;
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg){
    PRE_GETTER(config, msg, -1)
    return config->spNumOfImages;
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg) {
    PRE_GETTER(config, msg, -1)
    return config->spNumOfFeatures;
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
    PRE_GETTER(config, msg, -1)
    return config->spPCADimension;
}

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
                                   int index) {
    if (!imagePath || !config) return SP_CONFIG_INVALID_ARGUMENT;
    SP_CONFIG_MSG *msg = (SP_CONFIG_MSG *)malloc(sizeof(SP_CONFIG_MSG));
    int numOfImages = spConfigGetNumOfImages(config, msg);
    char *directory = config->spImagesDirectory;
    char *prefix = config->spImagesPrefix;
    char *suffix = config->spImagesSuffix;
    char strIndex[MAX_SIZE];
    sprintf(strIndex, "%d", index);
    if (numOfImages > 0 && index > numOfImages) {
        free(msg);
        return SP_CONFIG_INDEX_OUT_OF_RANGE;
    }
    int j = 0;
    for (int i = 0;i < strlen(directory); i++)
        imagePath[j++] = directory[i];
    for (int i = 0;i < strlen(prefix); i++)
        imagePath[j++] = prefix[i];
    for (int i = 0;i < strlen(strIndex); i++)
        imagePath[j++] = strIndex[i];
    for (int i = 0;i < strlen(suffix); i++)
        imagePath[j++] = suffix[i];
    free(msg);
    return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config){
    if (!pcaPath || !config) return SP_CONFIG_INVALID_ARGUMENT;
    char *directory = config->spImagesDirectory;
    char *pcaFileName = config->spPCAFilename;
    int j = 0;
    for (int i = 0;i < strlen(directory); i++)
        pcaPath[j++] = directory[i];
    for (int i = 0;i < strlen(pcaFileName); i++)
        pcaPath[j++] = pcaFileName[i];
    return SP_CONFIG_SUCCESS;
}

void spConfigDestroy(SPConfig config) {
    if (!config) return;
    free(config->spImagesPrefix);
    free(config->spImagesSuffix);
    free(config->spImagesDirectory);
    free(config->spLoggerFilename);
    free(config->spPCAFilename);
    free(config);
}