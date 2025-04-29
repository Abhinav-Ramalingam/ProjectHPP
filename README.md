# HPP Project

This folder contains the code for a High-Performance Programming (HPP) project, which involves generating data, sorting it, and checking for deviations. Below are the instructions to compile, run, and check the results.

## Prerequisites

- GCC (GNU Compiler Collection)
- Make
- OpenMP and pthread libraries

## Steps to Run the Code

### 1. Generate Data

First, compile the data generation program (`gen_data.c`):

```bash
gcc -O3 -o gd gen_data.c
```

Next, run the program with the desired input size (`100000000` in this example):

```bash
./gd 100000000
```

This will generate a file with the numbers that will later be sorted.

### 2. Compile the Quicksort Program

Once the data is generated, compile the quicksort program (`qsp.c`):

```bash
make qsp
```

This will compile the program with optimizations (`-O3`, `-ffast-math`, etc.) and enable OpenMP and pthread support.

### 3. Run the Quicksort Program

After compilation, run the program with the generated data (`numbers`), sorted output file (`sorted`), and other parameters as required. For example:

```bash
./qsp 100000000 numbers sorted 32 a
```

This command sorts the generated data and outputs the results. Here are the times for different operations:

- **Input time**: Time taken to read the data.
- **Sorting time**: Time taken to sort the data.
- **Output time**: Time taken to write the sorted data.

### 4. Compile the Data Checking Program

After sorting, compile the data checking program (`check_data.c`):

```bash
gcc -O3 -o cd check_data.c
```

### 5. Check the Sorted Data

Finally, run the program to check the sorted data:

```bash
./cd 100000000 sorted
```

This will check the sorted data and output the total deviations. A result of `0` deviations indicates that the data is correctly sorted.