/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <modules/visualneuro/statistics/ttest.h>

#include <tuple>

namespace inviwo {

namespace stats {

enum class IVW_MODULE_VISUALNEURO_API CorrelationMethod { Spearman, Pearson };
/**
 * \brief Compute correlation and p-value for two equally sized groups.
 * @return Correlation value in [-1 1] and its probability, p-value, in [0 1]
 * \pre A must have same size as B
 */
template <typename Iterator>
std::tuple<double, double> corrTest(Iterator firstA, Iterator lastA,
                                                                Iterator firstB, Iterator lastB,
                                                                CorrelationMethod method,
                                                                TailTest tailTest) {
    double r;
    switch (method) {
        case inviwo::stats::CorrelationMethod::Spearman:
            r = spearmanCorrelation(firstA, lastA, firstB, lastB);
            break;
        case inviwo::stats::CorrelationMethod::Pearson:
            r = pearsonCorrelation(firstA, lastA, firstB, lastB);
            break;
        default:
            r = 0;
            IVW_ASSERT(true, "Correlation method not implemented");
            break;
    }
    auto n = std::distance(firstA, lastA);
    // Test for significance according to Student's t-distribution
    auto t = r * sqrt((n - 2) / (1.0 - r * r));
    auto p = stats::tailTest(t, static_cast<double>(n-2), tailTest);
    return {r, p};
}

IVW_MODULE_VISUALNEURO_API std::tuple<double, double> corrTest(const std::vector<double>& A,
                                                                const std::vector<double>& B,
                                                                CorrelationMethod method,
                                                                TailTest tailTest);

}  // namespace stats

}  // namespace inviwo
