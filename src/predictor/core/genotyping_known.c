/*
 * Copyright 2014 Zamin Iqbal (zam@well.ox.ac.uk)
 * 
 *
 * **********************************************************************
 *
 * This file is part of myKrobe.
 *
 * myKrobe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * myKrobe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with myKrobe.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **********************************************************************
 */
/*
  genotyping_known.c - taking known resistance mutations and genotyping
*/


#include "genotyping_known.h"
#include "string_buffer.h"

//assume you are going to repeatedly call this on a fasta
//file of known mutations, with the first allele being WT/susceptible.
void get_next_mutation_allele_info(FILE* fp, dBGraph* db_graph, AlleleInfo* rinfo,
				   Sequence* seq, KmerSlidingWindow* kmer_window,
				   (*file_reader)(FILE * fp, 
						  Sequence * seq, 
						  int max_read_length, 
						  boolean new_entry, 
						  boolean * full_entry),
				   dBNode** array_nodes, Orientation*  array_or,
				   CovgArray* working_ca)
{
  

  int num_kmers = 
    align_next_read_to_graph_and_return_node_array(fp, 
						   max_read_length, 
						   array_nodes, 
						   array_or, 
						   false, 
						   &full_entry, 
						   file_reader_fasta,
						   seq, 
						   kmer_window, 
						   db_graph, 
						   dummy_colour_ignored);

  strbuf_reset(rinfo->name_of_gene);
  strbuf_append_str(rinfo->name_of_gene, seq->seq);

  //collect min, median covg on allele and also percentage of kmers with any covg
  boolean too_short=false;
  rinfo->susceptible_allele->median_covg = 
    median_covg_on_allele_in_specific_colour(array_nodes, 
					     num_kmers, 
					     working_ca, 
					     0, 
					     &too_short);
  rinfo->susceptible_allele->min_covg = 
    min_covg_on_allele_in_specific_colour(array_nodes, 
					  num_kmers, 
					  0, 
					  &too_short);
  rinfo->susceptible_allele->percent_nonzero = 
    percent_nonzero_on_allele_in_specific_colour(array_nodes, 
						 num_kmers, 
						 0, 
						 &too_short);


  num_kmers = align_next_read_to_graph_and_return_node_array(fp, 
							     max_read_length, 
							     array_nodes, 
							     array_or, 
							     false, 
							     &full_entry, 
							     file_reader_fasta,
							     seq, 
							     kmer_window, 
							     db_graph, 
							     dummy_colour_ignored);

  
  too_short=false;
  rinfo->resistant_allele->median_covg = 
    median_covg_on_allele_in_specific_colour(array_nodes, 
					     num_kmers, 
					     working_ca,
					     0,
					     &too_short);
  rinfo->resistant_allele->min_covg = 
    min_covg_on_allele_in_specific_colour(array_nodes,
					  num_kmers,
					  0,
					  &too_short);
  rinfo->resistant_allele->percent_nonzero = 
    percent_nonzero_on_allele_in_specific_colour(array_nodes,
						 num_kmers,
						 0,
						 &too_short);
  
}
