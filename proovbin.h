#ifndef PROOVBIN_H
#define PROOVBIN_H 1

#include <malloc.h>
#include "queue.h"
#include "bntseq.h"

extern int global_bin_size; // use ugly global because init has no access to opts

// list aln struct
struct aln_t {
  int length, score;
  TAILQ_ENTRY(aln_t) alns;
};

// bin struct
typedef struct {
  int length;  // holds the sum of aln length
  TAILQ_HEAD(que_t, aln_t) que;
} bin_t;

typedef struct binseq_ {
  int n_bins;
  bin_t *bins;
} binseq_t;

void bns_bins_init (bntseq_t *bns);
void bns_bins_destroy (bntseq_t *bns);
void bns_bins_print (bntseq_t *bns);

int bins_pos2idx (int bin_size, int length, int pos);
int bins_assess_aln_by_score (bin_t *bin, int bin_length, int length, int score);


#endif /* PROOVBIN_H */
