#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>

typedef struct Data {
    int all;
    int curve;
    int rate;
} Data;

// Считывает исходные данные из файла
// В файле записаны через пробел три числа:
//      all, curve, rate
// где:
//      all - общее количество булавок
//      curve - количество кривых булавок
//      rate – процент неуспеха выполнения заточки булавки вторым рабочим

int read_data_from_file(const char* file_name, Data* data) {
    std::string line;
    std::string allStr;
    std::string curveStr;
    std::string rateStr;
    std::ifstream in(file_name); // окрываем файл для чтения
    if (in.is_open())
    {
        in >> allStr;
        in >> curveStr;
        in >> rateStr;
    }
    else {
        std::cout << "Can't open file " << file_name << std::endl;
        return errno;
    }
    in.close();
    try {
        data->all = stoi(allStr);
        data->curve = stoi(curveStr);
        data->rate = stoi(rateStr);
    }
    catch (std::invalid_argument) {
        return -1;
    }
    catch (std::out_of_range) {
        return -1;
    }
    return 0;
}

// Первый рабочий проверяет булавку на годность (1 - ок, 0 - не ок)
int worker1(int material) {
    return material;
}

// Второй берет хорошую булавку, и пытается ее заточить (с вероятностью rate ошибается)
int worker2(int material, int rate) {
    if (material == 0) return 0;
    return ((rand() % 100) < rate) ? 0 : 1;
}

// Третий рабочий выполняет контроль и считает количество выполненной работы
void worker3(int result, int* sum) {
    *sum += result;
}

int main() {
    srand(time(NULL));

    Data data;  // Переменная, в которую залетают исходные данные

    if (read_data_from_file("in.txt", &data) != 0) {
        return -1;
    }

    fprintf(stdout, "all: %d, curve: %d, rate: %d\n", data.all, data.curve, data.rate);

    // Массив с булавками, 1 - нормальная, 0 - кривая
    int* material = (int*)malloc(sizeof(int) * data.all);
    int curves = data.curve;

    // Генерируем случайные данные для булавок
    // Сначала все нормальные (1)
    for (size_t i = 0; i < data.all; ++i) {
        material[i] = 1;
    }

    // Затем случайным образом распределяем data.curve кривых булавок в массиве (0)
    for (int i = 0; i < curves; ++i) {
        size_t index = rand() % data.all;
        while (material[index] == 0) {
            index = rand() % data.all;
        }
        material[index] = 0;
    }

    // Теперь из data.all булавок кривые data.curve булавок (0), остальные нормальные (1)

    int b;  // Булавка
    int sum = 0;  // Переменная для подсчета результатов третьим процессом

    omp_set_dynamic(0);
    omp_set_num_threads(3);
#pragma omp parallel 
    {
        for (size_t i = 0; i < data.all; ++i) {
            b = material[i];
            if (omp_get_thread_num() == 0) {
                b = worker1(b);
            }
#pragma omp barrier
            if (omp_get_thread_num() == 1) {
                b = worker2(b, data.rate);
            }
#pragma omp barrier
            if (omp_get_thread_num() == 2) {
                worker3(b, &sum);
            }
#pragma omp barrier
        }
    }

    std::cout << "result: " << sum << std::endl;

    return 0;
}
