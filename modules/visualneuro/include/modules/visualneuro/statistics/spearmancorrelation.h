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

#include <execution>
#include <numeric>
#include <iterator>

namespace inviwo {

namespace stats {

/*
 * \brief Rank elements from 1 to N in ascending order.
 * The rank of repeated elements will be averaged.
 * Example:
 * Input: 20, 30, 30, 10
 * Output: 2, 3.5, 3.5, 1
 */
template <typename Iterator>
std::vector<double> rank(Iterator first, Iterator last) {
    // Create pairs of indices and values
    // pairs[i] == A[i], i
    std::vector<std::pair<Iterator::value_type, size_t>> pairs;
    auto nElements = std::distance(first, last);
    pairs.reserve(nElements);
    size_t index = 0;
    std::transform(first, last, std::back_inserter(pairs),
                   [&index](const auto v) { return std::make_pair(v, index++); });
    // Sort according to values in a
    std::sort(std::begin(pairs), std::end(pairs),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<double> R;  // Rank
    R.resize(nElements);
    auto r = 1u, n = 1u, i = 0u;
    while (i < nElements) {
        auto j = i;
        // Find out how many elements that are equal
        while (j < nElements - 1 && pairs[j].first == pairs[j + 1].first) {
            j += 1;
        }
        // Number of adjacent elements
        n = j - i + 1;
        for (auto k = 0u; k < n; ++k) {
            auto idx = pairs[i + k].second;
            // Use rank formula, n==1 if there are no adjacent elements
            R[idx] = r + (n - 1) * 0.5;
        }
        // Skip already adjusted elements
        r += n;
        i += n;
    }
    return R;
}

template <typename T>
std::vector<double> rank(const std::vector<T> A) {
    return rank(std::begin(A), std::end(A));
}

/*
 * \brief Computes the non-parametric Spearman rank correlation.
 * Measures the monotonic relationship between two variables rather than
 * the linear relationshop, e.g. Pearson correlation.
 * Monotonic: As one variable increases, so does the other and vice versa.
 * Non-monotonic: As one variable increases the other decreases.
 * Thus, monotonic is not as strict as linear correlation.
 * Spearman correlation coefficient is in range [-1 1], where -1 indicates perfect negative
 * association, 0 indicates no association, +1 indicates perfect positive assocation.
 *
 * \pre A and B must be of same size, otherwise std::invalid_argument exception is thrown.
 */
template <typename Iterator>
double spearmanCorrelation(Iterator firstA, Iterator lastA, Iterator firstB, Iterator lastB) {
    if (std::distance(firstA, lastA) != std::distance(firstB, lastB))
        throw std::invalid_argument("The size of the two entered datasets does not match!");

    auto rankA(rank(firstA, lastA));
    auto rankB(rank(firstB, lastB));
    auto squaredDifference = [](const auto& a, const auto& b) {
        auto diff = a - b;
        return diff * diff;
    };
    double squaredDiffSum =
        std::transform_reduce(std::execution::par, rankA.begin(), rankA.end(), rankB.begin(), 0.0,
                              std::plus<>(), squaredDifference);

    return 1.0 - 6.0 * squaredDiffSum /
                     static_cast<double>(rankA.size() * (rankA.size() * rankA.size() - 1.0));
}

IVW_MODULE_VISUALNEURO_API double spearmanCorrelation(const std::vector<double>& A,
                                                       const std::vector<double>& B);



}  // namespace stats

}  // namespace inviwo
