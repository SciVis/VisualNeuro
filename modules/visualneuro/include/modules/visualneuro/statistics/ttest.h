/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/core/common/inviwo.h>
#include <modules/visualneuro/visualneuromoduledefine.h>

namespace inviwo {

namespace stats {

enum class IVW_MODULE_VISUALNEURO_API TailTest { TwoTailed, RightOneTailed, LeftOneTailed };

// Calculate the variance for vector v when vector mean is not previously known
template <typename T>
double calculateVariance(const std::vector<T>& v) {
    double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    return calculateVariance(v, mean);
}

// Calculate the variance for vector v when vector mean is known
template <typename T>
double calculateVariance(const std::vector<T>& v, const double mean) {
    std::vector<T> diff(v.size());
    std::transform(v.begin(), v.end(), diff.begin(), [mean](T x) { return x - mean; });
    return std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / (v.size() - 1);
}

// Calculate the t-test between vector A and B
template <typename T>
double tTest(const std::vector<T>& A, const std::vector<T>& B) {
    double meanA = std::accumulate(A.begin(), A.end(), 0.0) / A.size();
    double meanB = std::accumulate(B.begin(), B.end(), 0.0) / B.size();

    double varianceA = calculateVariance(A, meanA);
    double varianceB = calculateVariance(B, meanB);

    return (meanA - meanB) / sqrt((varianceA / A.size()) + (varianceB / B.size()));
}

IVW_MODULE_VISUALNEURO_API double student_t_cdf(double t, double df);
IVW_MODULE_VISUALNEURO_API double incbeta(double a, double b, double x);

// Calculate the p-value based on t-Test
IVW_MODULE_VISUALNEURO_API double tailTest(const double t, size_t sampleSize, TailTest tailTest);

}  // namespace stats

}  // namespace inviwo
