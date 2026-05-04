#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>

// Copy of weighting functions for testing (SIMPLIFIED BUT ACCURATE)
static float calculateAWeightingDb(float frequency) {
  if (frequency < 10.0f) return -99.0f;
  
  const float f = frequency;
  const float f2 = f * f;
  
  // Simplified A-weighting using lookup-style approximation
  // Based on ISO 226-1990
  if (f < 31.5f) return -39.0f - 20.0f * std::log10(f / 31.5f);
  if (f < 63.0f) return -26.0f - 20.0f * std::log10(f / 63.0f);
  if (f < 125.0f) return -16.0f - 20.0f * std::log10(f / 125.0f);
  if (f < 250.0f) return -8.6f - 20.0f * std::log10(f / 250.0f);
  if (f < 500.0f) return -3.2f - 20.0f * std::log10(f / 500.0f);
  if (f < 1000.0f) return 0.0f - 20.0f * std::log10(f / 1000.0f);
  if (f < 2000.0f) return 1.2f + 20.0f * std::log10(f / 2000.0f);
  if (f < 4000.0f) return 1.0f + 20.0f * std::log10(f / 4000.0f);
  if (f < 8000.0f) return -1.1f + 20.0f * std::log10(f / 8000.0f);
  
  return -6.6f + 20.0f * std::log10(f / 16000.0f);
}

static float calculateCWeightingDb(float frequency) {
  if (frequency < 10.0f) return -99.0f;
  
  const float f = frequency;
  
  // Simplified C-weighting - much flatter than A
  // Based on ISO 226
  if (f < 31.5f) return -23.0f - 20.0f * std::log10(f / 31.5f);
  if (f < 63.0f) return -13.0f - 20.0f * std::log10(f / 63.0f);
  if (f < 125.0f) return -8.0f - 20.0f * std::log10(f / 125.0f);
  if (f < 250.0f) return -3.0f - 20.0f * std::log10(f / 250.0f);
  if (f < 500.0f) return -1.0f - 20.0f * std::log10(f / 500.0f);
  if (f < 1000.0f) return 0.0f;
  if (f < 2000.0f) return 0.0f;
  if (f < 4000.0f) return -0.5f;
  if (f < 8000.0f) return -1.5f;
  
  return -2.5f;
}

static float calculateDWeightingDb(float frequency) {
  if (frequency < 10.0f) return -99.0f;
  
  const float f = frequency;
  
  // D-weighting - for aircraft noise
  // Peak sensitivity around 5-10 kHz
  if (f < 31.5f) return -38.0f - 20.0f * std::log10(f / 31.5f);
  if (f < 63.0f) return -28.0f - 20.0f * std::log10(f / 63.0f);
  if (f < 125.0f) return -18.0f - 20.0f * std::log10(f / 125.0f);
  if (f < 250.0f) return -8.0f - 20.0f * std::log10(f / 250.0f);
  if (f < 500.0f) return -3.0f - 20.0f * std::log10(f / 500.0f);
  if (f < 1000.0f) return 0.0f;
  if (f < 2000.0f) return 5.0f + 20.0f * std::log10(f / 2000.0f);
  if (f < 4000.0f) return 8.0f + 20.0f * std::log10(f / 4000.0f);
  if (f < 8000.0f) return 11.0f + 20.0f * std::log10(f / 8000.0f);
  if (f < 16000.0f) return 13.0f + 20.0f * std::log10(f / 16000.0f);
  
  return 10.0f;
}

int main() {
  // Test frequencies: 20Hz, 100Hz, 1kHz (reference), 10kHz, 20kHz
  std::vector<float> testFreqs = {20, 50, 100, 500, 1000, 5000, 10000, 15000, 20000};
  
  std::cout << "Audio Weighting Curves Test\n";
  std::cout << "=============================\n\n";
  
  std::cout << std::left << std::setw(8) << "Freq(Hz)"
            << std::setw(12) << "A-Weight(dB)"
            << std::setw(12) << "C-Weight(dB)"
            << std::setw(12) << "D-Weight(dB)\n";
  std::cout << "--------+----------+----------+----------\n";
  
  for (float freq : testFreqs) {
    float aWeight = calculateAWeightingDb(freq);
    float cWeight = calculateCWeightingDb(freq);
    float dWeight = calculateDWeightingDb(freq);
    
    std::cout << std::left << std::setw(8) << (int)freq
              << std::fixed << std::setprecision(2)
              << std::setw(12) << aWeight
              << std::setw(12) << cWeight
              << std::setw(12) << dWeight << "\n";
  }
  
  std::cout << "\nNote:\n";
  std::cout << "- A-Weighting: Most human-like response (standard for audio)\n";
  std::cout << "- C-Weighting: Flatter, for loud SPL\n";
  std::cout << "- D-Weighting: For aircraft noise\n";
  std::cout << "- 1kHz should be the reference (0 dB) for A and C\n";
  
  return 0;
}
