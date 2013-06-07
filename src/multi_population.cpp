#include "multi_population.h"

class hh_1 : public haploid_highd{

    virtual double trait_function(double aTrait1, double aTrait2, double phi_0 = 0)
    {
        //OFF-CENTER
        double rho1_0 =  0;
        double rho2_0 =  0;
        double rho_0 = sqrt(pow(rho1_0, 2) + pow(rho2_0, 2));

        //VARIANCE
        double sigma = 1;


        double t1 = aTrait1;
        double t2 = aTrait2;

        double rho = sqrt(pow(t1, 2) + pow(t2, 2));
        double phi = 0;
        if (rho != 0 )
            phi = acos(t1/rho);


        return  cos(phi - phi_0) * exp(-(pow(rho-rho_0, 2)) / (2 * sigma));
    }

public:

    //virtual void calc_individual_fitness_from_traits(int clonenum) {calc_individual_fitness_from_traits(population[clonenum]);}

    virtual void calc_individual_fitness_from_traits(clone_t &tempgt) {tempgt.fitness = trait_function(tempgt.trait[0], tempgt.trait[1], 0);}

    hh_1(){}
    ~hh_1(){}
};

class hh_2 : public haploid_highd{

    virtual double trait_function(double aTrait1, double aTrait2, double phi_0 = 0)
    {
        double rho1_0 =  1;
        double rho2_0 =  0;
        double sigma = 1;

        double rho_0 = sqrt(pow(rho1_0, 2) + pow(rho2_0, 2));
        double t1 = aTrait1;
        double t2 = aTrait2;

        double rho = sqrt(pow(t1, 2) + pow(t2, 2));
        double phi = 0;
        if (rho != 0 )
            phi = acos(t1/rho);

        return  cos(phi - phi_0) * exp(-(pow(rho-rho_0, 2)) / (2 * sigma));
    }

public:

    virtual void calc_individual_fitness_from_traits(clone_t &tempgt) {tempgt.fitness = trait_function(tempgt.trait[0], tempgt.trait[1], 3.14/4);}
    hh_2(){}
    ~hh_2(){}
};



class hh_3 : public haploid_highd{

    virtual double trait_function(double aTrait1, double aTrait2, double phi_0 = 0)
    {
        double rho1_0 =  1;
        double rho2_0 =  0;
        double sigma = 1;

        double rho_0 = sqrt(pow(rho1_0, 2) + pow(rho2_0, 2));
        double t1 = aTrait1;
        double t2 = aTrait2;

        double rho = sqrt(pow(t1, 2) + pow(t2, 2));
        double phi = 0;
        if (rho != 0 )
            phi = acos(t1/rho);

        return  cos(phi - phi_0) * exp(-(pow(rho-rho_0, 2)) / (2 * sigma));
    }

public:

    virtual void calc_individual_fitness_from_traits(clone_t &tempgt) {tempgt.fitness = trait_function(tempgt.trait[0], tempgt.trait[1], 3 * 3.14/4);}
    hh_3(){}
    ~hh_3(){}
};


multi_population::multi_population(int new_locations, int L_in, int n_o_traits)
{

    //sub_population = new haploid_highd[new_locations]; //.push_back(haploid_highd());


    //Location number check:
    if (new_locations < 3)
    {
        if (HP_VERBOSE)
            cerr << "too few locations provided for the required type of simulations!"<< endl;
        throw(HP_BADARG);
    }



   haploid_highd* loc;

   loc = new hh_2();
   loc->set_up(L_in, 0, n_o_traits);
   sub_population.push_back(loc);


   loc = new hh_3();
   loc->set_up(L_in, 0, n_o_traits);
   sub_population.push_back(loc);


   for (int i = 2; i < new_locations; i ++)
    {

        loc = new hh_1();
        loc->set_up(L_in, 0, n_o_traits);
        sub_population.push_back(loc);

    }
   cout << sub_population.size() << endl;

    number_of_locations = new_locations;
    track_genealogy = 0; //No genealogy by default
    migration_rate = 0.0; //No migration by default
    number_of_migration_events = 0;

}




multi_population::~multi_population()
{
    for (int i = 0; i < number_of_locations; i ++)
     {
        //delete sub_population[i];
     }



}




int multi_population::population_size()
{
    int size = 0;
    for (int i = 0; i < number_of_locations; i ++)
    {
        size += sub_population[i]->population.size();
    }

    return size;
}


void multi_population::reset()
{
    number_of_locations = 0;
    //sub_population.clear();

}



int multi_population::track_locus_genealogy(vector <int > loci)
{


    if(HP_VERBOSE){cerr <<"MULTI_POPULATION::track_locus_genealogy(vector <int> loci)... number of loci="<<loci.size();}
    genealogy.reset();
    for (unsigned int i = 0; i < loci.size(); i ++)
    {
        vector <node_t> temp_generation;
        genealogy.newGenerations.push_back(temp_generation);
        genealogy.track_locus(loci[i]);
    }
    genealogy.extend_storage(10 + population_size());
    if (HP_VERBOSE){cerr<<"done\n";}

    //set_global_generation(-1);
    for (int location_No = 0; location_No < number_of_locations; location_No ++)

    {
        point_sub_pop(location_No)->track_locus_genealogy_outside(2, loci);

    }

    track_genealogy = 2;
    return 0;
}

int multi_population::submit_pop_genealogy()
{
    genealogy.newGenerations.clear();
    vector <node_t> temp_generation;
    for (int i = 0; i < genealogy.trees.size(); i ++)
    {
        genealogy.newGenerations.push_back(temp_generation);

    }

    for (int i = 0; i < number_of_locations; i ++)
    {

        for (unsigned int loc_No = 0; loc_No < genealogy.trees.size(); loc_No ++)
        {
            for (unsigned int node_No = 0; node_No <= sub_population[i]->last_clone; node_No ++)
            {
                if (sub_population[i]->newGenerations[loc_No][node_No].clone_size != 0)
                {
                    genealogy.newGenerations[loc_No].push_back(sub_population[i]->newGenerations[loc_No][node_No]);
                    genealogy.newGenerations[loc_No].back().own_key.location = i;
                    //cout << "Node added to newGenerations   "<< genealogy.newGenerations[loc_No].back().own_key <<" size: "<< genealogy.newGenerations[loc_No].back().clone_size << endl;// "   " << genealogy.newGenerations[loc_No].back().own_key.location << "  " << endl;
                    //cout << "Parent:    "<< genealogy.newGenerations[loc_No].back().parent_node << endl;

                }



            }

        }

    }

    //cout << endl << endl;


    return 0;
}




double multi_population::max_fitness()
{
    double return_value = 0;
    for (int i = 0; i < number_of_locations; i ++)
    {
        if (return_value < sub_population[i]->get_max_fitness())
            return_value = sub_population[i]->get_max_fitness();

    }
    return return_value;
}


/**
 @brief multi_population::migrate
 * @return
 */



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++

int multi_population::migrate()
{
    //cout << endl << endl << "In migrate()" << endl;
    for (int i = 0; i < number_of_locations; i ++)
    {
        int actual_no_migrants = determine_number_of_migrants(i);
        if (actual_no_migrants > 0)
        {
            //cout << "Migrants from location "<< i << ":  "<< actual_no_migrants << endl;
            sub_population[i]->produce_random_sample(actual_no_migrants);


            for (int migrant = 0; migrant < actual_no_migrants; migrant ++)
            {
                int migrant_clone_No = sub_population[i]->random_clone();


                int migrant_clone_destination = determine_migration_destination();
                //cout << "Clone "<< migrant_clone_No << " is to migrate now to "<< migrant_clone_destination << "..." << endl;

                if (i != migrant_clone_destination){
                    transfer_clone(i, migrant_clone_destination, migrant_clone_No);
                    number_of_migration_events ++;
                }


            }
            sub_population[i]->random_sample.clear();

        }


    }

return 0;
}


int multi_population::determine_number_of_migrants(int sub_pop_No)
{
    int number_of_migrants;
    number_of_migrants = gsl_ran_poisson(sub_population[sub_pop_No]->evo_generator, sub_population[sub_pop_No]->N()*migration_rate);


    return number_of_migrants;
}

int multi_population::determine_migration_destination()
{
  int destination;
  if (number_of_locations == 1) destination = 0;
  else destination = gsl_rng_uniform_int(sub_population[0]->evo_generator, number_of_locations);
  return destination;

}

int multi_population::transfer_clone(int sub_pop_source, int sub_pop_destination, int source)
{
    if (sub_population[sub_pop_source]->last_clone < source)
        return -1;
    if (sub_population[sub_pop_source]->population[source].clone_size < 1)
        return -1;


    if (sub_population[sub_pop_destination]->available_clones.size() == 0)
        sub_population[sub_pop_destination]->provide_at_least(1);
    int new_gt = sub_population[sub_pop_destination]->available_clones.back();
    sub_population[sub_pop_destination]->available_clones.pop_back();

    sub_population[sub_pop_destination]->population[new_gt].genotype = sub_population[sub_pop_source]->population[source].genotype;
    sub_population[sub_pop_destination]->population[new_gt].clone_size = 1;



    //Update fitness of the new clone
    sub_population[sub_pop_destination]->calc_individual_traits(sub_population[sub_pop_destination]->population[new_gt]);
    sub_population[sub_pop_destination]->calc_individual_fitness_from_traits(sub_population[sub_pop_destination]->population[new_gt]);
    sub_population[sub_pop_destination]->check_individual_maximal_fitness(sub_population[sub_pop_destination]->population[new_gt]);
    sub_population[sub_pop_destination]->update_traits();
    sub_population[sub_pop_destination]->update_fitness();

    //location[destination]->population_size ++;
    sub_population[sub_pop_destination]->last_clone = (new_gt < sub_population[sub_pop_destination]->last_clone)?sub_population[sub_pop_destination]->last_clone:new_gt;



    //Update general sub-population parameters
    sub_population[sub_pop_source]->population_size --;
    sub_population[sub_pop_source]->population[source].clone_size -- ;
    if (sub_population[sub_pop_source]->population[source].clone_size < 1)
    {
        sub_population[sub_pop_source]->population[source].clone_size = 0;
        sub_population[sub_pop_source]->available_clones.push_back(source);
        sub_population[sub_pop_source]->number_of_clones -- ;
    }
    sub_population[sub_pop_destination]->number_of_clones ++;
    sub_population[sub_pop_destination]->population_size ++;

    // Update parameters for tree
    if (track_genealogy == 2) {
        for (unsigned int genlocus=0; genlocus < genealogy.loci.size(); genlocus++)
        {


           add_migrating_clone_to_genealogy
                    (   genlocus,
                        sub_pop_source,
                        sub_pop_destination,
                        new_gt,
                        source,
                        sub_population[sub_pop_source]->newGenerations[genlocus][source].crossover[0],
                       sub_population[sub_pop_source]->newGenerations[genlocus][source].crossover[1], 1, 1
                    );



            sub_population[sub_pop_source]->newGenerations[genlocus][source].clone_size --;


        }
    }
    return 0;

}


void multi_population::set_global_generation(int new_generation)
{
    generation = new_generation;
    for (int i = 0; i < number_of_locations; i ++)
    {
        sub_population[i]->generation = new_generation;
    }
    return;

}

void multi_population::add_migrating_clone_to_genealogy(int locusIndex, int old_location, int new_location,  int dest, int parent, int left, int right, int cs, int n)
{

    if (HP_VERBOSE) {
        cerr <<"multi_population::add_migrating_clone_to_genealogy(): dest:  "<<dest<<" parent: "<<parent<<endl;
        tree_key_t temp;
        temp = sub_population[old_location]->newGenerations[locusIndex][parent].parent_node;
        /*temp.age = generation - 1;
        temp.index = sub_population[old_location].newGenerations[locusIndex][parent].parent_node.index;
        temp.location = sub_population[old_location].newGenerations[locusIndex][parent].parent_node.location;*/
        if (genealogy.trees[locusIndex].check_node(temp)){
            cerr <<"multi_population::add_clone_to_genealogy(): parent node ok"<<endl;
        }else{
            cerr <<"multi_population::add_clone_to_genealogy(): parent node DOES NOT EXIST!"<<endl;
        }
    }
    sub_population[new_location]->newGenerations[locusIndex][dest].parent_node = sub_population[old_location]->newGenerations[locusIndex][parent].parent_node;

    sub_population[new_location]->newGenerations[locusIndex][dest].own_key.index = dest;
    sub_population[new_location]->newGenerations[locusIndex][dest].own_key.age = generation;
    sub_population[new_location]->newGenerations[locusIndex][dest].own_key.location = new_location;

    sub_population[new_location]->newGenerations[locusIndex][dest].fitness = sub_population[new_location]->population[dest].fitness;
    sub_population[new_location]->newGenerations[locusIndex][dest].number_of_offspring = n;
    sub_population[new_location]->newGenerations[locusIndex][dest].clone_size = cs;
    sub_population[new_location]->newGenerations[locusIndex][dest].crossover[0] = left;
    sub_population[new_location]->newGenerations[locusIndex][dest].crossover[1] = right;

    if (sub_population[new_location]->number_of_traits != sub_population[new_location]->newGenerations[locusIndex][dest].traits.size())
    {
        sub_population[new_location]->newGenerations[locusIndex][dest].traits.clear();
        for (int traitNo = 0; traitNo < sub_population[new_location]->number_of_traits; traitNo ++)
        {
            sub_population[new_location]->newGenerations[locusIndex][dest].traits.push_back(sub_population[new_location]->population[dest].trait[traitNo]);
        }
    }else
    {
        for (int traitNo = 0; traitNo < sub_population[new_location]->number_of_traits; traitNo ++)
        {
            sub_population[new_location]->newGenerations[locusIndex][dest].traits[traitNo] = sub_population[new_location]->population[dest].trait[traitNo];

        }
    }


    //cout << "New node created!  " << " age = " << sub_population[new_location].newGenerations[locusIndex][dest].own_key.age << " location = " << sub_population[new_location].newGenerations[locusIndex][dest].own_key.location << " index = " << sub_population[new_location].newGenerations[locusIndex][dest].own_key.index << endl;
    //cout << "Parent:  " << " age = " << sub_population[new_location].newGenerations[locusIndex][dest].parent_node.age << " location = " << sub_population[new_location].newGenerations[locusIndex][dest].parent_node.location << " index = " << sub_population[new_location].newGenerations[locusIndex][dest].parent_node.index << endl;




    if (HP_VERBOSE) {
        cerr <<"multi_population::add_clone_to_genealogy(): done"<<endl;
    }
    return;
}


 int multi_population::mutate(int location)
{

    if (HP_VERBOSE)	cerr <<"multi_population::mutate() ..."<<endl;

    vector <int> mutations;
    int tmp_individual=0, nmut=0;
    size_t mutant;
    sub_population[location]->allele_frequencies_up_to_date = false;
    int actual_n_o_mutations, actual_n_o_mutants;
    if (sub_population[location]->mutation_rate > HP_NOTHING and not sub_population[location]->all_polymorphic) {
        //determine the number of individuals that are hit by at least one mutation
        actual_n_o_mutants = gsl_ran_poisson(sub_population[location]->evo_generator, (1.0-exp(-sub_population[location]->mutation_rate*sub_population[location]->number_of_loci))*sub_population[location]->population_size);
        sub_population[location]->produce_random_sample(1 + min(actual_n_o_mutants, sub_population[location]->population_size));

        //make sure enough empty clones are available to accomodate the new mutants
        sub_population[location]->provide_at_least(min(actual_n_o_mutants, sub_population[location]->population_size));

        //loop over the mutant individuals and introduce the mutations
        for (int individual = 0; individual != actual_n_o_mutants; individual++) {
            //determine the target clone
            mutant = sub_population[location]->random_clone();
            //determine the number of mutation it suffers. this should be Poisson conditional on having at least one
            //in practice the solution is fine but it is somewhat inaccurate for multiple mutations
            actual_n_o_mutations = gsl_ran_poisson(sub_population[location]->evo_generator, sub_population[location]->number_of_loci * sub_population[location]->mutation_rate)+1;
            //introduce the mutations, not that flip_single_locus returns the number of new mutant, which is fed back into
            //flip_single_locus to introduce the next mutation
            for (int i = 0; i != actual_n_o_mutations; i++)
                mutant=flip_single_locus(location, mutant, gsl_rng_uniform_int(sub_population[location]->evo_generator, sub_population[location]->number_of_loci));
        }
    } else if(sub_population[location]->all_polymorphic) {
        if(HP_VERBOSE) cerr <<"haploid_highd::mutate(): keeping all loci polymorphic"<<endl;
        sub_population[location]->calc_allele_freqs(); //calculate the allele frequencies
        nmut=0;
        for (int locus=0; locus<sub_population[location]->L(); locus++){	//loop over all loci
            if (fabs(2*sub_population[location]->allele_frequencies[locus]-1)>1-HP_NOTHING){	//spot fixed loci
                if ((sub_population[location]->ancestral_state[locus]==0 and (2*sub_population[location]->allele_frequencies[locus]-1)<0) or
                    (sub_population[location]->ancestral_state[locus]==1 and (2*sub_population[location]->allele_frequencies[locus]-1)>0))
                {	//if they are in the ancestral state
                    tmp_individual = flip_single_locus(location, locus);		//introduce new allele
                    sub_population[location]->polymorphism[locus].birth = sub_population[location]->get_generation();
                    sub_population[location]->polymorphism[locus].fitness = sub_population[location]->population[tmp_individual].fitness-sub_population[location]->fitness_stat.mean;
                    sub_population[location]->polymorphism[locus].fitness_variance = sub_population[location]->fitness_stat.variance;
                    nmut++;
                }else{	//if locus is in derived state, flip coefficient of trait zero
                    sub_population[location]->trait[0].set_additive_coefficient(-sub_population[location]->trait[0].get_additive_coefficient(locus),locus,locus);
                    sub_population[location]->fixed_mutations.push_back(sub_population[location]->polymorphism[locus]);
                    sub_population[location]->fixed_mutations.back().sweep_time = sub_population[location]->get_generation() -sub_population[location]->fixed_mutations.back().birth;
                    tmp_individual=flip_single_locus(location, locus);
                    sub_population[location]->ancestral_state[locus]= (sub_population[location]->ancestral_state[locus]==0)?1:0;
                    sub_population[location]->polymorphism[locus].birth = sub_population[location]->get_generation();
                    sub_population[location]->polymorphism[locus].effect = (2*sub_population[location]->ancestral_state[locus]-1)*sub_population[location]->trait[0].get_additive_coefficient(locus);
                    sub_population[location]->polymorphism[locus].fitness = sub_population[location]->population[tmp_individual].fitness;
                    sub_population[location]->polymorphism[locus].fitness_variance = sub_population[location]->fitness_stat.variance;
                    nmut++;
                }
            }
        }
        sub_population[location]->number_of_mutations.push_back(nmut);
        sub_population[location]->calc_stat();

    } else if(HP_VERBOSE) cerr <<"haploid_highd::mutate(): mutation rate is zero."<<endl;

    if (HP_VERBOSE)	cerr <<"done."<<endl;;
    return 0;
}



unsigned int multi_population::flip_single_locus(int location, unsigned int clonenum, int locus) {
    // produce new genotype
    int new_clone = sub_population[location]->available_clones.back();
    sub_population[location]->available_clones.pop_back();
    sub_population[location]->allele_frequencies_up_to_date = false;

    //copy old genotype
    sub_population[location]->population[new_clone].genotype = sub_population[location]->population[clonenum].genotype;
    // new clone size == 1, old clone reduced by 1
    sub_population[location]->population[new_clone].clone_size = 1;
    sub_population[location]->population[clonenum].clone_size--;
    // flip the locus in new clone
    sub_population[location]->population[new_clone].genotype.flip(locus);
    // calculate traits and fitness
    vector<int> diff(1, locus);
    for (int t = 0; t < sub_population[location]->number_of_traits; t++){
        sub_population[location]->population[new_clone].trait[t] = sub_population[location]->population[clonenum].trait[t] + sub_population[location]->get_trait_difference(sub_population[location]->population[new_clone], sub_population[location]->population[clonenum], diff, t);
    }
    sub_population[location]->calc_individual_fitness_from_traits(sub_population[location]->population[new_clone]);
    sub_population[location]->check_individual_maximal_fitness(sub_population[location]->population[new_clone]);

    //update the last clones that is to be tracked
    sub_population[location]->last_clone = (new_clone<sub_population[location]->last_clone)?sub_population[location]->last_clone:new_clone;

    // add clone to current population
    if (sub_population[location]->population[clonenum].clone_size == 0)
        sub_population[location]->available_clones.push_back(clonenum);
    else
        sub_population[location]->number_of_clones++;

    if (track_genealogy == 2) {
        for (unsigned int genlocus=0; genlocus<genealogy.loci.size(); genlocus++) {


            add_migrating_clone_to_genealogy(
                    genlocus, location, location, new_clone, clonenum,
                    sub_population[location]->newGenerations[genlocus][clonenum].crossover[0],
                    sub_population[location]->newGenerations[genlocus][clonenum].crossover[1], 1, 1
                    );


            sub_population[location]->newGenerations[genlocus][clonenum].clone_size--;
        }
    }


    if (HP_VERBOSE >= 2) cerr <<"subpop::flip_single_spin(): mutated individual in clone "<<clonenum<<" at locus "<<locus<<endl;
    return new_clone;
}









unsigned int multi_population::flip_single_locus(int location, int locus) {
    if (sub_population[location]->available_clones.size() == 0)
        sub_population[location]->provide_at_least(1);
    return flip_single_locus(location, sub_population[location]->random_clone(), locus);
}



int multi_population::evolve(int location, int gen) {
    if (HP_VERBOSE) cerr<<"multi_population::evolve(int gen)...";
    if (sub_population[location]->population_size == 0)
        return 0;
    if (population_size() == 0)
    {

        cout << "Population went extinct!" << endl;
        return 0;
    }



    int err=0, g=0;
    sub_population[location]->allele_frequencies_up_to_date = false;
    // calculate an effective outcrossing rate to include the case of very rare crossover rates.
    // Since a recombination without crossovers is a waste of time, we scale down outcrossing probability
    // and scale up crossover rate so that at least one crossover is guaranteed to happen.
    if (sub_population[location]->recombination_model == CROSSOVERS)
        sub_population[location]->outcrossing_rate_effective = sub_population[location]->outcrossing_rate * (1 - exp(-sub_population[location]->number_of_loci * sub_population[location]->crossover_rate));
    else
        sub_population[location]->outcrossing_rate_effective = sub_population[location]->outcrossing_rate;

    // evolve cycle
    while((err == 0) && (g < gen)) {
        if (HP_VERBOSE) cerr << "generation " << generation << endl;
        sub_population[location]->random_sample.clear();			//discard the old random sample
        if(err==0) err=sub_population[location]->select_gametes();	//select a new set of gametes (partitioned into sex and asex)
        else if(HP_VERBOSE) cerr<<"Error in select_gametes()"<<endl;
        sort(sub_population[location]->available_clones.begin(), sub_population[location]->available_clones.end(), std::greater<int>()); //sort clones in order to use the first ones again and again
       // if(err==0) err=sub_population[location]->add_recombinants();	//do the recombination between pairs of sex gametes
       // else if(HP_VERBOSE) cerr<<"Error in recombine()"<<endl;
        if(err==0 && sub_population[location]->N() > 0) err=mutate(location);		//mutation step
        else if(HP_VERBOSE) cerr<<"Error in mutate()"<<endl;
        sub_population[location]->random_sample.clear();			//discard the old random sample
        g++;
        //generation++;

        //add the current generation to the genealogies and prune (i.e. remove parts that do not contribute the present.
        if (sub_population[location]->track_genealogy == 1) sub_population[location]->genealogy.add_generation(fitness_max, sub_population[location]->newGenerations);

    }
    if (HP_VERBOSE) {
        if(err==0) cerr<<"done."<<endl;
        else cerr<<"error "<<err<<"."<<endl;
    }
    return err;
}

