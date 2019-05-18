#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

// tries to estimate the frequency, returns false on failure
double measure_frequency() {
  const size_t test_duration_in_cycles =
     65536;// 1048576; 
  // travis feels strongly about the measure-twice-and-subtract trick.
  static_assert(test_duration_in_cycles / 16 * 16 == test_duration_in_cycles);
  auto begin1 = std::chrono::high_resolution_clock::now();
  size_t cycles = 2 * test_duration_in_cycles;
#ifdef __x86_64__
  __asm volatile("cyclemeasure1:\n dec %[counter] \n jnz cyclemeasure1 \n"
                 : /* read/write reg */ [counter] "+r"(cycles));
#elif defined(__aarch64__)
  // trying to bring the loop overhead down?
  __asm volatile(
      "cyclemeasure1:\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsubs %[counter],%[counter],#1\nbne cyclemeasure1\n "
      : /* read/write reg */ [counter] "+r"(cycles));
#endif
  auto end1 = std::chrono::high_resolution_clock::now();
  double nanoseconds1 =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - begin1)
          .count();
  auto begin2 = std::chrono::high_resolution_clock::now();
  cycles = test_duration_in_cycles;
#ifdef __x86_64__
  __asm volatile("cyclemeasure2:\n dec %[counter] \n jnz cyclemeasure2 \n"
                 : /* read/write reg */ [counter] "+r"(cycles));
#elif defined(__aarch64__)
  __asm volatile(
      "cyclemeasure2:\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsub %[counter],%[counter],#1\nsubs %[counter],%[counter],#1\nbne cyclemeasure2\n "
      : /* read/write reg */ [counter] "+r"(cycles));
#endif
  auto end2 = std::chrono::high_resolution_clock::now();
  double nanoseconds2 =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2)
          .count();
  double nanoseconds = (nanoseconds1 - nanoseconds2);
  if ((fabs(nanoseconds - nanoseconds1 / 2) > 0.01 * nanoseconds) or
      (fabs(nanoseconds - nanoseconds2) > 0.01 * nanoseconds)) {
    return 0;
  }

  double frequency = double(test_duration_in_cycles) / nanoseconds;
  return frequency;
}

int main(int argc,char** argv) {
  int attempt = 1000;
  if(argc > 1) {
      attempt = atoi(argv[1]);
  }
  if(attempt <= 0) attempt = 1;
  std::vector<double> freqs;
  for (int i = 0; i < attempt; i++) {
    double freq = measure_frequency();
    if(freq > 0) freqs.push_back(freq);
  }
  std::sort(freqs.begin(),freqs.end());
  std::cout << "median frequency detected: " << freqs[freqs.size() / 2] << " GHz" << std::endl;
}
