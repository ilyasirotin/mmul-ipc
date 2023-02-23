//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

#include "matrix_mmul.h"

// close() function wrapper.
static inline __attribute__((always_inline)) void __close(int fd) {
    if (close(fd) == -1) {
        perror(RED"close"RESET);
    }
}

matrix_t multiproc_mul(matrix_t *m_p0, matrix_t *m_p1, unsigned char proc_num) {
    int pipe_fd[2] = {0};
    pid_t child_pid = 0;
    proc_meta child_meta = {.total_proc = proc_num, 0, 0};
    matrix_t result = create_matrix(m_p1->m, m_p0->n);

    if (pipe(pipe_fd) == -1) {
        perror(RED"pipe"RESET);
        exit(EXIT_FAILURE);
    }

    // Save transmission channel descriptor in child process structure.
    child_meta.pipe_write = pipe_fd[1];

    // Clone processes.
    for (int i = 0; i < proc_num; i++) {
        switch (child_pid = fork()) {
            case -1:
                perror(RED"fork"RESET);
                exit(EXIT_FAILURE);
            case 0:
                __close(pipe_fd[0]); // Close child receiving pipe.
                child_meta.proc_index = i; // Save PID
                child_handler(m_p0, m_p1, child_meta); // Close child handler.
                __close(pipe_fd[1]); // Close transmission pipe.
                exit(EXIT_SUCCESS); // Terminate child process.
            default:
                fprintf(stdout, "Process started with pid: %d\n", child_pid); // Show child PID.
                break;
        }
    }

    __close(pipe_fd[1]); // Close ancestor transmission pipe.
    parent_handler(pipe_fd[0], &result, proc_num); // Call parent handler.
    __close(pipe_fd[0]); // Close ancestor receiving pipe.

    return result;
}

void send_to_pipe(proc_meta meta, long int *payload, long int len) {
    struct iovec iov[2];
    // Packet header. Save sender PID.
    unsigned short int head[HEAD_SZ] = {meta.proc_index, 0};

    ssize_t wr_stat = 0; // Write status of channel.

    // Actual payload size.
    unsigned short int payload_size = PAYLOAD_SZ; // Write channel descriptor.
    int pipe_fd = meta.pipe_write;

    // Size adjustment.
    if (len < payload_size) {
        payload_size = len;
    }

    head[1] = payload_size; // Write data size to header.

    iov[0].iov_base = head; // Initialize header segment buffer.
    iov[0].iov_len = sizeof(head);

    iov[1].iov_base = payload; // Initialize data segment buffer
    iov[1].iov_len = payload_size * PAYLOAD_TYPE_SZ;

    // Send both segments to channel.
    while (len != 0 && (wr_stat = writev(pipe_fd, iov, 2)) != 0) {
        if (wr_stat == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror(RED"pipe write"RESET);
            break;
        }

        iov[1].iov_base += payload_size; // Displace data buffer pointer.

        len -= payload_size; // Subtract sent data size.

        // Adjust data array if current sequence size < previous sequence size.
        if (len < payload_size) {
            payload_size = len;
            head[1] = payload_size;
            iov[1].iov_len = payload_size * PAYLOAD_TYPE_SZ;
        }
    }

    return;
}

void child_handler(matrix_t *m_p0, matrix_t *m_p1, proc_meta meta) {
    long int result_vect[m_p1->m];
    long int buff_row[m_p0->m];

    // Initial row pointer is PID.
    // The following row of matrix is current row number + number of processes.
    for (unsigned long int i = meta.proc_index; i < m_p0->n; i += meta.total_proc) {
        get_row(m_p0, i, buff_row); // Select a row.
        for (unsigned long int j = 0; j < m_p1->m; j++) {
            // Multiply vectors.
            result_vect[j] = mul_vectors(buff_row, m_p1->matrix[j], m_p0->m);
        }

        // Send data to pipe.
        send_to_pipe(meta, result_vect, m_p1->m);
    }
    return;
}

void parent_handler(int pipe_read, matrix_t *result, unsigned char proc_num) {
    unsigned short int head[HEAD_SZ]; // Accepted packet header.
    long int payload[PAYLOAD_SZ]; // Data buffer.
    unsigned long int row_counter[proc_num]; // Row counters of each process.
    unsigned long int row_pos[proc_num]; // Row displacements of each process.
    unsigned char current_proc = 0; // Accepted packet PID.
    unsigned long int current_row = 0; // Current row of sender process.
    unsigned long int current_pos = 0; // Current displacement of sender process.

    pid_t exit_child_pid = 0;
    unsigned char exit_child_counter = 0;
    int exit_child_status = 0;
    ssize_t rd_stat = 0;
    size_t wr_stat = 0;
    int log_fd = 0;

    // Progress bar.
    pbar_t pbar = pbar_init(result->size);

    for (unsigned char i = 0; i < proc_num; i++) {
        row_counter[i] = i;
        row_pos[i] = 0;
    }

    // Init transmission log.
    log_fd = open(LOG_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if (log_fd == -1) {
        perror(RED"transmission_log create"RESET);
    }

    // Read header from channel.
    while ((rd_stat = read(pipe_read, head, HEAD_SZ_BYTES)) != 0) {
        if (rd_stat == -1) {
            perror(RED"pipe head read"RESET);
        }

        // Log header.
        wr_stat = write(log_fd, head, HEAD_SZ_BYTES);
        if (wr_stat == -1) {
            perror(RED"fwrite transmission_log"RESET);
        }

        // Set displacements.
        current_proc = head[0];
        current_pos = row_pos[current_proc];
        current_row = row_counter[current_proc];

        // Save data to buffer.
        rd_stat = read(pipe_read, payload, head[1] * PAYLOAD_TYPE_SZ);
        if (rd_stat == -1) {
            perror(RED"pipe payload read"RESET);
        } else if (rd_stat) {
            // Save data to matrix.
            for (unsigned long int i = 0; i < head[1]; i++) {
                result->matrix[current_pos][current_row] = payload[i];
                ++current_pos;
            }

            // Check displacements.
            if (current_pos >= result->m) {
                row_counter[current_proc] += proc_num;
                row_pos[current_proc] = 0;
            } else {
                row_pos[current_proc] = current_pos;
            }
        }

        // Update progress bar.
        update_progress(&pbar, head[1]);
    }

    fprintf(stdout, "\n");

    // Wait for child processes termination. Get the completion status.
    while (exit_child_counter != proc_num) {
        exit_child_pid = wait(&exit_child_status);
        if (exit_child_pid == -1) {
            perror(RED"waitpid"RESET);
        } else {
            ++exit_child_counter;
            fprintf(stdout, "Process with pid: %d\n\t", exit_child_pid);
            if (WIFEXITED(exit_child_status)) {
                if (WEXITSTATUS(exit_child_status) == 0) {
                    fprintf(stdout, GRN"Exit with: 0\n"RESET);
                } else {
                    fprintf(stdout, RED"Exit with: %d\n"RESET, WEXITSTATUS(exit_child_status));
                }
            }
        }
    }

    // Force logs saving.
    fsync(log_fd);
    __close(log_fd);

    return;
}

void log_parse(unsigned char proc_num) {
    int log_fd = 0;
    ssize_t rd_stat = 0;

    unsigned short int read_buff[HEAD_SZ] = {0};
    unsigned long int transaction_counter[proc_num];
    unsigned long int block_counter[proc_num];
    unsigned long int total_blocks = 0;
    unsigned long int total_transactions = 0;

    for (unsigned int i = 0; i < proc_num; i++) {
        transaction_counter[i] = 0;
        block_counter[i] = 0;
    }

    log_fd = open("./transmission_log", O_RDONLY);
    if (log_fd == -1) {
        perror(RED"transmission_log open"RESET);
    }

    while ((rd_stat = read(log_fd, read_buff, HEAD_SZ_BYTES)) != 0) {
        if (rd_stat == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror(RED"transmission_log read"RESET);
            exit(EXIT_FAILURE);
        }
        transaction_counter[read_buff[0]]++;
        block_counter[read_buff[0]] += read_buff[1];
        total_transactions += read_buff[0];
        total_blocks += read_buff[1];
    }

    fprintf(stdout, YEL"Process transaction statistic:\n"RESET);

    for (unsigned int i = 0; i < proc_num; i++) {
        fprintf(stdout, CYN"Process %d:\n\t"RESET, i);
        fprintf(stdout, "Transactions: %ju\n\t", transaction_counter[i]);
        fprintf(stdout, "Blocks: %ju\n\t", block_counter[i]);
        fprintf(stdout, "Bytes: %ju\n"RESET, block_counter[i] * PAYLOAD_TYPE_SZ);
    }

    fprintf(stdout, YEL"Total transactions: %ju\n"RESET, total_transactions);
    fprintf(stdout, YEL"Total blocks received: %ju\n"RESET, total_blocks);

    __close(log_fd);
    return;
}
