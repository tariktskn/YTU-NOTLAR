﻿#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

// Function to print a matrix stored in a 1D array
void print_matrix(unsigned* matrix, unsigned rows, unsigned cols, FILE* file);
// Function to read matrix from a file
void read_matrix(const char* filename, unsigned** matrix, unsigned* rows, unsigned* cols);
// Function to read kernel from a file
void read_kernel(const char* filename, unsigned** kernel, unsigned* k);
// Function to write output matrix to a file
void write_output(const char* filename, unsigned* output, unsigned rows, unsigned cols);
// Initialize output as zeros.
void initialize_output(unsigned*, unsigned, unsigned);

int main() {

    unsigned n, m, k;  // n = rows of matrix, m = cols of matrix, k = kernel size
    // Dynamically allocate memory for matrix, kernel, and output
    unsigned* matrix = NULL;  // Input matrix
    unsigned* kernel = NULL;  // Kernel size 3x3
    unsigned* output = NULL;  // Max size of output matrix

    char matrix_filename[30];
    char kernel_filename[30];

    // Read the file names
    printf("Enter matrix filename: ");
    scanf("%s", matrix_filename);
    printf("Enter kernel filename: ");
    scanf("%s", kernel_filename);


    // Read matrix and kernel from files
    read_matrix(matrix_filename, &matrix, &n, &m);  // Read matrix from file
    read_kernel(kernel_filename, &kernel, &k);      // Read kernel from file

    // For simplicity we say: padding = 0, stride = 1
    // With this setting we can calculate the output size
    unsigned output_rows = n - k + 1;
    unsigned output_cols = m - k + 1;
    output = (unsigned*)malloc(output_rows * output_cols * sizeof(unsigned));
    initialize_output(output, output_rows, output_cols);

    // Print the input matrix and kernel
    printf("Input Matrix: ");
    print_matrix(matrix, n, m, stdout);

    printf("\nKernel: ");
    print_matrix(kernel, k, k, stdout);

    /******************* KODUN BU KISMINDAN SONRASINDA DEĞİŞİKLİK YAPABİLİRSİNİZ - ÖNCEKİ KISIMLARI DEĞİŞTİRMEYİN *******************/

    // Assembly kod bloğu içinde kullanacağınız değişkenleri burada tanımlayabilirsiniz. ---------------------->
    // Aşağıdaki değişkenleri kullanmak zorunda değilsiniz. İsterseniz değişiklik yapabilirsiniz.
    unsigned matrix_value, kernel_value;    // Konvolüsyon için gerekli 1 matrix ve 1 kernel değişkenleri saklanabilir.
    unsigned sum;                           // Konvolüsyon toplamını saklayabilirsiniz.
    unsigned matrix_offset;                 // Input matrisi üzerinde gezme işleminde sınırları ayarlamak için kullanılabilir.
    unsigned tmp_si, tmp_di;                // ESI ve EDI döngü değişkenlerini saklamak için kullanılabilir.

    matrix_offset = k / 2;
    sum = 0;
    // Assembly dilinde 2d konvolüsyon işlemini aşağıdaki blokta yazınız ----->
    __asm {
        MOV ECX, output_rows        // ECX = output_rows
        XOR ESI, ESI                // i = 0

        L1 : PUSH ECX
        XOR EDI, EDI                // j = 0
        MOV ECX, output_cols        // ECX = output_cols
        MOV tmp_si, ESI             // ESI'nin degeri saklanir.

        L2 : MOV sum, 0             // sum = 0
        PUSH ECX
        MOV EAX, k                  // EAX = k
        MUL AX                      // EAX = k * k
        MOV ECX, EAX                // ECX = k * k
        MOV tmp_di, EDI             // EDI'nin degeri saklanir.
        XOR ESI, ESI                // z = 0 (kernel'in indisi) (zi = satir, zj = sutun)

        L3 : MOV EAX, ESI           // EAX = z
        MOV EBX, k                  // EBX = k
        XOR EDX, EDX                // EDX = 0
        DIV BX                      // EAX = z / k (zi), EDX = z % k (zj)
        MOV EBX, EDX                // EBX = EDX = z % k (zj)

        ADD EAX, tmp_si             // matrixIdx = zi + i
        MUL m                       // matrixIdx = (zi + i) * M
        ADD EBX, EAX                // matrixIdx = (zi + i) * M + zj
        ADD EBX, tmp_di             // matrixIdx = (zi + i) * M + zj + j

        MOV EDI, matrix             // EDI = matrix[]
        MOV EAX, [EDI + EBX * 4]    // EAX = matrix[matrixIdx]
        MOV EDI, kernel             // EDI = kernel[]
        MOV EBX, [EDI + ESI * 4]    // EBX = kernel[z]
        MUL EBX                     // EAX = matrix[matrixIdx] * kernel[z]
        ADD sum, EAX                // sum = matrix[matrixIdx] * kernel[z]
        INC ESI
        LOOP L3

        MOV EAX, tmp_si             // EAX = i
        MUL output_cols             // EAX = i * output_cols
        ADD EAX, tmp_di             // EAX = i * output_cols + j
        MOV EDI, EAX
        MOV EBX, output
        MOV EAX, sum
        MOV [EBX + EDI * 4], EAX    // output[i * output_cols + j] = sum

        MOV EDI, tmp_di             // EDI'ya degeri aktarilir.
        INC EDI
        POP ECX
        LOOP L2

        MOV ESI, tmp_si             // ESI'ya degeri aktarilir.
        INC ESI
        POP ECX
        LOOP L11                    // L1 range disinda kaldigi icin iki asamada atlanir.
        L11 : CMP ECX, 0
        JNE L1
    }
    /******************* KODUN BU KISMINDAN ÖNCESİNDE DEĞİŞİKLİK YAPABİLİRSİNİZ - SONRAKİ KISIMLARI DEĞİŞTİRMEYİN *******************/


    // Write result to output file
    write_output("./output.txt", output, output_rows, output_cols);

    // Print result
    printf("\nOutput matrix after convolution: ");
    print_matrix(output, output_rows, output_cols, stdout);

    // Free allocated memory
    free(matrix);
    free(kernel);
    free(output);

    return 0;
}

void print_matrix(unsigned* matrix, unsigned rows, unsigned cols, FILE* file) {
    if (file == stdout) {
        printf("(%ux%u)\n", rows, cols);
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%u ", matrix[i * cols + j]);
        }
        fprintf(file, "\n");
    }
}

void read_matrix(const char* filename, unsigned** matrix, unsigned* rows, unsigned* cols) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Read dimensions
    fscanf(file, "%u %u", rows, cols);
    *matrix = (unsigned*)malloc(((*rows) * (*cols)) * sizeof(unsigned));

    // Read matrix elements
    for (int i = 0; i < (*rows); i++) {
        for (int j = 0; j < (*cols); j++) {
            fscanf(file, "%u", &(*matrix)[i * (*cols) + j]);
        }
    }

    fclose(file);
}

void read_kernel(const char* filename, unsigned** kernel, unsigned* k) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Read kernel size
    fscanf(file, "%u", k);
    *kernel = (unsigned*)malloc((*k) * (*k) * sizeof(unsigned));

    // Read kernel elements
    for (int i = 0; i < (*k); i++) {
        for (int j = 0; j < (*k); j++) {
            fscanf(file, "%u", &(*kernel)[i * (*k) + j]);
        }
    }

    fclose(file);
}

void write_output(const char* filename, unsigned* output, unsigned rows, unsigned cols) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Write dimensions of the output matrix
    fprintf(file, "%u %u\n", rows, cols);

    // Write output matrix elements
    print_matrix(output, rows, cols, file);

    fclose(file);
}

void initialize_output(unsigned* output, unsigned output_rows, unsigned output_cols) {
    int i;
    for (i = 0; i < output_cols * output_rows; i++)
        output[i] = 0;

}

