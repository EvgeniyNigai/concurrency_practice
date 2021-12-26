#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

const unsigned int HARDWARE_CONCURRENCY = std::thread::hardware_concurrency();
const unsigned int MAX_MEMORY = 128;
using SORT_TYPE = uint64_t;
const size_t MAX_ARRAY_SIZE = MAX_MEMORY / HARDWARE_CONCURRENCY / sizeof(SORT_TYPE);


std::vector<SORT_TYPE> ReadData(std::istream& in, const size_t max_size) {
    std::vector<SORT_TYPE> result;
    if (!in) {
        return result;
    }
    result.reserve(max_size);
    size_t read = 0;
    SORT_TYPE data;
    while (read < max_size && in.read(&data, sizeof(data))) {
        result.emplace_back(data);
        ++read;
    }
    return result;
}

std::vector<std::string> ReadDataChunks(std::istream& in) {
    std::vector<std::thread> threads;
    threads.reserve(HARDWARE_CONCURRENCY);
    std::vector<std::string> chunks;
    size_t file_counter = 0;
    while (true) {
        std::vector<SORT_TYPE> v = ReadData(in, MAX_ARRAY_SIZE);
        if (v.empty()) {
            break;
        }
        threads.emplace_back([&chunks, v, file_counter]() mutable {
            std::sort(v.begin(), v.end());
            chunks.emplace_back("temp_" + std::to_string(file_counter) + ".bin");
            std::ofstream out(chunks.back());
            for (auto x : v) {
                out << x << " ";
            }
            out.close();
        });
        ++file_counter;
        if (threads.size() == HARDWARE_CONCURRENCY) {
            for (auto& th : threads) {
                th.join();
            }
            threads.clear();
            threads.reserve(HARDWARE_CONCURRENCY);
        }
    }
    for (auto& th : threads) {
        th.join();
    }
    return chunks;
}

void MergeChunks(std::ofstream& out, const std::vector<std::string>& chunks) {
    std::vector<std::ifstream> chunk_streams;
    chunk_streams.reserve(chunks.size());
    for (auto& ch : chunks) {
        chunk_streams.emplace_back(ch);
    }

    std::priority_queue<std::pair<SORT_TYPE, size_t>,
        std::vector<std::pair<SORT_TYPE, size_t>>, std::greater<>> queue;
    for (size_t i = 0; i < chunks.size(); ++i) {
        SORT_TYPE element;
        if (chunk_streams[i] >> element) {
            queue.emplace(element, i);
        }
    }

    while (!queue.empty()) {
        auto&& top = queue.top();
        queue.pop();
        out << top.first << " ";
        SORT_TYPE element;
        if (chunk_streams[top.second] >> element) {
            queue.emplace(element, top.second);
        }
    }

    for (auto&& ch : chunk_streams) {
        ch.close();
    }

}

int main() {

    std::ifstream in("input.bin");
    auto chunk_files = ReadDataChunks(in);
    std::ofstream out("output.bin");
    MergeChunks(out, chunk_files);

    return 0;

}