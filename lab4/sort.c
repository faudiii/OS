#include "sort.h"

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int *array, int low, int high)
{
    int pivot = array[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++)
    {
        if (array[j] < pivot)
        {
            i++;
            swap(&array[i], &array[j]);
        }
    }

    swap(&array[i + 1], &array[high]);
    return i + 1;
}

int *QuickSortHelper(int *array, int low, int high)
{
    if (low < high)
    {
        int pi = partition(array, low, high);

        QuickSortHelper(array, low, pi - 1);
        QuickSortHelper(array, pi + 1, high);
    }

    return array;
}

int *QuickSort(int *array, int size)
{
    return QuickSortHelper(array, 0, size - 1);
}

int *BubbleSort(int *array, int size)
{
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                swap(&array[j], &array[j + 1]);
            }
        }
    }

    return array;
}
