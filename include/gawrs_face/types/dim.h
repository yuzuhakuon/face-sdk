#pragma once

#include <iostream>

namespace gawrs_face
{
struct Dims
{
    constexpr static int MAX_DIMS = 7;
    int d[MAX_DIMS];
    const int nbDims;

private:
    // Streaming operator.
    friend std::ostream& operator<<(std::ostream& out, const Dims& d)
    {
        out << "[";
        for (int i = 0; i < d.nbDims; i++)
        {
            out << d.d[i];
            if (i != d.nbDims - 1)
                out << ", ";
        }

        out << "]";

        return out;
    }
};
} // namespace gawrs_face