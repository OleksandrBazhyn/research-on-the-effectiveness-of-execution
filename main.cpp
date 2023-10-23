/*
* Compiler: GNU GCC 9
* var number 7) transform_reduce
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <chrono>
#include <random>
#include <execution>
#include <thread>
#include <future>
#include <mutex>


std::vector<int> generateRandomVector(int size)
{
    std::vector<int> result(size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 1000000); // діапазон значень

    for (int& value : result) {
        value = dist(gen);
    }

    return result;
}

template <typename ExecutionPolicy, typename InputIterator, typename T, typename BinaryOp, typename UnaryOp>
double measureTransformReduceTime(ExecutionPolicy policy, InputIterator begin, InputIterator end, T init, BinaryOp binaryOp, UnaryOp unaryOp)
{
    auto start = std::chrono::high_resolution_clock::now();

    int result = std::transform_reduce(policy, begin, end, init, binaryOp, unaryOp);

    auto stop = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = stop - start;

    std::cout << "Result: " << result << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl << std::endl;

    return duration.count();
}

template <typename InputIterator, typename T, typename BinaryOp, typename UnaryOp>
double measureTransformReduceTime(InputIterator begin, InputIterator end, T init, BinaryOp binaryOp, UnaryOp unaryOp)
{
    auto start = std::chrono::high_resolution_clock::now();

    int result = std::transform_reduce(begin, end, init, binaryOp, unaryOp);

    auto stop = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = stop - start;

    std::cout << "Result: " << result << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl << std::endl;

    return duration.count();
}

void printMinTime(std::vector<double> times)
{
	double minTime = times[0];
    for (double time : times) {
        if (time < minTime) {
			minTime = time;
		}
	}

	std::cout << "The smallest number: " << minTime << " ms" << std::endl << std::endl;
}

std::vector<std::vector<int>> splitArray(const std::vector<int>& inputArray, int K)
{
    std::vector<std::vector<int>> result;
    int arraySize = inputArray.size();
    int subArraySize = arraySize / K;
    int extraElements = arraySize % K;

    int currentIndex = 0;
    for (int i = 0; i < K; i++)
    {
        int subArrayLength = subArraySize + (extraElements-- > 0 ? 1 : 0);

        if (subArrayLength > 0)
        {
            result.push_back(std::vector<int>(inputArray.begin() + currentIndex, inputArray.begin() + currentIndex + subArrayLength));
            currentIndex += subArrayLength;
        }
    }

    return result;
}

int ScalarProduction(const std::vector<int>& A, const std::vector<int>& B)
{
    int result = 0;
    for (int i = 0; i < A.size(); i++)
    {
        result += A[i] * B[i];
    }
    return result;
}

double myAlgorithm(int K, std::vector<int>& A, std::vector<int>& B)
{
    auto start = std::chrono::high_resolution_clock::now();

    if (A.size() != B.size())
    {
        throw("Error: vectors have different sizes");
    }

    const int num_threads = 3; // Кількість потоків
    std::vector<std::vector<int>> subArraysA = splitArray(A, K);
    std::vector<std::vector<int>> subArraysB = splitArray(B, K);

    int totalResult = 0;
    std::mutex resultMutex;

    for (int i = 0; i < K; i++)
    {
        std::vector<std::thread> threads;

        for (int j = 0; j < num_threads && i < K; j++, i++)
        {
            threads.emplace_back(
                [i, &resultMutex, &totalResult, &subArraysA, &subArraysB]()
                {
                    int result = ScalarProduction(subArraysA[i], subArraysB[i]);

                    std::lock_guard<std::mutex> lock(resultMutex);

                    totalResult += result;
                });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = stop - start;

    std::cout << "Result: " << totalResult << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl << std::endl;

    return duration.count();
}

int main()
{
    //std::vector<int> arr = generateRandomVector(300000000);
    std::vector<int> arr = generateRandomVector(3000); //DEBUG

    std::cout << "EXECUTION POLICY: none" << std::endl;
    double time1 = measureTransformReduceTime(arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; });

    std::cout << "EXECUTION POLICY: sequenced policy" << std::endl;
    double time2 = measureTransformReduceTime(std::execution::seq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; });

    std::cout << "EXECUTION POLICY: parallel policy" << std::endl;
    double time3 = measureTransformReduceTime(std::execution::par, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; });

    std::cout << "EXECUTION POLICY: parallel unsequenced policy" << std::endl;
    double time4 = measureTransformReduceTime(std::execution::par_unseq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; });

    std::cout << "EXECUTION POLICY: unsequenced policy" << std::endl;
    double time5 = measureTransformReduceTime(std::execution::unseq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; });

    std::vector<double> times = { time1, time2, time3, time4, time5 };
    printMinTime(times);

    std::cout << "My Algorithm" << std::endl;
    double time6 = myAlgorithm(3, arr, arr);    

    return 0;
}
