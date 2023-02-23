//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

#include "pbar.h"

pbar_t pbar_init(unsigned long int seq) {
    pbar_t pbar = {
            .iter = 1.0 / (double) seq,
            .progress = 0.00,
            .upd_counter = 0.0
    };
    return pbar;
}

void update_progress(pbar_t *pbar, unsigned int upd) {
    pbar->progress += pbar->iter * (double) upd;
    if (pbar->progress - pbar->upd_counter >= 0.01) {
        if (pbar->progress > 0.99) {
            pbar->progress = 1.0;
        }
        print_progress(pbar->progress);
        pbar->upd_counter = pbar->progress;
    }
}

void print_progress(double percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf(GRN"\r%3d%% [%.*s%*s]"RESET, val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
