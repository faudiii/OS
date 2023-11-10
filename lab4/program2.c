#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
    void *libHandle = dlopen("./libsort.so", RTLD_LAZY);
    if (!libHandle)
    {
        fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
        return 1;
    }

    int size;
    int method;

    printf("Введите размер массива: ");
    scanf("%d", &size);

    int *array = (int *)malloc(size * sizeof(int));

    printf("Введите элементы массива:\n");
    for (int i = 0; i < size; i++)
    {
        scanf("%d", &array[i]);
    }

    printf("Выберите тип сортировки (0 - Быстрая Хоара, 1 - Пузырьковая): ");
    scanf("%d", &method);

    int *(*sortFunction)(int *, int) = NULL;

    if (method == 0)
    {
        sortFunction = dlsym(libHandle, "QuickSort");
    }
    else if (method == 1)
    {
        sortFunction = dlsym(libHandle, "BubbleSort");
    }
    else
    {
        fprintf(stderr, "Неправильный выбор метода сортировки.\n");
        dlclose(libHandle);
        free(array);
        return 1;
    }

    sortFunction(array, size);

    printf("Отсортированный массив: ");
    for (int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");

    free(array);
    dlclose(libHandle);

    return 0;
}
