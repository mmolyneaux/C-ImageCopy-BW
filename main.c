#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <unistd.h>

#define CT_SIZE 1024   // Bitmap color table size
#define HEADER_SIZE 54 // Bitmap file header size
#define VERSION "0.4"  // This is the 4th lesson / repo  of this program.

typedef struct {
    unsigned char header[HEADER_SIZE];
    uint32_t width;
    uint32_t height;
    uint32_t bitDepth;
    uint32_t imageType; // 1 for gray, 3 for color,might end up having to do
                        // with bitdepth.
    uint32_t imageSize;
    bool CT_EXISTS;
    unsigned char colorTable[CT_SIZE];
    unsigned char **imageBuffer; //[imgSize][3], 3 for rgb
} bitmap;

enum ImageType { GRAY = 1, RGB = 3 };
enum Mode { NO_mode, COPY, TO_GRAY, TO_BW };
char *mode_to_string(enum Mode mode) {
    switch (mode) {
    case NO_mode:
        return "No mode selected";
        break;
    case COPY:
        return "Copy only";
        break;
    case TO_GRAY:
        return "to Grayscale";
        break;
    case TO_BW:
        return "to Black & White";
        break;
    default:
        return "mode not found";    
    }
}
char* get_suffix(enum Mode mode) {
    switch (mode) {
    case NO_mode:
        return "_none";
        break;
    case COPY:
        return "_copy";
        break;
    case TO_GRAY:
        return "_gray";
        break;
    case TO_BW:
        return "_bw";
        break;
    default:
        return "_def";    
    }
}



// helper function, verify a filename ends with extension.
// returns true if str ends with the correct ext,
// returns false otherwise.
bool endsWith(char *str, const char *ext) {

    // Check for NULL;
    if (!str || !ext) {
        return false;
    }
    size_t len_str = strlen(str);
    size_t len_ext = strlen(ext);

    // return false if the str is shorter than the ext it's checking for.
    if (len_ext > len_str) {
        return false;
    }

    return strcmp(str + len_str - len_ext, ext) == 0;
}

// free memory allocated for bitmap structs.
void freeImage(bitmap *bmp) {
    for (int i = 0; i < bmp->imageSize; i++) {
        free(bmp->imageBuffer[i]);
    }
    free(bmp->imageBuffer);
    bmp->imageBuffer = NULL; // Avoid dangling pointer.
}
// returns false early and prints an error message if operation not complete.
// returns true on success of the operation.
bool readImage(char *filename1, bitmap *bitmapIn) {
    bitmapIn->imageType = 3; // multiplier for RGB

    FILE *streamIn = fopen(filename1, "rb");
    if (streamIn == NULL) {
        printf("Error opening file or file not found!\n");
        return false;
    }
    for (int i = 0; i < HEADER_SIZE; i++) {
        bitmapIn->header[i] = getc(streamIn);
    }

    // width starts at address of byte(char) 18, which is then cast to an
    // int*, so it can be dereferenced into an int, so it is cast to a 4
    // byte int instead stead of a single byte from the char header array.
    // Then the height can be retreived from the next 4 byts and so on.
    bitmapIn->width = *(int *)&bitmapIn->header[18];
    bitmapIn->height = *(int *)&bitmapIn->header[22];
    bitmapIn->bitDepth = *(int *)&bitmapIn->header[28];

    const size_t IMAGE_SIZE = bitmapIn->width * bitmapIn->height;

    // if the bit depth is less than or equal to 8 then we need to read the
    // color table. The read content is going to be stored in colorTable.
    // Not all bitmap images have color tables.
    if (bitmapIn->bitDepth <=
        8) { // by definition of bitmap, <= 8 has a color table
        bitmapIn->CT_EXISTS = true;
    }

    if (bitmapIn->CT_EXISTS) {
        fread(bitmapIn->colorTable, sizeof(char), CT_SIZE, streamIn);
    }
    // Allocate memory for the array of pointers (rows) for each pixel in
    // imagesize
    bitmapIn->imageBuffer =
        (unsigned char **)malloc(sizeof(char *) * IMAGE_SIZE);
    if (bitmapIn->imageBuffer == NULL) {
        return false;
    }

    // Allocate memory for each row (RGB values for each pixel)

    for (int i = 0; i < IMAGE_SIZE; i++) {
        bitmapIn->imageBuffer[i] =
            (unsigned char *)malloc(bitmapIn->imageType * sizeof(char *));
        if (bitmapIn->imageBuffer[i] == NULL) {
            return false;
        }
    }

    for (int i = 0; i < IMAGE_SIZE; i++) {
        bitmapIn->imageBuffer[i][0] = getc(streamIn); // red
        bitmapIn->imageBuffer[i][1] = getc(streamIn); // green
        bitmapIn->imageBuffer[i][2] = getc(streamIn); // blue
    }

    fclose(streamIn);
    return true;
}

void writeImage(char *filename, bitmap *bitmapIn) {
    FILE *streamOut = fopen(filename, "wb");
    printf("Filename2: %s\n", filename);

    fwrite(bitmapIn->header, sizeof(char), HEADER_SIZE, streamOut);

    if (bitmapIn->CT_EXISTS) {
        fwrite(bitmapIn->colorTable, sizeof(char), CT_SIZE, streamOut);
    }
    size_t imageSize = bitmapIn->width * bitmapIn->height;

    uint32_t temp = 0;
    for (int i = 0; i < imageSize; i++) {
        // the equation for mixing RGB to gray.
        temp = (bitmapIn->imageBuffer[i][0] * 0.3) +
               (bitmapIn->imageBuffer[i][1] * 0.59) +
               (bitmapIn->imageBuffer[i][2] * 0.11);
        // Write equally for each channel.
        putc(temp, streamOut); // red
        putc(temp, streamOut); // green
        putc(temp, streamOut); // blue
    }
    fclose(streamOut);
}

void readArgv() {}

void print_version() { printf("Program version: %s\n", VERSION); }

void print_usage(char *app_name) {
    printf("Usage: %s [OPTIONS] <input_filename> [output_filename]\n"
           "\n"
           "OPTIONS:\n"
           "  -h, --help           Show this help message and exit\n"
           "  -v, --verbose        Enable verbose output\n"
           "  --version            Show the program version\n"
           "\n"
           "ARGUMENTS:\n"
           "  <input_filename>  The required input filename\n"
           "  [output_filename]  An optional output filename\n"
           "\n"
           "EXAMPLES:\n"
           "  myprogram -v -g input.bmp\n"
           "  myprogram input.bmp output.bmp\n",
           app_name);
}

int main(int argc, char *argv[]) {
    enum Mode mode = NO_mode; // default
    int opt = 0;

    char *filename1 = NULL;
    char *filename2 = NULL;
    bool filename2_allocated = false;
    char *suffix = "_suffix"; // default
    char *extension = ".bmp";

    // if only the program name is called, print usage and exit.
    if (argc == 1) {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    // Parse command-line options
    bool g_flag = false,      // gray
        h_flag = false,       // help
        v_flag = false,       // verbose
        version_flag = false; // version

    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {"version", no_argument, 0, 0},
        {
            0,
            0,
            0,
            0,
        } // sentinal value indicating the end of the array, for getopt_long in
          // getopt.h
    };

    while ((opt = getopt(argc, argv, "ghv")) != -1) {
        switch (opt) {
        case 'g': // mode: TO_GRAY, to grayscale image
            g_flag = true;
            break;
        case 'h': // help
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 'v': // verbose
            v_flag = true;
            break;
        case 0: // checks for long options not tied to a short option
            if (strcmp("version", long_options[optind].name) == 0) {
                print_version();
                exit(EXIT_SUCCESS);
            }
            break;
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // set the mode
    if (g_flag) {
        mode = TO_GRAY;
    } else {
        mode = COPY;
    }

    // Check for required filename argument
    if (optind < argc) {
        filename1 = argv[optind];
        optind++;
    } else {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check for optional filename argument
    if (optind < argc) {
        filename2 = argv[optind];
    }

    // ---- Already read all the flags and filenames.
    // ---- help verbose and version already done
    // ---- filenames not verified
    // ---- Already fails if filename1 is null

    // confirm filename1 ends with extension
    if (!endsWith(filename1, extension)) {
        fprintf(stderr, "Error: Input file %s does not end with %s\n",
                filename1, extension);
        exit(EXIT_FAILURE);
    }
    // confirm filename2 ends with extension
    if (filename2) {
        if (!endsWith(filename2, extension)) {
            printf("Error: Input file %s does not end with %s\n", filename2,
                   extension);
            exit(EXIT_FAILURE);
        }
    } else { // create filename2 with proper suffix from mode
        // Find the last position of the  '.' in the filename
        char *dot_pos = strrchr(filename1, '.');
        if (dot_pos == NULL) {
            fprintf(stderr, "\".\" not found in filename: %s\n", filename1);
            exit(EXIT_FAILURE);
        }

        suffix = get_suffix(mode);

        // Calculate the length of the parts
        size_t base_len = dot_pos - filename1;
        size_t suffix_len = strlen(suffix);
        size_t extention_len = strlen(extension);

        filename2 = (char *)calloc(base_len + suffix_len + extention_len + 1,
                                   sizeof(char));
        if (filename2 == NULL) {
            printf("Memory allocation for output filename has failed.\n");
            exit(EXIT_FAILURE);
        }
        filename2_allocated = true;
        // Copy the base part of filename1 and append the suffix and extension.
        // strncpy copies the first base_len number of chars from filename1 into
        // filename2
        strncpy(filename2, filename1, base_len);
        // use ptr math to copy suffix to filename2ptr's + position + (can't use
        // strcat because strncpy doesn't null terminate.)
        strcpy(filename2 + base_len, suffix);
        strcpy(filename2 + base_len + suffix_len, extension);
    }

    if (v_flag) {
        printf("-g (to gray) flag: %s\n", g_flag ? "true" : "false");
        printf("-h (help) flag:    %s\n", h_flag ? "true" : "false");
        printf("-v (verbose) flag: %s\n", v_flag ? "true" : "false");
        printf("--version flag:    %s\n", version_flag ? "true" : "false");
        printf("filename1: %s\n", filename1);
        if (filename2)
            printf("filename2: %s\n", filename2);
        printf("mode: %s\n", mode_to_string(mode));
    }

    bitmap bitmapIn = {.header = {0},
                       .width = 0,
                       .height = 0,
                       .bitDepth = 0,
                       .imageSize = 0,
                       .imageType = 0,
                       .CT_EXISTS = false,
                       .colorTable = {0},
                       .imageBuffer = NULL};

    bool imageRead = readImage(filename1, &bitmapIn);
    if (!imageRead) {
        fprintf(stderr, "Image read failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("width: %d\n", bitmapIn.width);
    printf("height: %d\n", bitmapIn.height);
    printf("bitDepth: %d\n", bitmapIn.bitDepth);

    switch (mode) {
    case COPY:
        printf("Copy not ported, still gray.\n");
    case TO_GRAY:
        writeImage(filename2, &bitmapIn);
        break;
    default:
        printf("No mode matched.\n");
    }
    // free filename2 memory if it was allocated
    if (filename2_allocated && filename2 != NULL) {
        free(filename2);
        filename2 = NULL;
        filename2_allocated = false;
    }

    freeImage(&bitmapIn);

    return 0;
}