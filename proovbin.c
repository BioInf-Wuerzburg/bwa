#include <malloc.h>
#include <stdio.h>
#include "queue.h"
#include "proovbin.h"

void bns_bins_init (bntseq_t *bns) {
  int i, j;

  bns->bin_size = global_bin_size;

  // basically what I want:
  // bns->binseqs[n_seqs].bins[n_bins]

  bns->binseqs = (binseq_t*) calloc(bns->n_seqs, sizeof(binseq_t));

  for(i=0;i<bns->n_seqs;i++){
    int n_bins = bns->anns[i].len / bns->bin_size + 1;

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
      while ( (ai = TAILQ_FIRST(&bns->binseqs[n].bins[b].que)) != NULL) {
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
  int n,b,len;

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



int bins_pos2idx (int bin_size, int length, int pos) {
  int b = ( pos + length/2 ) / bin_size;
  return b;
}


int bins_assess_aln_by_score (bin_t *bin, int bin_length, int length, int score) {
  //printf("%5d %5d\n", length, score);

  struct aln_t *a;
  struct aln_t *ai;
  struct aln_t *ar;

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
