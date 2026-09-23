#ifndef TOLERANCE_H
#define TOLERANCE_H

#include <stddef.h>

static inline int compare_f32_tolerance(float const *actual,
                                        float const *reference, size_t count,
                                        float tolerance, float *max_error) {
  float max = 0.0f;
  int pass = 1;

  for (size_t i = 0; i < count; ++i) {
    float error = actual[i] - reference[i];
    if (error < 0.0f)
      error = -error;
    if (error != error) {
      *max_error = error;
      return 0;
    }
    if (error > max)
      max = error;
    if (!(error <= tolerance))
      pass = 0;
  }

  *max_error = max;
  return pass;
}

#endif
