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

#ifndef IVW_PEARSONCORRELATION_H
#define IVW_PEARSONCORRELATION_H

#include <inviwo/core/common/inviwo.h>
#include <modules/visualneuro/visualneuromoduledefine.h>
#include <execution>



IVW_MODULE_VISUALNEURO_API double calculateMeanIgnoreNaNs(const std::vector<double>& v);

template <typename Iterator>
double calculateMeanIgnoreNaNs(Iterator first, Iterator last) {
    double mean = 0.0;
    size_t validElements = 0;
    for (auto element = first; element != last; element++) {
        if (isnan(*element)) continue;
        mean += static_cast<double>(*element);
        validElements++;
    }

    return mean / validElements;
}

// Three overloaded functions for calculating the Pearson correlation between two datasets A and B.
IVW_MODULE_VISUALNEURO_API double pearsonCorrelation(const std::vector<double>& A,
                                                     const std::vector<double>& B);

IVW_MODULE_VISUALNEURO_API double pearsonCorrelation(const std::vector<double>& A,
                                                     const std::vector<double>& B,
                                                     const double& meanOfA);

IVW_MODULE_VISUALNEURO_API double pearsonCorrelation(const std::vector<double>& A,
                                                     const std::vector<double>& B,
                                                     const double& meanOfA, const double& meanOfB);

template <typename Iterator>
double pearsonCorrelation(Iterator firstA, Iterator lastA, Iterator firstB, Iterator lastB,
                          const double& meanOfA, const double& meanOfB) {
    auto nA = std::distance(firstA, lastA);
    auto nB = std::distance(firstB, lastB);
    if (nA != nB) throw inviwo::Exception("The size of the two entered datasets does not match!");

    std::vector<double> diffA, diffB;
    diffA.reserve(nA);
    diffB.reserve(nB);
    std::transform(firstA, lastA, std::back_inserter(diffA),
                   [meanOfA](auto v) { return v - meanOfA; });
    std::transform(firstB, lastB, std::back_inserter(diffB),
                   [meanOfB](auto v) { return v - meanOfB; });

    auto numerator = std::transform_reduce(std::execution::par, diffA.begin(), diffA.end(),
                                           diffB.begin(), 0.0, std::plus<>(), std::multiplies<>());
    auto standardDeviationParameters =
        std::transform_reduce(std::execution::par, diffA.begin(), diffA.end(), diffA.begin(), 0.0,
                              std::plus<>(), std::multiplies<>());
    auto standardDeviationVoxels =
        std::transform_reduce(std::execution::par, diffB.begin(), diffB.end(), diffB.begin(), 0.0,
                              std::plus<>(), std::multiplies<>());

    numerator /= nA;

    standardDeviationParameters = sqrt(standardDeviationParameters / nA);
    standardDeviationVoxels = sqrt(standardDeviationVoxels / nB);

    return numerator / (standardDeviationParameters * standardDeviationVoxels);
}

template <typename Iterator>
double pearsonCorrelation(Iterator firstA, Iterator lastA, Iterator firstB, Iterator lastB) {
    double meanOfA = calculateMeanIgnoreNaNs(firstA, lastA);
    double meanOfB = calculateMeanIgnoreNaNs(firstB, lastB);

    return pearsonCorrelation(firstA, lastA, firstB, lastB, meanOfA, meanOfB);
}





#endif  // IVW_PEARSONCORRELATION_H
