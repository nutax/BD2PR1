#include "tables.hpp"
#include <chrono>
#include <thread>
#include <functional>
#include <sstream>
#include <mutex>

std::mutex mtx;

void cliente_operations(void){
std::vector<std::function<void(void)>> writes;
std::vector<std::function<void(std::stringstream&)>> reads;

reads.push_back([](std::stringstream& ss){cliente result;
if(cliente_.search({12345}, result)) ss<<result;});

std::vector<std::stringstream>ssv(reads.size());
std::vector<std::thread> tv;
for(std::size_t i = 0; i<reads.size(); ++i)tv.emplace_back(reads[i], std::ref(ssv[i]));
for(auto& t : tv) t.join();
for(const auto& w : writes) w();
std::fstream file;
mtx.lock();
file.open("OUTPUT/_results.txt", std::fstream::app);
file<<"TABLE: "<<"cliente"<<std::endl;
for(auto& ss : ssv) file<<ss.rdbuf()<<std::endl;
file.close();
mtx.unlock();
}

void pastel_operations(void){
std::vector<std::function<void(void)>> writes;
std::vector<std::function<void(std::stringstream&)>> reads;

reads.push_back([](std::stringstream& ss){pastel result;
if(pastel_.search({13}, result)) ss<<result;});

std::vector<std::stringstream>ssv(reads.size());
std::vector<std::thread> tv;
for(std::size_t i = 0; i<reads.size(); ++i)tv.emplace_back(reads[i], std::ref(ssv[i]));
for(auto& t : tv) t.join();
for(const auto& w : writes) w();
std::fstream file;
mtx.lock();
file.open("OUTPUT/_results.txt", std::fstream::app);
file<<"TABLE: "<<"pastel"<<std::endl;
for(auto& ss : ssv) file<<ss.rdbuf()<<std::endl;
file.close();
mtx.unlock();
}


int main(void){
auto _start = std::chrono::high_resolution_clock::now();
std::thread cliente_thread (cliente_operations);
std::thread pastel_thread (pastel_operations);
cliente_thread.join();
pastel_thread.join();

auto _stop = std::chrono::high_resolution_clock::now();
auto _duration = std::chrono::duration_cast<std::chrono::microseconds>(_stop - _start);
std::fstream _file("OUTPUT/_results.txt", std::fstream::app); _file<<std::endl<<"Total execution time of all read and writes: "<<_duration.count()<<" microseconds"; _file.close();
}
