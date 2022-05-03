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

#include <modules/visualneuro/visualneuromoduledefine.h>

#include <algorithm>
#include <vector>
#include <numeric>
#include <math.h>

namespace inviwo {

namespace stats {

enum class IVW_MODULE_VISUALNEURO_API TailTest { Both, Greater, Less };

enum class IVW_MODULE_VISUALNEURO_API EqualVariance : bool { Yes = true, No = false };

IVW_MODULE_VISUALNEURO_API double student_t_cdf(double t, double df);
IVW_MODULE_VISUALNEURO_API double incbeta(double a, double b, double x);
// Calculate the p-value based on the Student's distribution
IVW_MODULE_VISUALNEURO_API double tailTest(const double t, double degreesOfFreedom,
                                           TailTest tailTest);

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

/*
 * \brief Test if two independent samples have equal means.
 * Calculates the t-test between vectors A and B and its probability to
 * be this value by chance.
 * Uses Student't t-test if EqualVariance = Yes, and otherwise Welch's t-test. Small probability
 * values indicate that the two vectors have equal means.
 * @param A independent samples from a population.
 * @param B independent samples from a population.
 * @param equalVariance Yes if the samples are assumed to have the same variance, No otherwise.
 * @param tailTest Both: Mean of A is same as mean of B. Greater: Mean of A is greater than mean of B. Less: Mean of
 * A is less than mean of B.
 * @return the significance 't' and the probability 'p-value'
 */
template <typename T>
std::tuple<double, double> tTest(const std::vector<T>& A, const std::vector<T>& B,
                                 EqualVariance equalVariance = EqualVariance::No,
                                 TailTest tail = TailTest::Both) {
    double meanA = std::accumulate(A.begin(), A.end(), 0.0) / A.size();
    double meanB = std::accumulate(B.begin(), B.end(), 0.0) / B.size();

    double varianceA = calculateVariance(A, meanA);
    double varianceB = calculateVariance(B, meanB);

    // Degrees of freedom
    double df, denom;
    if (equalVariance == EqualVariance::Yes) {
        df = static_cast<double>(A.size() + B.size() - 2);
        double svar = ((A.size() - 1) * varianceA + (B.size() - 1) * varianceB) / df;
        denom = sqrt(svar * (1.0 / A.size() + 1.0 / B.size()));
    } else {
        // See numerical recipies 'tutest'
        double vn1 = varianceA / A.size();
        double vn2 = varianceB / B.size();
        df = (vn1 + vn2) * (vn1 + vn2) / (vn1 * vn1 / (A.size() - 1) + vn2 * vn2 / (B.size() - 1));
        denom = sqrt(vn1 + vn2);
    }

    double t = (meanA - meanB) / denom;
    double prob = tailTest(t, df, tail);
    return {t, prob};
}

/*
 * \brief Test if two populations have different variances.
 * Calculates the F-test between vectors A and B and its probability to
 * be by chance. F is the ratio of one variance to the other, so values either >> 1 or << 1
 * indicate significant differences. Small probability values indicate that the two vectors have
 * significantly different variances.
 * @return the significance 't' and the two-tailed probability 'p-value'
 */
template <typename T>
std::tuple<double, double> fTest(const std::vector<T>& A, const std::vector<T>& B) {
    // See numerical recipies 'ftest'
    double meanA = std::accumulate(A.begin(), A.end(), 0.0) / A.size();
    double meanB = std::accumulate(B.begin(), B.end(), 0.0) / B.size();

    double varianceA = calculateVariance(A, meanA);
    double varianceB = calculateVariance(B, meanB);
    double f, df1, df2;
    if (varianceA > varianceB) {
        f = varianceA / varianceB;
        df1 = A.size() - 1;
        df2 = B.size() - 1;
    } else {
        f = varianceB / varianceA;
        df1 = B.size() - 1;
        df2 = A.size() - 1;
    }
    double prob = 2.0 * incbeta(df2 / 2.0, df1 / 2.0, df2 / (df2 + df1 * f));
    if (prob > 1.0) {
        prob = 2.0 - prob;
    }
    return {f, prob};
}

}  // namespace stats

}  // namespace inviwo
