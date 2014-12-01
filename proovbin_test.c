#include "proovbin.h"

int main () {

  int bin_length = 300;
  int bin_size = 20;

  int i;

  // init bwa bns struct
  bntseq_t bnstmp;
  bntseq_t *bns = &bnstmp;
  bns->n_seqs = 2;

  // init bins
  bns_bins_init(bns, bin_size);

  // some test data
  int data_s = 15;
  // sidx, len, score, pos
  int data[15][4] = {
    {0, 70, 4, 0},
    {0, 100, 7, 2},
    {0, 80, 6, 12},
    {0, 50, 3, 19},
    {0, 70, 5, 21},
    {0, 90, 4, 21},

    {0, 100, 7, 37},
    {0, 110, 6, 33},
    {0, 60, 3, 31},

    {1, 60, 5, 22},
    {1, 50, 4, 33},
    {1, 80, 7, 29},
    {1, 90, 6, 40},
    {1, 70, 3, 59},
    {1, 100, 5, 43}
  };

  // run test data
  for(i=0;i<data_s;i++){
    int n = data[i][0];
    int l = data[i][1];
    int s = data[i][3];
    int p = data[i][4];
    int b = bins_pos2idx(bin_size, l, p);

    bin_t *pbin = &bns->binseqs[n].bins[b];
    bins_assess_aln_by_score( pbin, bin_length, data[i][1], data[i][2] );
  }

  // print bin content
  bns_bins_print(bns);

  // cleanup bin memory
  bns_bins_destroy(bns);

  return 0;
}
