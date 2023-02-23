//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

/* SUMMARY
* ====================================================================================
* print_progress    -   prints current process in terminal;
* pbar_init         -   inits progress indicator;
* update_progress   -   refresh current progress;
* ====================================================================================
* */

#pragma once

#ifndef MMUL_IPC_PBAR_H
#define MMUL_IPC_PBAR_H

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

#include <stdlib.h>
#include <stdio.h>
#include "colors.h"

typedef struct pbar_t {
    double iter;
    double progress;
    double upd_counter;
} pbar_t;

void print_progress(double percentage);

pbar_t pbar_init(unsigned long int seq);

void update_progress(pbar_t *pbar, unsigned int upd);

#endif //MMUL_IPC_PBAR_H
