//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

/* SUMMARY
* ====================================================================================
* multiproc_mul     -   handlers wrapper;
* child_handler     -   handles child processes;
* parent_handler    -   handles ancestor process;
* send_to_pipe      -   formats packet;
* log_parse         -   log parser.
* ====================================================================================
**/

#pragma once

#ifndef MMUL_IPC_MATRIX_MMUL_H
#define MMUL_IPC_MATRIX_MMUL_H

#define HEAD_TYPE_SZ 2          //unsigned short int (2 bytes)
#define PAYLOAD_TYPE_SZ 8       //long int(8 bytes)
#define HEAD_SZ_BYTES 4         //HEAD_TYPE_SZ * HEAD_SZ
#define PAYLOAD_SZ_BYTES 2048   //PAYLOAD_TYPE_SZ * PAYLOAD_SZ
#define HEAD_SZ 2               //HEAD_SZ_BYTES / HEAD_TYPE_SZ
#define PAYLOAD_SZ 256          //PAYLOAD_SZ_BYTES / PAYLOAD_TYPE_SZ
#define LOG_FILENAME "./transmission_log"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include "matrix.h"
#include "colors.h"
#include "pbar.h"

// Process metadata structure.
typedef struct proc_meta {
    unsigned char total_proc;
    unsigned short int proc_index;
    int pipe_write;
} proc_meta;

matrix_t multiproc_mul(matrix_t *m_p0, matrix_t *m_p1, unsigned char proc_num);

void child_handler(matrix_t *m_p0, matrix_t *m_p1, proc_meta meta);

void send_to_pipe(proc_meta meta, long int *data, long int len);

void parent_handler(int pipe_read, matrix_t *result, unsigned char proc_num);

void log_parse(unsigned char proc_num);

#endif //MMUL_IPC_MATRIX_MMUL_H
