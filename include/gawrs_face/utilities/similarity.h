#pragma once

namespace gawrs_face
{
/**
 * @brief Calculate the cosine similarity between two vectors A and B.
 *
 * @param A First vector
 * @param B Second vector
 * @param len Vectors must be of the same length
 * @return float Cosine similarity between A and B
 */
float cosineSimilarity(const float* A, const float* B, unsigned int len);
} // namespace gawrs_face