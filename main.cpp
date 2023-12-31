/*
* Bazhyn Oleksandr
* K-24
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
    std::uniform_int_distribution<int> dist(1, 1000000); // ������� �������

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

struct TimeResult
{
	std::string name;
	double result;
};

void printMinTime(std::vector<TimeResult>& times)
{
    TimeResult minTime = times[0];
    for (TimeResult time : times) {
        if (time.result < minTime.result) {
			minTime = time;
		}
	}

	std::cout << "The smallest number: " << minTime.name << " for\t" << minTime.result << " ms" << std::endl;
}

int ScalarProduction(const std::vector<int>& A, const std::vector<int>& B)
{
    if (A.size() != B.size())
    {
        throw std::runtime_error("Error: Vectors have different sizes");
    }

    int result = 0;
    for (size_t i = 0; i < A.size(); ++i)
    {
        result += A[i] * B[i];
    }
    return result;
}

std::vector<std::vector<int>> splitVector(const std::vector<int>& vec, int numParts)
{
    std::vector<std::vector<int>> result(numParts);
    int partSize = vec.size() / numParts;
    int remainder = vec.size() % numParts;

    int startIndex = 0;
    for (int i = 0; i < numParts; ++i)
    {
        int partLength = partSize + (i < remainder ? 1 : 0);

        if (partLength > 0)
        {
            result[i].assign(vec.begin() + startIndex, vec.begin() + startIndex + partLength);
            startIndex += partLength;
        }
    }

    return result;
}


double myAlgorithm(int K, std::vector<int>& A, std::vector<int>& B) {
    if (K <= 0 || K > A.size())
    {
        throw std::runtime_error("Error: Invalid value of K");
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> subArraysA = splitVector(A, K);
    std::vector<std::vector<int>> subArraysB = splitVector(B, K);

    int totalResult = 0;
    std::mutex resultMutex;

    std::vector<std::thread> threads;

    for (int i = 0; i < K; i++)
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

    auto stop = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = stop - start;

    //std::cout << "Result: " << totalResult << std::endl;
    //std::cout << "Execution time: " << duration.count() << " ms" << std::endl << std::endl;

    return duration.count();
}

void printMyAlgorithmKDependence(std::vector<int>& arr)
{
    std::cout << "My Algorithm" << std::endl;
    std::cout << "K" << "\t\t" << "Time, ms" << std::endl;

    int K = 1;
    TimeResult time1("K = 1", myAlgorithm(K, arr, arr));
    std::cout << K << "\t\t" << time1.result << std::endl;

    K = 50;
    TimeResult time2("K = 50", myAlgorithm(K, arr, arr));
    std::cout << K << "\t\t" << time2.result << std::endl;

    K = 100;
    TimeResult time3("K = 100", myAlgorithm(K, arr, arr));
    std::cout << K << "\t\t" << time3.result << std::endl;

    K = 250;
    TimeResult time4("K = 250", myAlgorithm(K, arr, arr));
    std::cout << K << "\t\t" << time4.result << std::endl;

    K = 500;
    TimeResult time5("K = 500", myAlgorithm(K, arr, arr));
    std::cout << K << "\t\t" << time5.result << std::endl;

    std::vector<TimeResult> times = { time1, time2, time3, time4, time5 };
    printMinTime(times);
}

int main()
{
    std::vector<int> arr = generateRandomVector(300000000);
    //std::vector<int> arr = generateRandomVector(300); //DEBUG

    std::cout << "EXECUTION POLICY: none" << std::endl;
    TimeResult time1("none", measureTransformReduceTime(arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; }));

    std::cout << "EXECUTION POLICY: sequenced policy" << std::endl;
    TimeResult time2("sequenced policy", measureTransformReduceTime(std::execution::seq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; }));

    std::cout << "EXECUTION POLICY: parallel policy" << std::endl;
    TimeResult time3("parallel policy", measureTransformReduceTime(std::execution::par, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; }));

    std::cout << "EXECUTION POLICY: parallel unsequenced policy" << std::endl;
    TimeResult time4("parallel unsequenced policy", measureTransformReduceTime(std::execution::par_unseq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; }));

    std::cout << "EXECUTION POLICY: unsequenced policy" << std::endl;
    TimeResult time5("unsequenced policy", measureTransformReduceTime(std::execution::unseq, arr.begin(), arr.end(), 0, std::plus<>(), [](int i) { return i * i; }));

    std::vector<TimeResult> times = { time1, time2, time3, time4, time5 };
    printMinTime(times);

    for (int i = 0; i < 20; i++)
    {
        std::cout << "=";
    }
    std::cout << std::endl << std::endl;

    printMyAlgorithmKDependence(arr);

    return 0;
}
