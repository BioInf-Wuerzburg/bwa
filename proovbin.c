#include <malloc.h>
#include <stdio.h>
#include <queue.h>

int bin_length_max = 300;
int bin_size = 20;

// list aln struct
struct aln_t {
  int length, score;
  TAILQ_ENTRY(aln_t) alns;
};


// bin struct
struct bin_t {
  int length;  // holds the sum of aln length
  TAILQ_HEAD(que_t, aln_t) que;
};


typedef struct {
  //	int64_t l_pac;
  //	int32_t n_seqs;
  int n_seqs;
  //	uint32_t seed;
  //	bntann1_t *anns; // n_seqs elements
  //	int32_t n_holes;
  //	bntamb1_t *ambs; // n_holes elements
  //	FILE *fp_pac;
  int bin_size;
  struct bin_t *bin_seqs;
} bntseq_t;


void init_bins (bntseq_t *bns, int bin_size) {
  int i, j;
  //struct bin_t *bs[bns->n_seqs];

  bns->bin_seqs = (struct bin_t*)calloc(bns->n_seqs, sizeof(struct bin_t));;
  bns->bin_size = bin_size;

  /*
  for(i=0;i<bns->n_seqs;i++){

    // init bins
    // int bins_n = bns->anns[i].len / bin_size;
    int bins_n = 100 / bin_size;
    struct bin_t bins[bins_n];

    for(j=0;j<bins_n;j++){
      TAILQ_INIT(&bins[j].que);
      bins[j].length = 0;
    }

    //bs[i] = &bins;
  }
  */

}

int main () {
  int i;

  bntseq_t bnstmp;
  bntseq_t *bns = &bnstmp;

  init_bins(bns, bin_size);

  int bins_s = 5;
  struct bin_t bins[bins_s];

  for(i=0;i<bins_s;i++){
    TAILQ_INIT(&bins[i].que);
    bins[i].length = 0;
  }


  int data_s = 15;
  int data[15][3] = {
    {70, 4},
    {100, 7},
    {80, 6},
    {50, 3},
    {70, 5, 1},
    {90, 4, 1},
    {100, 7, 1},
    {110, 6, 1},
    {60, 3, 4},
    {60, 5, 4},
    {50, 4, 4},
    {80, 7, 4},
    {90, 6, 3},
    {70, 3, 3},
    {100, 5, 3},
  };

  for(i=0;i<data_s;i++){
    struct bin_t *pbin = &bins[ data[i][2] ];
    assess_aln_by_score( pbin, data[i][0], data[i][1] );
  }

  struct aln_t *ai;

  for(i=0;i<bins_s;i++){
    printf("\nbin:  %4d\n----------\n", bins[i].length);
    TAILQ_FOREACH(ai, &bins[i].que, alns)
      printf(" %4d %4d\n", ai->score, ai->length);

    printf("----------\n");
  }

  // clean up
  for(i=0;i<bins_s;i++){
    while (ai = TAILQ_FIRST(&bins[i].que)) {
      TAILQ_REMOVE(&bins[i].que, ai, alns);
      free(ai);
    }
  }


  printf("BUYA\n");
  return 0;
}

int assess_aln_by_score (struct bin_t *bin, int length, int score) {
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

  else if (bin->length < bin_length_max)
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

      while (bin->length - TAILQ_FIRST(&bin->que)->length > bin_length_max)
        { // remove overflow
          ar = TAILQ_FIRST(&bin->que);
          TAILQ_REMOVE(&bin->que, ar, alns);
          bin->length -= ar->length;
          free(ar);
        }
    }
  return 1;
}
