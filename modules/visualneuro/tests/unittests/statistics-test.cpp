/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/visualneuro/statistics/correlation.h>
#include <modules/visualneuro/statistics/pearsoncorrelation.h>
#include <modules/visualneuro/statistics/spearmancorrelation.h>
#include <modules/visualneuro/statistics/ttest.h>


namespace inviwo {

const std::vector<double> A = {
    2.0000000000000000, 4.0000000000000000, 5.0000000000000000, 7.0000000000000000,
    8.0000000000000000, 9.0000000000000000, 11.000000000000000, 13.000000000000000,
    16.000000000000000, 19.000000000000000, 22.000000000000000, 23.000000000000000,
    24.000000000000000, 25.000000000000000, 29.000000000000000, 30.000000000000000,
    31.000000000000000, 33.000000000000000, 38.000000000000000, 41.000000000000000};

const std::vector<double> B = {
    59.872665405273438,  57.483406066894531, 75.959632873535156, 29.893421173095703,
    73.312240600585938,  39.305175781250000, 40.337642669677734, 55.143035888671875,
    63.770393371582031,  60.329902648925781, 85.213577270507813, 39.686199188232422,
    0.00000000000000000, 40.500068664550781, 79.381027221679688, 37.810451507568359,
    50.466075897216797,  70.091110229492188, 60.939647674560547, 60.370235443115234};

// These two has been calculated with tools other than Inviwo
// source: https://www.wessa.net/rwasp_correlation.wasp
const double correctCorrelation = 0.0118887850055591;
// source: https://planetcalc.com/7857/
const double correctT = -6.64561333506045;

const double maximumError = 0.000000001;

// Using EXPECT_NEAR instead of EXPECT_DOUBLE_EQ in all tests since the reference values have fewer
// decimals than the calculated values.
TEST(pearsonCorrelation, correlationIsCorrect) {

    double correlation = pearsonCorrelation(A, B);

    EXPECT_NEAR(correlation, correctCorrelation, maximumError)
        << "Pearson Correlation is not correct.";
}

TEST(pearsonCorrelation, correlationIsCorrectReverse) {

    double correlation = pearsonCorrelation(B, A);

    EXPECT_NEAR(correlation, correctCorrelation, maximumError)
        << "Pearson Correlation (reversed) is not correct.";
}

TEST(tTest, tTestIsCorrect) {

    double t = stats::tTest(A, B);

    EXPECT_NEAR(t, correctT, maximumError) << "T-test is not correct.";
}

TEST(tTest, tTestIsCorrectReverse) {

    double t = stats::tTest(B, A);

    EXPECT_NEAR(t, -correctT, maximumError) << "T-test (reversed) is not correct.";
}

TEST(Rank, rankEqualValues) {
    std::vector<int> a = {20, 30, 10};
    std::vector<double> aExpect = {2.0, 3.0, 1.0};
    auto aRank = inviwo::stats::rank(a);
    EXPECT_EQ(aRank, aExpect) << "Rank is not correct.";

    std::vector<int> b = {10, 12, 15, 12, 10, 25, 12};
    std::vector<double> bExpect = {1.5, 4.0, 6.0, 4.0, 1.5, 7.0, 4.0};
    auto bRank = inviwo::stats::rank(b);
    EXPECT_EQ(bRank, bExpect) << "Rank with repeated elements is not correct.";
}

TEST(spearman, spearmanIsCorrect) {
    // https://en.wikipedia.org/wiki/Spearman%27s_rank_correlation_coefficient
    // IQ
    std::vector<double> x = {106, 100, 86, 101, 99, 103, 97, 113, 112, 110};
    // Hours of TV
    std::vector<double> y = {7, 27, 2, 50, 28, 29, 20, 12, 6, 17};
    auto [corr, p] = corrTest(x, y, stats::CorrelationMethod::Spearman, stats::TailTest::TwoTailed);
    EXPECT_DOUBLE_EQ(corr, -29.0 / 165.0) << "Spearman is not correct.";

    auto pExpected = 0.62718834477648455;
    EXPECT_DOUBLE_EQ(p, pExpected) << "Spearman p-value is not correct.";
}

}  // namespace inviwo
