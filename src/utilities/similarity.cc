#include <cmath>

#include "gawrs_face/utilities/similarity.h"

namespace gawrs_face
{
float cosineSimilarity(const float* A, const float* B, unsigned int len)
{
    // Compute dot product
    // Compute the norm (magnitude) of each vector
    double dot = 0.0, denomA = 0.0, denomB = 0.0;
    for (unsigned int i = 0u; i < len; ++i)
    {
        dot += A[i] * B[i];
        denomA += A[i] * A[i];
        denomB += B[i] * B[i];
    }
    // Compute cosine similarity
    return dot / (std::sqrt(denomA * denomB));
}
} // namespace gawrs_face