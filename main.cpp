#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <fstream>

// --- Параметри для Варіанту 5 (Схема №5) ---
const int NT = 5;

const int COUNT_A = 4;
const int COUNT_B = 5;
const int COUNT_C = 9;
const int COUNT_D = 9;

const int COUNT_E = 4;
const int COUNT_F = 4;

const int COUNT_G = 4;
const int COUNT_H = 9;
const int COUNT_I = 4;
const int COUNT_J = 6;

std::atomic<int> next_a{1}, next_b{1}, next_c{1}, next_d{1};
std::atomic<int> next_e{1}, next_f{1};
std::atomic<int> next_g{1}, next_h{1}, next_i{1}, next_j{1};

std::atomic<int> done_a{0};

std::atomic<int> done_bcd{0};
const int TOTAL_BCD = COUNT_B + COUNT_C + COUNT_D;

std::mutex io_mutex;

void f(char name, int index) {
    {
        std::lock_guard<std::mutex> lock(io_mutex);
        std::cout << "З набору " << name << " виконано дію " << index << ".\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

bool try_run_task(std::atomic<int>& next_idx, int max_count, char name, std::atomic<int>* completion_counter = nullptr) {
    int i = next_idx.load(std::memory_order_relaxed);
    if (i > max_count) return false;

    if (next_idx.compare_exchange_strong(i, i + 1)) {
        if (i <= max_count) {
            f(name, i);
            if (completion_counter) {
                (*completion_counter)++;
            }
            return true;
        }
    }
    return false;
}

void worker_thread() {
    while (true) {
        bool worked = false;

        if (try_run_task(next_a, COUNT_A, 'a', &done_a)) { worked = true; continue; }
        if (try_run_task(next_b, COUNT_B, 'b', &done_bcd)) { worked = true; continue; }
        if (try_run_task(next_c, COUNT_C, 'c', &done_bcd)) { worked = true; continue; }
        if (try_run_task(next_d, COUNT_D, 'd', &done_bcd)) { worked = true; continue; }


        if (done_a.load(std::memory_order_acquire) == COUNT_A) {
            if (try_run_task(next_e, COUNT_E, 'e')) { worked = true; continue; }
            if (try_run_task(next_f, COUNT_F, 'f')) { worked = true; continue; }
        }

        if (done_bcd.load(std::memory_order_acquire) == TOTAL_BCD) {
            if (try_run_task(next_g, COUNT_G, 'g')) { worked = true; continue; }
            if (try_run_task(next_h, COUNT_H, 'h')) { worked = true; continue; }
            if (try_run_task(next_i, COUNT_I, 'i')) { worked = true; continue; }
            if (try_run_task(next_j, COUNT_J, 'j')) { worked = true; continue; }
        }

        bool all_done =
            (next_a > COUNT_A) && (next_b > COUNT_B) && (next_c > COUNT_C) && (next_d > COUNT_D) &&
            (next_e > COUNT_E) && (next_f > COUNT_F) &&
            (next_g > COUNT_G) && (next_h > COUNT_H) && (next_i > COUNT_I) && (next_j > COUNT_J);

        if (all_done) break;

        if (!worked) {
            std::this_thread::yield();
        }
    }
}

int main() {
    std::cout << "Обчислення розпочато." << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < NT; ++i) {
        threads.emplace_back(worker_thread);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Обчислення завершено." << std::endl;
    return 0;
}