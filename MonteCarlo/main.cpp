#include <thread>
#include <iostream>
#include <math.h>
#include <vector>
#include <mutex>
#include <numeric>


const unsigned int HARDWARE_CONCURRENCY = std::thread::hardware_concurrency();

inline int rrand(int range_max)
    {
        return rand() % (range_max + 1);
    }

int main()
    {
        std::vector<double> values;
        std::vector<std::thread> threads;
        threads.reserve(HARDWARE_CONCURRENCY);
        std::mutex mtx;
        srand((unsigned)time(0));

        const unsigned NUMBER = 10000000;
        const unsigned  side = RAND_MAX;
        const unsigned qside = side * side;
        

        for (int i = 0; i < HARDWARE_CONCURRENCY; ++i)
        {
            threads.emplace_back([&values, NUMBER, side, qside, &mtx]() {
                unsigned cnt = 0;

                for (unsigned i = 0; i < NUMBER; ++i)
                {
                    unsigned x = rrand(side),
                        y = rrand(side);

                    if (x * x + y * y < qside) ++cnt;
                }
                std::lock_guard<std::mutex> lock(mtx);
                values.emplace_back((4. * cnt) / NUMBER);
            });
        }
        for (auto& th : threads)
        {
            th.join();
        }

        double total = accumulate(values.begin(), values.end(), 0.0);
        double min = total / HARDWARE_CONCURRENCY;
        std::cout.precision(30);
        std::cout << min << "\n";
    }