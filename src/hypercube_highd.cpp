/*
 *  hypercube_highd.cpp
 *  Haploids
 *
 *
 * Copyright (c) 2012, Richard Neher, Fabio Zanini
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ffpopsim_highd.h"

hypercube_highd::hypercube_highd()
{
mem=false;
if (HCF_VERBOSE) cerr<<"hypercube_highd::hypercube_highd(): constructing...!\n";
}

//constructor
hypercube_highd::hypercube_highd(int dim_in, int s) : dim(dim_in), mem(false), hcube_allocated(false), hypercube_mean(0), epistatic_std(0) {
	if (HCF_VERBOSE) cerr<<"hypercube_highd::hypercube_highd(): constructing...!\n";
	set_up(dim_in, s);
}

//set up and allocate memory, perform basic consistency checks on input.
int hypercube_highd::set_up(int dim_in, int s) {
	if (dim_in>0) {
		if (HCF_VERBOSE) cerr<<"hypercube_highd::set_up(): setting up...!";
                // FIXME: we should really go for a global rng seeder for the whole library
		if (s==0) s = time(NULL);
		seed = s;
		if (HCF_VERBOSE) cerr<<"done.\n";
		return allocate_mem(dim_in);
	} else {
		cerr <<"hypercube_highd: need positive dimension!\n";
		return HCF_BADARG;
	}
}

//destructor
hypercube_highd::~hypercube_highd() {
	if (HCF_VERBOSE) cerr<<"hypercube_highd::~hypercube_highd(): destructing...!\n";
	if (mem) free_mem();
}

//allocate the necessary memory
int hypercube_highd::allocate_mem(int dim_in) {
	if (HCF_VERBOSE) cerr<<"hypercube_highd::allocate_mem(): allocating memory...";
	if (mem) {
		cerr <<"hypercube_highd::allocate_mem(): memory already allocated, freeing and reallocating ...!\n";
		free_mem();
	}

	//a random number generator used for the coefficients and the like
	rng=gsl_rng_alloc(RNG);
	gsl_rng_set(rng, seed);
	rng_offset = gsl_rng_uniform_int(rng, 1000000);
        if (HCF_VERBOSE) {
        	cerr <<"hypercube_highd() random number seed: "<<seed<<endl;
	        cerr <<"hypercube_highd() random number offset: "<<rng_offset<<endl;
        }

	// set the static array of single locus coefficients
	coefficients_single_locus_static = new double[dim_in];

	mem=true;
	if (HCF_VERBOSE) cerr<<"done.\n";
	return 0;
}

//free the memory
int hypercube_highd::free_mem() {
	if (!mem) {
		cerr <<"hypercube_highd::free_mem(): no memory allocated...!\n";
		return 0;
	}
 	mem=false;
	delete [] coefficients_single_locus_static;
 	gsl_rng_free(rng);
 	return 0;
}

/**
 * @brief Get single value on the hypercube
 *
 * @param genotype Point of the hypercube
 *
 * @returns the value corresponding to that point
 */
double hypercube_highd::get_func(boost::dynamic_bitset<>& genotype) {
	if (HCF_VERBOSE) cerr<<"fluct_hypercube::get_func()"<<endl;
	double result=hypercube_mean;
	int sign, locus;
	// first order contributions
	for (coefficients_single_locus_iter = coefficients_single_locus.begin();
	     coefficients_single_locus_iter != coefficients_single_locus.end();
	     coefficients_single_locus_iter++) {
		if ((genotype)[coefficients_single_locus_iter->locus]) result+=coefficients_single_locus_iter->value;
		else result -= coefficients_single_locus_iter->value;
	}
	// interaction contributions
	for (coefficients_epistasis_iter = coefficients_epistasis.begin();
	     coefficients_epistasis_iter != coefficients_epistasis.end();
	     coefficients_epistasis_iter++) {
		sign=1;
		for (locus=0; locus < coefficients_epistasis_iter->order; locus++) {
			if (!(genotype)[coefficients_epistasis_iter->loci[locus]]) sign*= -1;
		}
		result += sign * coefficients_epistasis_iter->value;
	}
	// calculate the random fitness part
	if (epistatic_std > HP_NOTHING) {
		int gt_seed=0;
		int word=0, locus, ii;
		// calculate the seed for the random number generator
		for (locus=0;locus<dim;) {
			word=0;
			ii=0;
			while (ii<WORDLENGTH and locus<dim) {
				if ((genotype)[locus]) word+=(1<<ii);
				ii++; locus++;
			}
			gt_seed+=word;
		}
		// add a gaussion random number to the fitness from the rng seeded with the genoytpe
		gsl_rng_set(rng,gt_seed+rng_offset);
		result+=gsl_ran_gaussian(rng,epistatic_std);
	}
	return result;
}

/**
 * @brief Calculate difference between two hypercube points efficiently
 *
 * @param genotype1 first point on the hypercube
 * @param genotype2 second point on the hypercube
 * @param diffpos vector of positions at which they differ
 *
 * @returns the difference between the values, f(gt1) - f(gt2)
 */
double hypercube_highd::get_func_diff(boost::dynamic_bitset<>& genotype1, boost::dynamic_bitset<>& genotype2, vector<int> &diffpos) {
	if (HCF_VERBOSE) cerr<<"fluct_hypercube::get_func_diff()"<<endl;
	double result = 0;
	int locus;
	// first order contributions
	for(size_t i=0; i != diffpos.size(); i++) {
		locus = diffpos[i];
		if(genotype1[locus]) result += 2 * get_additive_coefficient(locus);
		else result -= 2 * get_additive_coefficient(locus);
	}
	// TODO: calculate epistasis more efficiently!
	// interaction contributions
	int sign1, sign2;
	for (coefficients_epistasis_iter = coefficients_epistasis.begin();
	     coefficients_epistasis_iter != coefficients_epistasis.end();
	     coefficients_epistasis_iter++) {
		sign1 = sign2 = 1;
		for (locus=0; locus < coefficients_epistasis_iter->order; locus++) {
			if (!(genotype1)[coefficients_epistasis_iter->loci[locus]]) sign1 *= -1;
			if (!(genotype2)[coefficients_epistasis_iter->loci[locus]]) sign2 *= -1;
		}
		if(sign1 != sign2)
			result += (sign1 - sign2) * coefficients_epistasis_iter->value;
	}
	// calculate the random fitness part
	if (epistatic_std > HP_NOTHING) {
		int gt_seed=0;
		int word=0, locus, ii;
		// calculate the seed for the random number generator
		for (locus=0;locus<dim;) {
			word=0;
			ii=0;
			while (ii<WORDLENGTH and locus<dim) {
				if ((genotype1)[locus]) word+=(1<<ii);
				ii++; locus++;
			}
			gt_seed+=word;
		}
		// add a gaussion random number to the fitness from the rng seeded with the genoytpe
		gsl_rng_set(rng,gt_seed+rng_offset);
		result+=gsl_ran_gaussian(rng,epistatic_std);
	}
	return result;
}


/**
 * @brief: get the trait coefficient of a locus.
 *
 * Loop over the additive coefficients and return the value once the locus is found.
 */
double hypercube_highd::get_additive_coefficient(int locus){
//	int l=0;
//	while(l<coefficients_single_locus.size()) {
//		if (coefficients_single_locus[l].locus==locus)
//			return coefficients_single_locus[l].value;
//		l++;
//	}
//	return 0.0;
	return coefficients_single_locus_static[locus];
}


/**
 * @brief Reset the hypercube
 */
void hypercube_highd::reset() {
	hypercube_mean = epistatic_std = 0;
	coefficients_single_locus.clear();
	coefficients_epistasis.clear();
	double *tmp = coefficients_single_locus_static;
	for(size_t i=0; i != dim; i++, tmp++) {
		*tmp = 0;
	}
}


/**
 * @brief Reset the additive part of the hypercube
 */
void hypercube_highd::reset_additive() {
	hypercube_mean = 0;
	coefficients_single_locus.clear();
	double *tmp = coefficients_single_locus_static;
	for(size_t i=0; i != dim; i++, tmp++) {
		*tmp = 0;
	}
}


/**
 * @brief assign single- or multi-locus trait coefficients.
 *
 * It takes a double and assigns it to a trait coefficients with index set
 * specified in vector loci.
 */
int hypercube_highd::add_coefficient(double value, vector <int> loci)
{
	if (loci.size()>1) {
		coeff_t temp_coeff(value, loci);
		coefficients_epistasis.push_back(temp_coeff);
	} else if (loci.size()==1) {
		coeff_single_locus_t temp_coeff(value, loci[0]);
		coefficients_single_locus.push_back(temp_coeff);
		coefficients_single_locus_static[loci[0]] = value;
	} else {
		hypercube_mean=value;
	}
	return 0;
}

/**
 * @brief reset the value of an additive coefficient
 *
 * Note: it requires the index of the coefficient and the locus it is supposed to refer to.
 */
int hypercube_highd::set_additive_coefficient(double value, int lindex, int expected_locus)
{
	//check whether the locus is what was expected
	if (coefficients_single_locus[lindex].locus==expected_locus){
		coefficients_single_locus[lindex].value=value;
		return 0;
	}else if (expected_locus>-1){
		cerr <<"hypercube_highd::set_additive_coefficient: coefficient[locus] does not match locus: "<<coefficients_single_locus[lindex].locus<<"  "<<expected_locus<<endl;
		return HCF_BADARG;
	}
}

/**
 * @brief set the standard deviation of random epistatic fitness
 */
int hypercube_highd::set_random_epistasis_strength(double sigma)
{
	if (sigma>0){epistatic_std=sigma; return 0;}
	else{
		cerr<<"hypercube_highd::set_random_epistasis_strength(): Epistasis strength has to be positive!"<<endl;
		return HCF_BADARG;
	}
}