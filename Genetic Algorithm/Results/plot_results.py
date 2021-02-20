
import pylab as plt
import csv
import numpy as np

# change this variable to load results from different data sets
file_name = "data3_results.txt"

# Read data in from file
with open(file_name) as f: 
    reader = csv.reader(f)
    next(reader)

    generation = []
    max_fitness = []
    mean_fitness = []
    line_count = 0

    for row in reader:
        generation.append(int(row[0]))
        max_fitness.append(int(row[1]))
        mean_fitness.append(int(row[2]))
        line_count += 1

# plot data
plt.plot(generation, mean_fitness, 'b-', label='mean fitness')
plt.plot(generation, max_fitness, 'r-', label='max fitness')
plt.xlabel("Generations")
plt.ylabel("Fitness")
plt.legend(loc = 'lower right')
plt.title("GA 3 [Rules: 20, Pop: 50, Gen: 250, Mutation: 0.02/Gene]")
plt.show()

