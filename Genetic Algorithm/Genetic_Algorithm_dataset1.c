#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ConL 5
#define NumR 10 // no of rules
#define NumP 50 // population size
#define Vars 5
#define size_of_data 32
#define generations 200
#define mutation_rate 0

/*=================
TYPE DEFINTIONS :
=================*/

// data type for the rules
typedef struct {
    int cond[ConL];
    int out;
} rule;

// Each individual represented by an array of rules and a fitness value
typedef struct {
    rule genes[NumR]; 
    int fitness;
} individual;

// data type to be used when reading data file
typedef struct {
    int variables[Vars];
    int class;
} data;

/*====================
FUNCTION PROTOTYPES :
====================*/

int  fitness_function (individual solution);
int  matches_cond (data data_example, int condition[ConL]);
int  matches_output (data data_example, int rule_output);
void load_data();
void genetic_algorithm();
int  randInRange(int min, int max);
void single_point_crossover(individual parent1, individual parent2);
void generate_inital_population();
void roulette_wheel_selection();
void tournament_selection();
void check_population_fitness(); 
void print_individual(); 
individual mutation();
void nextGeneration();
void save_data(FILE *fptr, int generation);

/*==================
GLOBAL VARIABLES  :
==================*/

data data_set[size_of_data]; // loaded actual data set
individual population[NumP]; // population of individual candidate solutions
individual offspring[NumP]; // new population from selection
individual offspring1; 
individual offspring2;

void main() {
    load_data();

    // creating file pointer to work with files
    FILE *data_file;

    // opening file in writing mode
    data_file = fopen("Results/data1_results.txt", "w");

    // exiting program 
    if (data_file == NULL) {
        printf("Error!");
        exit(1);
    }

    genetic_algorithm(data_file);

    fclose(data_file);
}

/*====================================================================
Genetic Algorithm Implementation
====================================================================*/
void genetic_algorithm(FILE *data_file) {
    // random seed
    srand(time(0)); 

    // Initialize a random population
    generate_inital_population();

    // Calculate fitness of the individuals in the population
    for( int j=0; j<NumP; j++ ) {
        population[j].fitness = fitness_function(population[j]);
    }

    // Loop through generations
    for (size_t i = 0; i < generations; i++) {
        //printf("Generation %d: ", i);

        // Select individuals for a new generation. 
        tournament_selection(); 
        //roulette_wheel_selection();
        
        // Create next generation of the population
        nextGeneration();
        
        // check fitness values
        if (i == generations-1) {
            check_population_fitness();
        }

        save_data(data_file, i);
    }
}

/*====================================================================
Randomly creates initial population
====================================================================*/
void generate_inital_population() {
    for( int i=0; i < NumP; i++) {
        for( int j=0; j < NumR; j++) {
            for( int z=0; z < ConL; z++ ) {
                population[i].genes[j].cond[z] = rand() % 2; 
            }
            population[i].genes[j].out = randInRange(0,1); 
        }
    }
}

/*====================================================================
Tournament Selection Implementation
====================================================================*/
void tournament_selection() {
    for( int i=0; i < NumP; i+=2 ) {
        int check = 0;
        int check2 = 0;
        int rand_parent1;
        int rand_parent2;
        individual parent1;
        while (check != 1) {
            rand_parent1 = rand() % NumP;
            rand_parent2 = rand() % NumP;
            if (rand_parent1 != rand_parent2) { 
                check = 1; 
            }
        }
        if ( population[rand_parent1].fitness >= population[rand_parent2].fitness) { 
            parent1 = population[rand_parent1];
        }
        else {
            parent1 = population[rand_parent2];
        }
        
        while (check2 != 1) {
            rand_parent1 = rand() % NumP;
            rand_parent2 = rand() % NumP;
            if (rand_parent1 != rand_parent2) { 
                check2 = 1; 
            }
        }
        if ( population[rand_parent1].fitness >= population[rand_parent2].fitness) {
            single_point_crossover(parent1, population[rand_parent1]);
        }
        else {
            single_point_crossover(parent1, population[rand_parent2]);
        }
        offspring[i] = offspring1;
        offspring[i+1] = offspring2;
    }
}

/*====================================================================
Roulette wheel selection implementation
====================================================================*/
void roulette_wheel_selection() {
    for( int i=0; i < NumP; i+=2 ) {
        individual parent1;
        individual parent2;
        // select first parent
        int rand_parent1 = rand() % NumP;
        int rand_parent2 = rand() % NumP;
        if ( population[rand_parent1].fitness >= population[rand_parent2].fitness) {
            parent1 = population[rand_parent1];
        }
        else {
            parent1 = population[rand_parent2];
        }
        // select second parent
        rand_parent1 = rand() % NumP;
        rand_parent2 = rand() % NumP;
        if ( population[rand_parent1].fitness >= population[rand_parent2].fitness) {
            parent2 = population[rand_parent1];
        }
        else {
            parent2 = population[rand_parent2];
        }
        single_point_crossover(parent1, parent2);
        offspring[i] = offspring1;
        offspring[i+1] = offspring2;
    }
}

/*====================================================================
Produce a new generation of individuals using Offspring population
====================================================================*/
void nextGeneration() {
    for (int x = 0; x < NumP; x++) {
        for (int y = 0; y < NumR; y++) {
            for( int z=0; z < ConL; z++ ) {
                population[x].genes[y].cond[z] = offspring[x].genes[y].cond[z];
            }
            population[x].genes[y].out = offspring[x].genes[y].out;
            population[x].fitness = fitness_function(population[x]);
        }
    }
}

/*====================================================================
Fitness function implementation
====================================================================*/
int fitness_function(individual solution) {

    int fitness=0;
    int k=0; // index variable to move along the string of genes as the rule parts are filled
    rule rulebase[NumR]; // declaring an array of type rule
    data data_example;

    // filling the conditions of each rule from the genes of the individual passed in
    for(int i=0; i <= NumR-1; i++) {
        // this for loop adds condition to rule
        for(int j=0; j <= ConL-1; j++) {
            rulebase[i].cond[j] = solution.genes[i].cond[j]; 
        }
        // add output to if rule
        rulebase[i].out = solution.genes[i].out; 
    }
    
    for(int i=0; i <= size_of_data-1; i++ ) { // got through the population
        data_example = data_set[i];
        for(int j=0; j <= NumR-1; j++) { // go through the rules of an individual
            if( matches_cond(data_example, rulebase[j].cond) == 1) {
                if( matches_output(data_example, rulebase[j].out) == 1) {
                    fitness++;
                }
                break; // get the next data item after a match
            }
        }
    }
    return fitness;
}

/*====================================================================
Return true if rule conditions is same as the data_example rule conditions
====================================================================*/
int matches_cond (data data_example, int condition[ConL]) {
    for (int i = 0; i < Vars; i++) {
        if (condition[i] != 2) {
            if (data_example.variables[i] != condition[i]) {
                return 0; // false
            }
        }
    }
    return 1; // true
}

/*====================================================================
Return true if rule output is same as the data_example output
====================================================================*/
int matches_output (data data_example, int rule_output) {
    if (data_example.class != rule_output) {
        return 0; // false
    }
    return 1; // true
}

/*====================================================================
Load data to data_set (Code generated from another program)
--> Program used to generate this code is in supporting files folder
====================================================================*/
void load_data() {

   data temp;
   temp = (data){.variables = {0, 0, 0, 0, 0}, .class = 0};
   data_set[0] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1}, .class = 0};
   data_set[1] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0}, .class = 0};
   data_set[2] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1}, .class = 1};
   data_set[3] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0}, .class = 0};
   data_set[4] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1}, .class = 1};
   data_set[5] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0}, .class = 1};
   data_set[6] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1}, .class = 0};
   data_set[7] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0}, .class = 0};
   data_set[8] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1}, .class = 1};
   data_set[9] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0}, .class = 1};
   data_set[10] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1}, .class = 0};
   data_set[11] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0}, .class = 1};
   data_set[12] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1}, .class = 0};
   data_set[13] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0}, .class = 0};
   data_set[14] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1}, .class = 1};
   data_set[15] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0}, .class = 0};
   data_set[16] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1}, .class = 1};
   data_set[17] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0}, .class = 1};
   data_set[18] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1}, .class = 0};
   data_set[19] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0}, .class = 1};
   data_set[20] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1}, .class = 0};
   data_set[21] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0}, .class = 0};
   data_set[22] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1}, .class = 1};
   data_set[23] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0}, .class = 1};
   data_set[24] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1}, .class = 0};
   data_set[25] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0}, .class = 0};
   data_set[26] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1}, .class = 1};
   data_set[27] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0}, .class = 0};
   data_set[28] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1}, .class = 1};
   data_set[29] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0}, .class = 1};
   data_set[30] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1}, .class = 0};
   data_set[31] = temp;

}

/*====================================================================
Single point crossover implementation
====================================================================*/
void single_point_crossover(individual parent1, individual parent2) {
    
    for (size_t i = 0; i < 100; i++)
    {
        randInRange(1, NumR-1);
    }
    int crossover_point = randInRange(1, NumR-1);
    individual parent = parent1;
    for (int i = 0; i < NumR; i++) {
        if (i < crossover_point) {
            offspring1.genes[i] = parent1.genes[i];
            offspring2.genes[i] = parent2.genes[i];
        }
        else {
            offspring1.genes[i] = parent2.genes[i];
            offspring2.genes[i] = parent1.genes[i];
        }
    }
    // Proceed with mutation after crossover
    int mutation_prob = randInRange(1, 100);
    if (mutation_prob <= mutation_rate) {
        int offspringNo = randInRange(1,2);
        if (offspringNo == 1) {
           offspring1 =  mutation(offspring1);
        }
        else {
            offspring2 = mutation(offspring2);
        }
    }
}

/*====================================================================
Print the given individual
====================================================================*/
void print_individual(individual ind) {
    for (int i = 0; i < NumR; i++) {
        for (int j = 0; j < ConL; j++) {
            printf("%d", ind.genes[i].cond[j]);
        }
        printf("-%d ", ind.genes[i].out);
    }
    printf("\n");
}

/*====================================================================
Return an integer between the two given values
====================================================================*/
int randInRange(int min, int max)
{
  return min + (int) (rand() / (double) (RAND_MAX + 1) * (max - min + 1));
}

/*====================================================================
Mutation function which mutates random bits to wildcards
====================================================================*/
individual mutation(individual offspring) {
    int mutation_pos = randInRange(0, ConL-1);
    int mutation_rule = randInRange(0, NumR-1);
    int cond_out = randInRange(0, 1);
    if (cond_out == 1) {
        offspring.genes[mutation_rule].out = 2;
    }
    else {
        offspring.genes[mutation_rule].cond[mutation_pos] = 2;
    }
    return offspring;
}

/*====================================================================
Print fitness of individuals in current population
====================================================================*/
void check_population_fitness() {
    int mean_fitness;
    int total = 0;
    int max = 0;
    for (int i = 0; i < NumP; i++)
    {
        if (population[i].fitness > max) {
            max = population[i].fitness;
        }
        total += population[i].fitness;
    }
    mean_fitness = total / NumP ;
    int fitness_percentage = ((mean_fitness * 100) / size_of_data);
    printf("Mean: %d , Max: %d, Percentage: %d%%\n", mean_fitness, max, fitness_percentage);
}

/*====================================================================
Save mean fitness and fitness of fittest individual to a file
====================================================================*/
void save_data(FILE *data_file, int generation) {
    int fittest = 0;
    int total = 0;
    individual solution;
    for (int i = 0; i < NumP; i++)
    {
        if (population[i].fitness > fittest) {
            fittest = population[i].fitness;
            solution = population[i];
        }
        total += population[i].fitness;
    }

    if (generation+1 == generations) {
        print_individual(solution);
        printf("fitness    -->     %d\n", solution.fitness);
    }

    int mean_fitness = total / NumP ;
    int fitness_percentage = ((mean_fitness * 100) / size_of_data);
    fprintf(data_file, "%d,%d,%d\n", generation + 1, fittest, mean_fitness);
}

