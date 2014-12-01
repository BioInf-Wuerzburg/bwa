#include <malloc.h>
#include <stdio.h>
#include "queue.h"

int bin_length = 300;
int bin_size = 20;

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

typedef struct {
  int n_bins;
  bin_t *bins;
} binseq_t;

typedef struct {
  //    int64_t l_pac;
  //    int32_t n_seqs;
  int n_seqs;
  //    uint32_t seed
  //    bntann1_t *anns; // n_seqs elements
  //    int32_t n_holes;
  //    bntamb1_t *ambs; // n_holes elements
  //    FILE *fp_pac;
  int bin_size;
  binseq_t *binseqs;
} bntseq_t;


void bns_bins_init (bntseq_t *bns, int bin_size) {
  int i, j;

  bns->bin_size = bin_size;

  // basically what I want:
  // bns->binseqs[n_seqs].bins[n_bins]

  bns->binseqs = (binseq_t*) calloc(bns->n_seqs, sizeof(binseq_t));

  for(i=0;i<bns->n_seqs;i++){
    // int n_bins = bns->anns[i].len / bin_size + 1;
    int n_bins = 5;

    bin_t *bins;
    bins = (bin_t*) calloc(n_bins, sizeof(bin_t));

    for(j=0;j<n_bins;j++){
      TAILQ_INIT(&bins[j].que);
      bins[j].length = 0;
    }

    bns->binseqs[i].n_bins = n_bins;
    bns->binseqs[i].bins = bins;
  }

}

void bns_bins_destroy (bntseq_t *bns) {
  struct aln_t *ai;
  int n,b;
  // clean up
  for(n=0; n<bns->n_seqs; n++){ // loop seqs
    for(b=0; b<bns->binseqs[n].n_bins; b++){ // loop bins
      while (ai = TAILQ_FIRST(&bns->binseqs[n].bins[b].que)) {
        TAILQ_REMOVE(&bns->binseqs[n].bins[b].que, ai, alns);
        free(ai);
      }
    }
    free(bns->binseqs[n].bins);
  }
  free(bns->binseqs);
}

void bns_bins_print (bntseq_t *bns) {
  struct aln_t *ai;
  int n,b,len,sco;

  printf("#seq bin    len  alignments\n");
  for(n=0; n<bns->n_seqs; n++){ // loop seqs
    printf(">%d\n", n);
    for(b=0; b<bns->binseqs[n].n_bins; b++){ // loop bins
      len = bns->binseqs[n].bins[b].length;
      printf("     #%-4d %4d ------------\n", b, len);
      TAILQ_FOREACH(ai, &bns->binseqs[n].bins[b].que, alns)
        printf("                | %4d %4d\n", ai->score, ai->length);
    }
  }
}


int main () {
  int i;

  // init bwa bns struct
  bntseq_t bnstmp;
  bntseq_t *bns = &bnstmp;
  bns->n_seqs = 2;

  // init bins
  bns_bins_init(bns, bin_size);

  // some test data
  int data_s = 15;
  // sidx, len, score, bin
  int data[15][4] = {
    {0, 70, 4},
    {0, 100, 7},
    {0, 80, 6},
    {0, 50, 3},
    {0, 70, 5, 1},
    {0, 90, 4, 1},
    {0, 100, 7, 1},
    {0, 110, 6, 1},
    {0, 60, 3, 4},
    {1, 60, 5, 4},
    {1, 50, 4, 4},
    {1, 80, 7, 4},
    {1, 90, 6, 3},
    {1, 70, 3, 3},
    {1, 100, 5, 3}
  };

  // run test data
  for(i=0;i<data_s;i++){
    int n = data[i][0];
    int b = data[i][3];
    bin_t *pbin = &bns->binseqs[n].bins[b];
    assess_aln_by_score( pbin, data[i][1], data[i][2]);
  }

  // print bin content
  bns_bins_print(bns);

  // cleanup bin memory
  bns_bins_destroy(bns);

  return 0;
}

int assess_aln_by_score (bin_t *bin, int length, int score) {
  //printf("%5d %5d\n", length, score);

  struct aln_t *a;
  struct aln_t *ai;
  struct aln_t *ar;
  struct aln_t *am;

  a = malloc(sizeof(struct aln_t));
  a->length = length;
  a->score = score;

  if (TAILQ_EMPTY(&bin->que))
    { // empty bin
      bin->length = length;  // first item => total length == item length
      TAILQ_INSERT_HEAD(&bin->que, a, alns);
    }

  else if (bin->length < bin_length)
    { // partially filled bin
      if (a->score <= TAILQ_FIRST(&bin->que)->score)
        { // new min score aln
          TAILQ_INSERT_HEAD(&bin->que, a, alns);
          bin->length = bin->length + a->length;
        }
      else if (a->score >= TAILQ_LAST(&bin->que, que_t)->score)
        { // new max score
          TAILQ_INSERT_TAIL(&bin->que, a, alns);
          bin->length += a->length;
        }
      else
        { // non-min score aln
          TAILQ_FOREACH(ai, &bin->que, alns){
            if (a->score < ai->score)
              break;
          }
          TAILQ_INSERT_BEFORE(ai, a, alns);
          bin->length += a->length;
        }
    }

  else
    { // full bin
      if (a->score <= TAILQ_FIRST(&bin->que)->score)
        { // new min score aln
          return 0;
        }
      if (a->score >= TAILQ_LAST(&bin->que, que_t)->score)
        { // new max score
          TAILQ_INSERT_TAIL(&bin->que, a, alns);
          bin->length += a->length;
        }
      else
        { // non-min score aln - find pos
          TAILQ_FOREACH(ai, &bin->que, alns){
            if (a->score < ai->score)
              break;
          }
          TAILQ_INSERT_BEFORE(ai, a, alns);
          bin->length += a->length;
        }

      while (bin->length - TAILQ_FIRST(&bin->que)->length > bin_length)
        { // remove overflow
          ar = TAILQ_FIRST(&bin->que);
          TAILQ_REMOVE(&bin->que, ar, alns);
          bin->length -= ar->length;
          free(ar);
        }
    }
  return 1;
}
