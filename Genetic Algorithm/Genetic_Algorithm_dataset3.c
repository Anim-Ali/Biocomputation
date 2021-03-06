#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ConL 6
#define NumR 10 // no of rules
#define NumP 10 // population size
#define Vars 6
#define size_of_data 2000
#define generations 100
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
    data_file = fopen("Results/data3_results.txt", "w");

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
Produces a new generation of individuals using Offspring population
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
   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[0] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[2] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[3] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[4] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[5] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[6] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[7] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[8] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[9] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[10] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[11] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[12] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[13] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[14] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[15] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[16] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[17] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[18] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[19] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[20] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[21] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[22] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[23] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[24] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[25] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[26] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[27] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[28] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[29] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[30] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[31] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[32] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[33] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[34] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[35] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[36] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[37] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[38] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[39] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[40] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[41] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[42] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[43] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[44] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[45] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[46] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[47] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[48] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[49] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[50] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[51] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[52] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[53] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[54] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[55] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[56] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[57] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[58] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[59] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[60] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[61] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[62] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[63] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[64] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[65] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[66] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[67] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[68] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[69] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[70] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[71] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[72] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[73] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[74] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[75] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[76] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[77] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[78] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[79] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[80] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[81] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[82] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[83] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[84] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[85] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[86] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[87] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[88] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[89] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[90] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[91] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[92] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[93] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[94] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[95] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[96] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[97] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[98] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[99] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[100] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[101] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[102] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[103] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[104] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[105] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[106] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[107] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[108] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[109] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[110] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[111] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[112] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[113] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[114] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[115] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[116] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[117] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[118] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[119] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[120] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[121] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[122] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[123] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[124] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[125] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[126] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[127] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[128] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[129] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[130] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[131] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[132] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[133] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[134] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[135] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[136] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[137] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[138] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[139] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[140] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[141] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[142] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[143] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[144] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[145] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[146] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[147] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[148] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[149] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[150] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[151] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[152] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[153] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[154] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[155] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[156] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[157] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[158] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[159] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[160] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[161] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[162] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[163] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[164] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[165] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[166] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[167] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[168] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[169] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[170] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[171] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[172] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[173] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[174] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[175] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[176] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[177] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[178] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[179] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[180] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[181] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[182] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[183] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[184] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[185] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[186] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[187] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[188] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[189] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[190] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[191] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[192] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[193] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[194] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[195] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[196] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[197] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[198] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[199] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[200] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[201] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[202] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[203] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[204] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[205] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[206] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[207] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[208] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[209] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[210] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[211] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[212] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[213] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[214] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[215] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[216] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[217] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[218] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[219] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[220] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[221] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[222] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[223] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[224] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[225] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[226] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[227] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[228] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[229] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[230] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[231] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[232] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[233] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[234] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[235] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[236] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[237] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[238] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[239] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[240] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[241] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[242] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[243] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[244] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[245] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[246] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[247] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[248] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[249] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[250] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[251] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[252] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[253] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[254] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[255] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[256] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[257] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[258] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[259] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[260] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[261] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[262] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[263] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[264] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[265] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[266] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[267] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[268] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[269] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[270] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[271] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[272] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[273] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[274] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[275] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[276] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[277] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[278] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[279] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[280] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[281] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[282] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[283] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[284] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[285] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[286] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[287] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[288] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[289] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[290] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[291] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[292] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[293] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[294] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[295] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[296] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[297] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[298] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[299] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[300] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[301] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[302] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[303] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[304] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[305] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[306] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[307] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[308] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[309] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[310] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[311] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[312] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[313] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[314] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[315] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[316] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[317] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[318] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[319] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[320] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[321] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[322] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[323] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[324] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[325] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[326] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[327] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[328] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[329] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[330] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[331] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[332] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[333] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[334] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[335] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[336] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[337] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[338] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[339] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[340] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[341] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[342] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[343] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[344] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[345] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[346] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[347] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[348] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[349] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[350] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[351] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[352] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[353] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[354] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[355] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[356] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[357] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[358] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[359] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[360] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[361] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[362] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[363] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[364] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[365] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[366] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[367] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[368] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[369] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[370] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[371] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[372] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[373] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[374] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[375] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[376] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[377] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[378] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[379] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[380] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[381] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[382] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[383] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[384] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[385] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[386] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[387] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[388] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[389] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[390] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[391] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[392] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[393] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[394] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[395] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[396] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[397] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[398] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[399] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[400] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[401] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[402] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[403] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[404] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[405] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[406] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[407] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[408] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[409] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[410] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[411] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[412] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[413] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[414] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[415] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[416] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[417] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[418] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[419] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[420] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[421] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[422] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[423] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[424] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[425] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[426] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[427] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[428] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[429] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[430] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[431] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[432] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[433] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[434] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[435] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[436] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[437] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[438] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[439] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[440] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[441] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[442] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[443] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[444] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[445] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[446] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[447] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[448] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[449] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[450] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[451] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[452] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[453] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[454] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[455] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[456] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[457] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[458] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[459] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[460] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[461] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[462] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[463] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[464] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[465] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[466] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[467] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[468] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[469] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[470] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[471] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[472] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[473] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[474] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[475] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[476] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[477] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[478] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[479] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[480] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[481] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[482] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[483] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[484] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[485] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[486] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[487] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[488] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[489] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[490] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[491] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[492] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[493] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[494] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[495] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[496] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[497] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[498] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[499] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[500] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[501] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[502] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[503] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[504] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[505] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[506] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[507] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[508] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[509] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[510] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[511] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[512] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[513] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[514] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[515] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[516] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[517] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[518] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[519] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[520] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[521] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[522] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[523] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[524] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[525] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[526] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[527] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[528] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[529] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[530] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[531] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[532] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[533] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[534] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[535] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[536] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[537] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[538] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[539] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[540] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[541] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[542] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[543] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[544] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[545] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[546] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[547] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[548] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[549] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[550] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[551] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[552] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[553] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[554] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[555] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[556] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[557] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[558] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[559] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[560] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[561] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[562] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[563] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[564] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[565] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[566] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[567] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[568] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[569] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[570] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[571] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[572] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[573] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[574] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[575] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[576] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[577] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[578] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[579] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[580] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[581] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[582] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[583] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[584] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[585] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[586] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[587] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[588] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[589] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[590] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[591] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[592] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[593] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[594] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[595] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[596] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[597] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[598] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[599] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[600] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[601] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[602] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[603] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[604] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[605] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[606] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[607] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[608] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[609] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[610] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[611] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[612] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[613] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[614] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[615] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[616] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[617] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[618] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[619] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[620] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[621] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[622] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[623] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[624] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[625] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[626] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[627] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[628] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[629] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[630] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[631] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[632] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[633] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[634] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[635] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[636] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[637] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[638] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[639] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[640] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[641] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[642] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[643] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[644] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[645] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[646] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[647] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[648] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[649] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[650] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[651] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[652] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[653] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[654] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[655] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[656] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[657] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[658] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[659] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[660] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[661] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[662] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[663] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[664] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[665] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[666] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[667] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[668] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[669] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[670] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[671] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[672] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[673] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[674] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[675] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[676] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[677] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[678] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[679] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[680] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[681] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[682] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[683] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[684] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[685] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[686] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[687] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[688] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[689] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[690] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[691] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[692] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[693] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[694] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[695] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[696] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[697] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[698] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[699] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[700] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[701] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[702] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[703] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[704] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[705] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[706] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[707] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[708] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[709] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[710] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[711] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[712] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[713] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[714] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[715] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[716] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[717] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[718] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[719] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[720] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[721] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[722] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[723] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[724] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[725] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[726] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[727] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[728] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[729] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[730] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[731] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[732] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[733] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[734] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[735] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[736] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[737] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[738] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[739] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[740] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[741] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[742] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[743] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[744] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[745] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[746] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[747] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[748] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[749] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[750] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[751] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[752] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[753] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[754] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[755] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[756] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[757] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[758] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[759] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[760] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[761] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[762] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[763] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[764] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[765] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[766] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[767] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[768] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[769] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[770] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[771] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[772] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[773] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[774] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[775] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[776] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[777] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[778] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[779] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[780] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[781] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[782] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[783] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[784] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[785] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[786] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[787] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[788] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[789] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[790] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[791] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[792] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[793] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[794] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[795] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[796] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[797] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[798] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[799] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[800] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[801] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[802] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[803] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[804] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[805] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[806] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[807] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[808] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[809] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[810] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[811] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[812] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[813] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[814] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[815] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[816] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[817] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[818] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[819] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[820] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[821] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[822] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[823] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[824] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[825] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[826] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[827] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[828] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[829] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[830] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[831] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[832] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[833] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[834] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[835] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[836] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[837] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[838] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[839] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[840] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[841] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[842] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[843] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[844] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[845] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[846] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[847] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[848] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[849] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[850] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[851] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[852] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[853] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[854] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[855] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[856] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[857] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[858] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[859] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[860] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[861] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[862] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[863] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[864] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[865] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[866] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[867] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[868] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[869] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[870] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[871] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[872] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[873] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[874] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[875] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[876] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[877] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[878] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[879] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[880] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[881] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[882] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[883] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[884] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[885] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[886] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[887] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[888] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[889] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[890] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[891] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[892] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[893] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[894] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[895] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[896] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[897] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[898] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[899] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[900] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[901] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[902] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[903] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[904] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[905] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[906] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[907] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[908] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[909] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[910] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[911] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[912] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[913] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[914] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[915] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[916] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[917] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[918] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[919] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[920] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[921] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[922] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[923] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[924] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[925] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[926] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[927] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[928] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[929] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[930] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[931] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[932] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[933] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[934] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[935] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[936] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[937] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[938] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[939] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[940] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[941] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[942] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[943] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[944] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[945] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[946] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[947] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[948] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[949] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[950] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[951] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[952] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[953] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[954] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[955] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[956] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[957] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[958] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[959] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[960] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[961] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[962] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[963] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[964] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[965] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[966] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[967] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[968] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[969] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[970] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[971] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[972] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[973] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[974] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[975] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[976] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[977] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[978] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[979] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[980] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[981] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[982] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[983] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[984] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[985] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[986] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[987] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[988] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[989] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[990] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[991] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[992] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[993] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[994] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[995] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[996] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[997] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[998] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[999] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1000] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1001] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1002] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1003] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1004] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1005] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1006] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1007] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1008] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1009] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1010] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1011] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1012] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1013] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1014] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1015] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1016] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1017] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1018] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1019] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1020] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1021] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1022] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1023] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1024] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1025] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1026] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1027] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1028] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1029] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1030] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1031] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1032] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1033] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1034] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1035] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1036] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1037] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1038] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1039] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1040] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1041] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1042] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1043] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1044] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1045] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1046] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1047] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1048] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1049] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1050] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1051] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1052] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1053] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1054] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1055] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1056] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1057] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1058] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1059] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1060] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1061] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1062] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1063] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1064] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1065] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1066] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1067] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1068] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1069] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1070] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1071] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1072] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1073] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1074] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1075] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1076] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1077] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1078] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1079] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1080] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1081] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1082] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1083] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1084] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1085] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1086] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1087] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1088] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1089] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1090] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1091] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1092] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1093] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1094] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1095] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1096] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1097] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1098] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1099] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1100] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1101] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1102] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1103] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1104] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1105] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1106] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1107] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1108] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1109] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1110] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1111] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1112] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1113] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1114] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1115] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1116] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1117] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1118] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1119] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1120] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1121] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1122] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1123] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1124] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1125] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1126] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1127] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1128] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1129] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1130] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1131] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1132] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1133] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1134] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1135] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1136] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1137] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1138] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1139] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1140] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1141] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1142] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1143] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1144] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1145] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1146] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1147] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1148] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1149] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1150] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1151] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1152] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1153] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1154] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1155] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1156] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1157] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1158] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1159] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1160] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1161] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1162] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1163] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1164] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1165] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1166] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1167] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1168] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1169] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1170] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1171] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1172] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1173] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1174] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1175] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1176] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1177] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1178] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1179] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1180] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1181] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1182] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1183] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1184] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1185] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1186] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1187] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1188] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1189] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1190] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1191] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1192] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1193] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1194] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1195] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1196] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1197] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1198] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1199] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1200] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1201] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1202] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1203] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1204] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1205] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1206] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1207] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1208] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1209] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1210] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1211] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1212] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1213] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1214] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1215] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1216] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1217] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1218] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1219] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1220] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1221] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1222] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1223] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1224] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1225] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1226] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1227] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1228] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1229] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1230] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1231] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1232] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1233] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1234] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1235] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1236] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1237] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1238] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1239] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1240] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1241] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1242] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1243] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1244] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1245] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1246] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1247] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1248] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1249] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1250] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1251] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1252] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1253] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1254] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1255] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1256] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1257] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1258] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1259] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1260] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1261] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1262] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1263] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1264] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1265] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1266] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1267] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1268] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1269] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1270] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1271] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1272] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1273] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1274] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1275] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1276] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1277] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1278] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1279] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1280] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1281] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1282] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1283] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1284] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1285] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1286] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1287] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1288] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1289] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1290] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1291] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1292] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1293] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1294] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1295] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1296] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1297] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1298] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1299] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1300] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1301] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1302] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1303] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1304] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1305] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1306] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1307] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1308] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1309] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1310] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1311] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1312] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1313] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1314] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1315] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1316] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1317] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1318] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1319] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1320] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1321] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1322] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1323] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1324] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1325] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1326] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1327] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1328] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1329] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1330] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1331] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1332] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1333] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1334] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1335] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1336] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1337] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1338] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1339] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1340] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1341] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1342] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1343] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1344] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1345] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1346] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1347] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1348] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1349] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1350] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1351] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1352] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1353] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1354] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1355] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1356] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1357] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1358] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1359] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1360] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1361] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1362] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1363] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1364] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1365] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1366] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1367] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1368] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1369] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1370] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1371] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1372] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1373] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1374] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1375] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1376] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1377] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1378] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1379] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1380] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1381] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1382] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1383] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1384] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1385] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1386] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1387] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1388] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1389] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1390] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1391] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1392] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1393] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1394] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1395] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1396] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1397] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1398] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1399] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1400] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1401] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1402] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1403] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1404] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1405] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1406] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1407] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1408] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1409] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1410] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1411] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1412] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1413] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1414] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1415] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1416] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1417] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1418] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1419] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1420] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1421] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1422] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1423] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1424] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1425] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1426] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1427] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1428] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1429] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1430] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1431] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1432] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1433] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1434] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1435] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1436] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1437] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1438] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1439] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1440] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1441] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1442] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1443] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1444] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1445] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1446] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1447] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1448] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1449] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1450] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1451] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1452] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1453] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1454] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1455] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1456] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1457] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1458] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1459] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1460] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1461] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1462] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1463] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1464] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1465] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1466] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1467] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1468] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1469] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1470] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1471] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1472] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1473] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1474] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1475] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1476] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1477] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1478] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1479] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1480] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1481] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1482] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1483] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1484] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1485] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1486] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1487] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1488] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1489] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1490] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1491] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1492] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1493] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1494] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1495] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1496] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1497] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1498] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1499] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1500] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1501] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1502] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1503] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1504] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1505] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1506] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1507] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1508] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1509] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1510] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1511] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1512] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1513] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1514] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1515] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1516] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1517] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1518] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1519] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1520] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1521] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1522] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1523] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1524] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1525] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1526] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1527] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1528] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1529] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1530] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1531] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1532] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1533] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1534] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1535] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1536] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1537] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1538] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1539] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1540] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1541] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1542] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1543] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1544] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1545] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1546] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1547] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1548] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1549] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1550] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1551] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1552] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1553] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1554] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1555] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1556] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1557] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1558] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1559] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1560] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 1}, .class = 0};
   data_set[1561] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1562] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1563] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1564] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1565] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1566] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1567] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1568] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1569] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1570] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1571] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1572] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1573] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1574] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1575] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1576] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1577] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1578] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1579] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1580] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1581] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1582] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1583] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1584] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1585] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1586] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1587] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1588] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1589] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1590] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1591] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1592] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1593] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1594] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1595] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1596] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1597] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1598] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1599] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1600] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1601] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1602] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1603] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1604] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1605] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1606] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1607] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1608] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1609] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1610] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1611] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1612] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1613] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1614] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1615] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1616] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1617] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1618] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1619] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1620] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1621] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1622] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1623] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1624] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1625] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1626] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1627] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1628] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1629] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1630] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1631] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1632] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1633] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1634] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1635] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1636] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1637] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1638] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1639] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1640] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1641] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1642] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1643] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1644] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1645] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1646] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1647] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1648] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1649] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1650] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1651] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1652] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1653] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1654] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1655] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1656] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1657] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1658] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1659] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1660] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1661] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1662] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1663] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1664] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1665] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1666] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1667] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1668] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1669] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1670] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1671] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1672] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1673] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1674] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1675] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1676] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1677] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1678] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1679] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1680] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1681] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1682] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1683] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1684] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1685] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1686] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1687] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1688] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1689] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1690] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1691] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1692] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1693] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1694] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1695] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1696] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1697] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1698] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1699] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1700] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1701] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1702] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1703] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1704] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1705] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1706] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1707] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1708] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1709] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1710] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1711] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1712] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1713] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1714] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1715] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1716] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1717] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1718] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1719] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1720] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1721] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1722] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1723] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1724] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1725] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1726] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1727] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1728] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1729] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1730] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1731] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1732] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1733] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1734] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1735] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1736] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1737] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1738] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1739] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1740] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1741] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1742] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1743] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1744] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1745] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1746] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1747] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1748] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1749] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1750] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1751] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1752] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1753] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1754] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1755] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1756] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1757] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1758] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1759] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1760] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1761] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1762] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1763] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1764] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1765] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1766] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1767] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1768] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1769] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1770] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1771] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1772] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1773] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1774] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1775] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1776] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1777] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1778] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1779] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1780] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1781] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1782] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1783] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1784] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1785] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1786] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1787] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1788] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1789] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1790] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1791] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1792] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1793] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1794] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1795] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1796] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1797] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1798] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1799] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1800] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1801] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1802] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1803] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1804] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1805] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1806] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1807] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1808] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1809] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1810] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1811] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1812] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 1}, .class = 1};
   data_set[1813] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1814] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 1}, .class = 0};
   data_set[1815] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1816] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1817] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1818] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1819] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1820] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1821] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1822] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1823] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1824] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1825] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1826] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1827] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1828] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1829] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1830] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1831] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1832] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1833] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1834] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1835] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1836] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1837] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1838] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1839] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1840] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1841] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1842] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1843] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1844] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1845] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1846] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1847] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1848] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1849] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1850] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1851] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1852] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1853] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1854] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1855] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1856] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1857] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1858] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1859] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 1}, .class = 0};
   data_set[1860] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1861] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1862] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1863] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1864] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1865] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1866] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1867] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1868] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1869] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 1}, .class = 0};
   data_set[1870] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1871] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1872] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1873] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1874] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 0, 0}, .class = 1};
   data_set[1875] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1876] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1877] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1878] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1879] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1880] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1881] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1882] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1883] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 1}, .class = 0};
   data_set[1884] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1885] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1886] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1887] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1888] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1889] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 1}, .class = 1};
   data_set[1890] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1891] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1892] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1893] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1894] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1895] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1896] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1897] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 0}, .class = 1};
   data_set[1898] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 0, 0}, .class = 1};
   data_set[1899] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 0}, .class = 0};
   data_set[1900] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1901] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1902] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1903] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1904] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 1}, .class = 1};
   data_set[1905] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1906] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 0}, .class = 0};
   data_set[1907] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1908] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1909] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1910] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1911] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 1}, .class = 0};
   data_set[1912] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1913] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1914] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1915] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1916] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1917] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1918] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1919] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1920] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1921] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1922] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1923] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1924] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1925] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1926] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1927] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1928] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1929] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1930] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1931] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 0}, .class = 1};
   data_set[1932] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1933] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1934] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 1, 0}, .class = 0};
   data_set[1935] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1936] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1937] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1938] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1939] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1940] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1941] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1942] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 0}, .class = 0};
   data_set[1943] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1944] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 0, 0}, .class = 1};
   data_set[1945] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1946] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1947] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1948] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1949] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 1}, .class = 0};
   data_set[1950] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1951] = temp;

   temp = (data){.variables = {1, 0, 1, 0, 1, 1}, .class = 1};
   data_set[1952] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 1}, .class = 0};
   data_set[1953] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 0}, .class = 0};
   data_set[1954] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1955] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1956] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1957] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 1}, .class = 1};
   data_set[1958] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1959] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1960] = temp;

   temp = (data){.variables = {1, 0, 0, 1, 0, 0}, .class = 0};
   data_set[1961] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1962] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1963] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1964] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1965] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1966] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1967] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 0, 0}, .class = 0};
   data_set[1968] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1969] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1970] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1971] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1972] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 1}, .class = 1};
   data_set[1973] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 1, 1}, .class = 0};
   data_set[1974] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 0}, .class = 0};
   data_set[1975] = temp;

   temp = (data){.variables = {0, 1, 0, 0, 0, 0}, .class = 1};
   data_set[1976] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 1}, .class = 1};
   data_set[1977] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 1, 0}, .class = 0};
   data_set[1978] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1979] = temp;

   temp = (data){.variables = {1, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1980] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 1, 1}, .class = 1};
   data_set[1981] = temp;

   temp = (data){.variables = {1, 1, 0, 0, 0, 1}, .class = 0};
   data_set[1982] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 0}, .class = 1};
   data_set[1983] = temp;

   temp = (data){.variables = {1, 0, 1, 1, 1, 0}, .class = 1};
   data_set[1984] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 1, 1}, .class = 1};
   data_set[1985] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 1, 1}, .class = 0};
   data_set[1986] = temp;

   temp = (data){.variables = {0, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1987] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 1, 0}, .class = 0};
   data_set[1988] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 0, 1}, .class = 0};
   data_set[1989] = temp;

   temp = (data){.variables = {1, 1, 0, 1, 1, 1}, .class = 1};
   data_set[1990] = temp;

   temp = (data){.variables = {0, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1991] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 0, 0}, .class = 1};
   data_set[1992] = temp;

   temp = (data){.variables = {0, 0, 1, 0, 0, 0}, .class = 0};
   data_set[1993] = temp;

   temp = (data){.variables = {1, 1, 1, 0, 1, 0}, .class = 1};
   data_set[1994] = temp;

   temp = (data){.variables = {0, 0, 0, 1, 0, 1}, .class = 1};
   data_set[1995] = temp;

   temp = (data){.variables = {0, 0, 1, 1, 0, 1}, .class = 1};
   data_set[1996] = temp;

   temp = (data){.variables = {0, 1, 1, 1, 1, 0}, .class = 1};
   data_set[1997] = temp;

   temp = (data){.variables = {1, 0, 0, 0, 0, 0}, .class = 0};
   data_set[1998] = temp;

   temp = (data){.variables = {0, 1, 1, 0, 0, 0}, .class = 1};
   data_set[1999] = temp;

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

