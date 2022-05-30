#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <iostream>
#include <chrono>

using namespace std::chrono;
using namespace std;

void CreateArray(int* newArray, int sizeOfNewArray) {
    int i, j, t;
    for (i = 0; i < sizeOfNewArray; i++)
        newArray[i] = i;
    for (i = 0; i < sizeOfNewArray; i++)
    {
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

void PartialOddEvenSort(int* sortedArray, int start, int end)
{
    int n = end - start + 1;
    if (n == 1) return;

    int stepInSort, changeOfNumberPlaces, i, firstOdd;

    if (start % 2 == 0) firstOdd = start + 1;
    else firstOdd = start;

    for (stepInSort = 0; stepInSort < n; stepInSort++)
    {
        if (stepInSort % 2 == 0)
        {

            for (i = firstOdd; i < n; i += 2)
            {
                if (sortedArray[i - 1] > sortedArray[i])
                {
                    changeOfNumberPlaces = sortedArray[i - 1];
                    sortedArray[i - 1] = sortedArray[i];
                    sortedArray[i] = changeOfNumberPlaces;
                }
            }
        }
        else
        {
            for (i = firstOdd; i < n - 1; i += 2)
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
}

void SortedLocalArrayToGlobalArray(int globalArray[], int localArray[], int localArraySize) {
    int i;
    for (i = 0; i < localArraySize; i++)
        globalArray[i] = localArray[i];
}

void PointerToArray(int globalArray[], int* compilator, int localArraySize) {
    int i;
    for (i = 0; i < localArraySize; i++) {
        globalArray[i] = compilator[i];
    }
}

int* MergeArraysTogether(int globalArray[], int recvArray[], int localArraySize, int recvArraySize) {
    int size = localArraySize + recvArraySize;
    int* mergeArray = new int[size];
    int i = 0, j = 0, k = 0;
    while (i <= localArraySize - 1 && j <= recvArraySize - 1) {
        if (globalArray[i] <= recvArray[j]) {
            mergeArray[k++] = globalArray[i++];
        }
        else {
            mergeArray[k++] = recvArray[j++];
        }
    }
    while (i <= localArraySize - 1) {
        mergeArray[k++] = globalArray[i++];
    }
    while (j <= recvArraySize - 1) {
        mergeArray[k++] = recvArray[j++];
    }
    return mergeArray;
}

int main(int argc, char* argv[]) {
    int NUM = 100000;
    int localArraySize;
    int processTasks, rankOfProcess;
    int* array = new int[NUM];
    int* globalArray = new int[NUM];
    int* compilator;
    MPI_Status status;
    MPI_Request request;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankOfProcess);
    int* localArray = new int[(NUM / processTasks)]; //Створюємо локальний масив для кожного процесу

    CreateArray(array, NUM); 

    //Розподіляємо масиви на процеси
    MPI_Scatter(array, NUM / processTasks, MPI_INT, localArray, NUM / processTasks, MPI_INT, 0, MPI_COMM_WORLD);
    localArraySize = NUM / processTasks;

    if (rankOfProcess == 0) {//Родич

        double timeSpent;
        auto start = high_resolution_clock::now();
        //double start = MPI_Wtime();
        PartialOddEvenSort(localArray, 0, localArraySize - 1); //Послідовне сортування

        SortedLocalArrayToGlobalArray(globalArray, localArray, localArraySize); //Передаємо відсортований локальний масив до глобального
        int recvArraySize = localArraySize;
        int* recvArray = new int[recvArraySize];
        for (int j = 0; j < processTasks - 1; j++) {

            MPI_Recv(recvArray, recvArraySize, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status); //Отримуємо відсортовані масиви від дочерніх класів

            compilator = MergeArraysTogether(globalArray, recvArray, localArraySize, recvArraySize); //Зливаємо отриманий масив та глобальний
            localArraySize += recvArraySize;
            PointerToArray(globalArray, compilator, localArraySize); //Ссилаємося на масив
        }
        //double end = MPI_Wtime();
        //double duraction = end - start;
        
        //PrintArray(global, NUM);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        //printf("Time: %f", duraction);
        cout << "Sorted time: " << duration.count() << endl;
    }
    else {//Дочерний
        PartialOddEvenSort(localArray, 0, localArraySize - 1);   //Послідовне сортуваня отриманого масиву
        MPI_Send(localArray, localArraySize, MPI_INT, 0, 0, MPI_COMM_WORLD);    //Відправляємо відсортований масив до родича
    }
    
    MPI_Finalize();
}