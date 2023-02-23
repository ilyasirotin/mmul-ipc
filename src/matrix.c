//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

#include "matrix.h"

matrix_t create_matrix(unsigned long int m, unsigned long int n) {
    matrix_t new_matrix;
    new_matrix.m = m;
    new_matrix.n = n;
    new_matrix.size = m * n;

    // Allocate memory for matrix row pointers.
    new_matrix.matrix = (long int **) malloc(sizeof(long int *) * m);
    if (!(new_matrix.matrix)) {
        perror(RED"malloc"RESET);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for matrix elements.
    new_matrix.matrix[0] = (long int *) malloc(sizeof(long int) * m * n);
    if (!(new_matrix.matrix[0])) {
        perror(RED"malloc"RESET);
        exit(EXIT_FAILURE);
    }

    // Split memory for rows.
    for (unsigned long int i = 1; i < new_matrix.m; i++) {
        new_matrix.matrix[i] = new_matrix.matrix[0] + i * new_matrix.n;
    }

    return new_matrix;
}

int fill_matrix(matrix_t *m_p, const char *pattern) {
    unsigned long int i = 0;
    long int write_value = 0;

    if (!strcmp(pattern, "r")) {
        srand(time(NULL));
        for (i = 0; i < m_p->size; i++) {
            // Init matrix by random numbers.
            write_value = rand() % 32 - 16;
            m_p->matrix[0][i] = write_value;
        }
    } else {
        for (i = 0; i < m_p->size; i++) {
            // Init by exact number.
            write_value = strtol(pattern, 0, 10);
            m_p->matrix[0][i] = write_value;
        }
    }
    return 0;
}

void get_row(matrix_t *m_p, unsigned long int n, long int *destination) {
    // Save values to array.
    for (unsigned long int i = 0; i < m_p->m; i++) {
        destination[i] = m_p->matrix[i][n];
    }
    return;
}

long int mul_vectors(long int *vect_0, long int *vect_1, unsigned long int len) {
    long int result = 0;
    for (unsigned long int i = 0; i < len; i++) {
        result += vect_0[i] * vect_1[i];
    }
    return result;
}

matrix_t mul_matrix(matrix_t *m_p0, matrix_t *m_p1) {
    long int row[m_p0->m];
    long int row_mul = 0;
    matrix_t result = create_matrix(m_p1->m, m_p0->n);
    pbar_t pbar = pbar_init(m_p0->n);

    // Multiply matrices.
    for (unsigned long int i = 0; i < m_p0->n; i++) {
        get_row(m_p0, i, row);
        for (unsigned long int j = 0; j < m_p1->m; j++) {
            row_mul = mul_vectors(row, m_p1->matrix[j], m_p1->n);
            result.matrix[j][i] = row_mul;
        }
        // Show progress bar...
        update_progress(&pbar, 1);
    }
    fprintf(stdout, "\n");
    return result;
}

void save_matrix(const char *filename, matrix_t *matrix) {
    FILE *file = NULL;
    long int matrix_row[matrix->m];
    char str[32] = {0};
    file = fopen(filename, "w+t");
    if (file == NULL) {
        perror(RED"create"RESET);
        return;
    }

    fprintf(file, "Matrix %jux%ju:\n", matrix->m, matrix->n);

    // Save results to file.
    for (unsigned int i = 0; i < matrix->n; i++) {
        get_row(matrix, i, matrix_row);
        for (unsigned int j = 0; j < matrix->m; j++) {
            sprintf(str, "%jd%s", matrix_row[j], "\0");
            fprintf(file, "%s ", str);
        }
        fprintf(file, "\n");
    }
    if (fclose(file)) {
        perror(RED"close"RESET);
    }
    return;
}

void free_matrix(matrix_t *m_p) {
    free(m_p->matrix[0]);
    free(m_p->matrix);
}
