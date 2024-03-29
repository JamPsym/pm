#include "dsp_utils.h"

#include <algorithm>



using namespace dsp_utils;

window_generator::window_generator(int N)
  : size{N}, index{0} {}

hann_generator::hann_generator(int N)
  : window_generator(N) {}

hamming_generator::hamming_generator(int N)
  : window_generator(N) {}

void hann_generator::operator()(double& x) {
      x = x*(0.5*(1.0 - gsl_sf_cos(2.0*M_PI*(index++)/(size -1))));
}

void hamming_generator::operator()(double& x) {
      x = x*(0.54 - 0.46*gsl_sf_cos(2.0*M_PI*(index++)/(size -1)));
}

void dsp_utils::window_frame(std::array<double, N>& fr, const Window_type& win_type)
{
  switch(win_type)
  {
    case Window_type::hann_generator: 
      std::for_each(fr.begin(), fr.end(), hann_generator(N));

    case Window_type::hamming_generator:
      std::for_each(fr.begin(), fr.end(), hamming_generator(N));
  }
}

void dsp_utils::power_fft_frame(std::array<double, 256ul>& fr)
{
    gsl_fft_real_radix2_transform(fr.data(), 1, N);
    std::for_each(fr.begin(), fr.end(), [](double &x) {x = gsl_pow_2(x); });
    std::transform(fr.cbegin() +1, fr.cbegin() + (N/2),
                   fr.crbegin(),
                   fr.begin() +1,
                   std::plus<double>());
}

std::array<double, K> dsp_utils::dct_frame(const std::array<double, 30ul>& mel_frame, const std::array<double, 4*30ul>& cos_table)
{
    std::array<double, K> mfcc_frame;
    std::generate(mfcc_frame.begin(), mfcc_frame.end(), mfcc_gen<K>(mel_frame, cos_table));
    return mfcc_frame;
}

#include "test_inc/speak_frame.h"
#include "inc/mel_frame_generator.h"
//#include "inc/pm_test.h"
void dsp_utils::numeric_verifiaction()
{
  dsp_utils::window_frame(speak_test_frame, dsp_utils::Window_type::hamming_generator);
//test_mpl(speak_test_frame.begin(), speak_test_frame.end());
  dsp_utils::power_fft_frame(speak_test_frame);
//test_mpl(speak_test_frame.begin(), speak_test_frame.end());
  auto mfcc = mel_utils::mel_frame(speak_test_frame, 12500);
//test_mpl(mfcc.begin(), mfcc.end());
  std::for_each(mfcc.begin(), mfcc.end(), [](double &val){val = std::log10(val); });
  const auto K = 30;
  std::array<double, 4*K> cos_table;
  std::generate(cos_table.begin(), cos_table.end(), cos_dct_gen(4*K));
  mfcc = dsp_utils::dct_frame(mfcc, cos_table);
//test_mpl(mfcc.begin(), mfcc.end());
}
