#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>
using namespace std::chrono;
using namespace std;

void CreateArray(int* newArray, int sizeOfNewArray) {
    int i, j, t;
    for (i = 0; i < sizeOfNewArray; i++)
        newArray[i] = i;
    for (i = 0; i < sizeOfNewArray; i++) {
        j = rand() % sizeOfNewArray;
        t = newArray[i];
        newArray[i] = newArray[j];
        newArray[j] = t;
    }
}

void PrintArray(int printedArray[], int sizeOfPrintedArray) {
    for (int i = 0; i < sizeOfPrintedArray; i++)
        cout << printedArray[i] << " ";
    cout << endl;
}

void OddEvenSort(int* sortedArray, int sizeOfSortedArray)
{
    int stepInSort, changeOfNumberPlaces, i;
#pragma omp parallel default(none) shared(sortedArray, sizeOfSortedArray) private(i, changeOfNumberPlaces, stepInSort) 
    for (stepInSort = 0; stepInSort < sizeOfSortedArray; stepInSort++)
    {
        if (stepInSort % 2 == 0)
#pragma omp for   
            for (i = 1; i < sizeOfSortedArray; i += 2)
            {
                if (sortedArray[i - 1] > sortedArray[i])
                {
                    changeOfNumberPlaces = sortedArray[i - 1];
                    sortedArray[i - 1] = sortedArray[i];
                    sortedArray[i] = changeOfNumberPlaces;
                }
            }
        else
#pragma omp for
            for (i = 1; i < sizeOfSortedArray - 1; i += 2)
            {
                if (sortedArray[i] > sortedArray[i + 1])
                {
                    changeOfNumberPlaces = sortedArray[i + 1];
                    sortedArray[i + 1] = sortedArray[i];
                    sortedArray[i] = changeOfNumberPlaces;
                }
            }
    }
}

int main() {
    int size = 100000;
    int* array = new int[size];
    CreateArray(array, size);

    //printArray(array, size);
    omp_set_num_threads(32);

    auto start = high_resolution_clock::now();
    OddEvenSort(array, size);
    auto stop = high_resolution_clock::now();

    //cout << "Sorted array: \n";
    //PrintArray(array, size);

    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "\n\n Sorted time:" << duration.count();
    return 0;
}