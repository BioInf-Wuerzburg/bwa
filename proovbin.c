#include <malloc.h>
#include <stdio.h>
#include <queue.h>

int bin_length_max = 300;

// list aln struct
struct aln_t {
  int length, score;
  TAILQ_ENTRY(aln_t) alns;
};


// bin struct
struct bin_t {
  int length;  // holds the sum of aln length in bins if aln is bin head
  TAILQ_HEAD(que_t, aln_t) que; // newline to prevent emacs auto-indent fail :P
} bin;


int main () {
  //struct bin_t bin;
  TAILQ_INIT(&bin.que);

  int data_s = 15;
  int data[15][2] = {
    {70, 4},
    {100, 7},
    {80, 6},
    {50, 3},
    {70, 5},
    {90, 4},
    {100, 7},
    {110, 6},
    {60, 3},
    {60, 5},
    {50, 4},
    {80, 7},
    {90, 6},
    {70, 3},
    {100, 5},
  };

  int i;
  for(i=0;i<data_s;i++){
    assess_aln_by_score( data[i][0], data[i][1] );
  }

  struct aln_t *a1;
  // clean up
  while (a1 = TAILQ_FIRST(&bin.que)) {
    TAILQ_REMOVE(&bin.que, a1, alns);
    free(a1);
  }

  printf("BUYA\n");
  return 0;
}

int assess_aln_by_score (int length, int score) {
  //printf("%5d %5d\n", length, score);

  struct aln_t *a;
  struct aln_t *ai;
  struct aln_t *ar;
  struct aln_t *am;

  a = malloc(sizeof(struct aln_t));
  a->length = length;
  a->score = score;
  printf("\nassessing:\n%4d %4d\n", a->score, a->length);

  // empty bin
  if (TAILQ_EMPTY(&bin.que))
    {
      printf("empty\n");
      bin.length = length;  // first item => total length == item length
      TAILQ_INSERT_HEAD(&bin.que, a, alns);
    }

  // partially filled bin
  else if (bin.length < bin_length_max)
    {
      printf("part: ");
      // new min score aln
      if (a->score <= TAILQ_FIRST(&bin.que)->score)
        {
          printf("min\n");

          TAILQ_INSERT_HEAD(&bin.que, a, alns);
          // update length
          bin.length = bin.length + a->length;

        }
      // new max score
      else if (a->score >= TAILQ_LAST(&bin.que, que_t)->score)
        {
          printf("max\n");

          TAILQ_INSERT_TAIL(&bin.que, a, alns);
          // update length
          bin.length += a->length;
        }
      // non-min score aln
      else
        {
          printf("within\n");

          TAILQ_FOREACH(ai, &bin.que, alns){
            if (a->score < ai->score)
              break;
          }

          TAILQ_INSERT_BEFORE(ai, a, alns);
          // update length
          bin.length += a->length;

        }
    }

  // full bin
  else
    {
      printf("full: ");
      // new min score aln
      if (a->score <= TAILQ_FIRST(&bin.que)->score)
        {
          printf("min\n");
          return 0;
        }

      // new max score
      if (a->score >= TAILQ_LAST(&bin.que, que_t)->score)
        {
          printf("max");

          TAILQ_INSERT_TAIL(&bin.que, a, alns);
          // update length
          bin.length += a->length;
        }
      // non-min score aln
      else
        {
          printf("within");

          TAILQ_FOREACH(ai, &bin.que, alns){
            if (a->score < ai->score)
              break;
          }

          TAILQ_INSERT_BEFORE(ai, a, alns);
          // update length
          bin.length += a->length;
        }

      // remove overflow
      while (bin.length - TAILQ_FIRST(&bin.que)->length > bin_length_max)
        {
          printf(" rm");
          ar = TAILQ_FIRST(&bin.que);
          TAILQ_REMOVE(&bin.que, ar, alns);
          // update length
          bin.length -= ar->length;
          free(ar);
        }
      printf("\n");
    }

  printf("      %4d\n----------\n", bin.length);
  TAILQ_FOREACH(ai, &bin.que, alns)
    printf(" %4d %4d\n", ai->score, ai->length);

  printf("----------\n");


}
