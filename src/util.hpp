#pragma once

#include <cmath>

namespace entity {

// a = a + b.
void VectorAdd(float* a, const float* b, int dim);

// Set b = W * a. W is of size [m x n], len(a) = n, len(b) = m. It assumes a
// and b have been allocated.
void MatrixVectorMultiply(const float* const* W, const float* a,
    float* b, int m, int n);

// Set b = W' * a (W' is W transposed). W is of size [m x n], len(a) = m,
// len(b) = n. It assumes a and b have been allocated.
void MatrixTransposedVectorMultiply(const float* const* W,
    const float* a, float* b, int m, int n);

// Compute x[i] = logistic(x[i])
void LogisticComponentwise(float* x, int dim);

// sigmoid'(a) = sigmoid(a) * (1 - sigmoid(a))
float LogisticDerivative(float logistic);

// Compute log(a + b) from log(a) and log(b) using the identity
// log(a + b) = log(a) + log(1 + (b/a))
float LogSum(float log_a, float log_b);

// Return log{\sum_i exp(logvec[i])}.
float LogSumVec(const float* logvec, int D);

// vec[i] = softmax(vec[i], vec).
void Softmax(float* vec, int dim);

// Softmax delta for unit i is delta_i = z_i - I(i = y).
// See http://www.iro.umontreal.ca/~bengioy/ift6266/H12/html.old/mlp_en.html
void ComputeSoftmaxDelta(const float* output_layer_units,
    int32_t num_output_units, int32_t output_label,
    float* delta_output_layer);

}  // namespace dnn
