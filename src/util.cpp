#include "util.hpp"

namespace entity {

void VectorAdd(float* a, const float* b, int dim) {
	for (int i = 0; i < dim; ++i) {
		a[i] += b[i];
  }
}

void MatrixVectorMultiply(const float* const* W, const float* a,
    float* b, int m, int n) {
  for (int i = 0; i < m; ++i) {
    float sum = 0.;
    for (int j = 0; j < n; ++j) {
      sum += W[i][j] * a[j];
    }
    b[i] = sum;
  }
}

void MatrixTransposedVectorMultiply(const float* const* W,
    const float* a, float* b, int m, int n) {
  for (int i = 0; i < n; ++i) {
    float sum = 0.;
    for (int j = 0; j < m; ++j) {
      sum += W[j][i] * a[j];
    }
    b[i] = sum;
  }
}

void LogisticComponentwise(float* x, int dim) {
  for (int i = 0; i < dim; ++i) {
    x[i] = fastsigmoid(x[i]);
  }
}

float LogisticDerivative(float logistic) {
  return logistic * (1. - logistic);
}

float LogSum(float log_a, float log_b) {
  return (log_a < log_b) ? log_b + fastlog(1 + fastexp(log_a - log_b)) :
    log_a + log(1 + fastexp(log_b-log_a));
}

float LogSumVec(const float* logvec, int D) {
	float sum = 0.;
	sum = logvec[0];
	for (int i = 1; i < D; ++i) {
		sum = LogSum(sum, logvec[i]);
	}
	return sum;
}

void Softmax(float* vec, int dim) {
  for (int i = 0; i < dim; ++i) {
    if (std::abs(vec[i]) < 1e-10) {
      vec[i] = 1e-10;
    }
  }
  double lsum = LogSumVec(vec, dim);
  for (int i = 0; i < dim; ++i) {
    vec[i] = fastexp(vec[i] - lsum);
  }
}

void ComputeSoftmaxDelta(const float* output_layer_units,
    int32_t num_output_units, int32_t output_label,
    float* delta_output_layer) {
  for (int i = 0; i < num_output_units; ++i) {
    if (i == output_label) {
      delta_output_layer[i] = output_layer_units[i] - 1;
    } else {
      delta_output_layer[i] = output_layer_units[i];
    }
  }
}


}  // namespace entity
