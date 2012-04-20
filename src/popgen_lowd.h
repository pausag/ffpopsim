/**
 * @file popgen_lowd.h
 * @brief Header file for low-dimensional simulations
 * @author Richard Neher, Boris Shraiman, Fabio Zanini
 * @version 
 * @date 2012-04-19
 */
#ifndef POPGEN_LOWD_H_
#define POPGEN_LOWD_H_

#define HC_MEMERR -131545		//memory error code
#define HC_BADARG -131546		//bad argument error code
#define HC_VERBOSE 0			//debugging: if set to one, each function prints out a message into the error stream
#define HC_FUNC 1			//hypercube.func is up-to-date
#define HC_COEFF -1			//hypercube.coeff is up-to-date
#define HC_FUNC_EQ_COEFF 0		//hypercube.func equal hypercube.coeff

using namespace std;

/**
 * @brief Binary hypercube used in low-dimensional simulations.
 *
 * This class is a generic object that can be used to represent various things, e.g.
 * - the fitness landscape or any other phenotypic landscape;
 * - the genotype frequencies of a population with a genome of size L.
 *
 * If you are planning to model a whole population evolving on the hypercube, see the class haploid_gt_dis.
 *
 * Notes on scalability:
 * - The number of genotypes to store increases as \f$2^L\f$, where L is the number of sites. This class can thus only be used for \f$L \lesssim 20\f$.
 * - The population size N is actually unimportant, as far as it can be stored as a long integer. In other words, this class scales with N like O(1).
 */
class hypercube
{
public:
	//dimension of the hypercube
	int dim;

	int state;				//takes values HC_FUNC, HC_COEFF, HC_HC_FUNC_EQ_COEFF, depending on the current state of hypercube
	double *coeff;				//array holding 2^N coefficients: a entry 0101001101 corresponds to a term with spins at each 1
	double *func;				//array holding the values of the function on the hypercube

	//random number generator
	gsl_rng *rng;
	unsigned int seed;

	//checks and memory
	bool mem;
	int allocate_mem();
	int free_mem();
	int *order;				//Auxiliary array holding the number of spins, i.e. the number of ones of coeff[k]

	hypercube();
	hypercube(int dim_in, int s=0);
	~hypercube();
	int set_up(int dim_in, int s=0);

	// set coefficients
	int gaussian_coefficients(double* vark, bool add=false);
	int additive(double* additive_effects, bool add=false);
	int init_rand_gauss(double sigma, bool add=false);
	int init_list(vector<index_value_pair> iv, bool add=false);
	int init_coeff_list(vector <index_value_pair> iv, bool add=false);
	void calc_order();
	void set_state(int s){state=s;}

	//in and out
	int read_coeff(istream &in);
	int write_func(ostream &out);
	int write_coeff(ostream &in,  bool label=false);
	int read_func(istream &out);
	int read_func_labeled(istream &in);

	//analysis
	int signature(int point);

	//transform from coefficients to function and vice versa
	int fft_func_to_coeff();
	int fft_coeff_to_func();

	//read out
	int get_state() {return state;}
	unsigned int get_dim(){return dim;}
	unsigned int get_seed() {return seed;}
	double get_func(int point) {if (state==HC_COEFF) {fft_coeff_to_func();} return func[point]; }
	double get_coeff(int point) {if (state==HC_FUNC) {fft_func_to_coeff();} return coeff[point]; }

	//operations on the function
	int argmax();
	double valuemax();
	void func_set(int point, double f) {func[point]=f; }
	void func_increment(int point, double f) {func[point]+=f; }
	int normalize(double targetnorm=1.0);
	int reset();
	int scale(double scale);
	int shift(double shift);
	int test();
};


#define HG_LONGTIMEGEN 1000000
#define HG_CONTINUOUS 10000
#define HG_NOTHING 1e-15
#define HG_EXTINCT -9287465
#define HG_MEMERR -32656845

/**
 * @brief Low-dimensional population evolving on the hypercube.
 *
 * This class enables simulation of short genomes (\f$L \lesssim 20\f$) but potentially large populations.
 * Random mutation, recombination and selection are supported.
 * A number of properties of the population can be obtained using methods of this class, including:
 * - genotype and allele frequencies;
 * - statistics on fitness and phenotypic traits;
 * - linkage disequilibrium.
 */
class haploid_gt_dis
{
protected:
	//hypercubes that store the distribution of recombinations and the change in the
	//population distribution due to mutations
	hypercube recombinants;
	hypercube mutants;
	double** recombination_patters;				//array that holds the probabilities of all possible recombination outcomes for every subset of loci

	double population_size;
	int number_of_loci;
	int generation;
	double long_time_generation;
	double** mutation_rates;				//mutation rate can be made locus specific and genotype dependent.
	double outcrossing_rate;
	bool free_recombination;

	//random number generator used for resampling and seeding the hypercubes
	gsl_rng* rng;	//uses the same RNG as defined in hypercube.h from the  GSL library.
	int seed;	//seed of the rng

	int allocate_mem();
	int free_mem();
	bool mem;
public:
	// public hypercubes
	hypercube fitness;
	hypercube population;

	//setting up
	haploid_gt_dis();
	~haploid_gt_dis();
	haploid_gt_dis(int nloci, double popsize, int rngseed=0);
	int setup(int nloci, double popsize, int rngseed=0);

	//initialization
	int init_frequencies(double *freq);
	int init_genotypes(vector <index_value_pair> gt);
	int set_recombination_rates(double *rec_rates);
	void set_population_size(double popsize){population_size=popsize;}
	void set_outcrossing_rate(double orate){outcrossing_rate=orate;}
	int set_mutation_rate(double m);
	int set_mutation_rate(double m1, double m2);
	int set_mutation_rate(double* m);
	int set_mutation_rate(double** m);

	//evolution
	int evolve(int gen=1);
	int evolve_norec(int gen=1);
	int evolve_deterministic(int gen=1);
	int select();
	int mutate();
	int recombine();
	int resample(double n=0.0);
	int calculate_recombinants();
	int calculate_recombinants_free();
	int calculate_recombinants_general();

	//analyze and access population
	int L(){return number_of_loci;}
	double N(){return population_size;}
	double get_genotype_frequency(int gt){return population.get_func(gt);}
	double get_allele_frequency(int locus){return 0.5*(1+(1<<number_of_loci)*population.get_coeff(1<<locus));}
	double get_chi(int locus){return (1<<number_of_loci)*population.get_coeff(1<<locus);}
	double get_moment(int locus1, int locus2){return (1<<number_of_loci)*population.get_coeff((1<<locus1)+(1<<locus2));}
	double get_LD(int locus1, int locus2){return get_moment(locus1, locus2)-get_chi(locus1)*get_chi(locus2);}
	double get_generation(){return long_time_generation+generation;}
	double genotype_entropy();
	double allele_entropy();
	double fitness_mean();
	double fitness_variance();
	double get_population_size(){return population_size;}
	double get_mutation_rate(int locus, int direction) {return mutation_rates[direction][locus];}
	double get_outcrossing_rate() {return outcrossing_rate;}

	//testing
	int test_recombinant_distribution();
	int test_recombination(double *rec_rates);
	int mutation_drift_equilibrium(double** mutrates);
};


#endif /* POPGEN_LOWD_H_ */
