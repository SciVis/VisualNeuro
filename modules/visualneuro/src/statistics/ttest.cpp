#include <modules/visualneuro/statistics/ttest.h>

namespace inviwo {

namespace stats {

double student_t_cdf(double t, double df) {
    double x = (t + sqrt(t * t + df)) / (2.0 * sqrt(t * t + df));
    double prob = incbeta(df / 2.0, df / 2.0, x);
    return prob;
}

double incbeta(double a, double b, double x) {
    if (x < 0.0 || x > 1.0) return 1.0;  // 1.0 / 0.0;

    if (x > (a + 1.0) / (a + b + 2.0)) {
        return (1.0 - incbeta(b, a, 1.0 - x));
    }

    const double lbeta_ab = lgamma(a) + lgamma(b) - lgamma(a + b);
    const double front = exp(log(x) * a + log(1.0 - x) * b - lbeta_ab) / a;

    double f = 1.0, c = 1.0, d = 0.0;

    int i, m;
    for (i = 0; i <= 200; ++i) {
        m = i / 2;

        double numerator;
        if (i == 0) {
            numerator = 1.0;
        } else if (i % 2 == 0) {
            numerator = (m * (b - m) * x) / ((a + 2.0 * m - 1.0) * (a + 2.0 * m));
        } else {
            numerator = -((a + m) * (a + b + m) * x) / ((a + 2.0 * m) * (a + 2.0 * m + 1));
        }

        d = 1.0 + numerator * d;
        if (fabs(d) < 1.0e-30) d = 1.0e-30;
        d = 1.0 / d;

        c = 1.0 + numerator / c;
        if (fabs(c) < 1.0e-30) c = 1.0e-30;

        const double cd = c * d;
        f *= cd;

        if (fabs(1.0 - cd) < 1.0e-8) {
            return front * (f - 1.0);
        }
    }

    return 1.0;
}

double tailTest(const double t, double degreesOfFreedom, TailTest tailTest) {
    if (isnan(t)) return 1.0;

    double p;
    // Two-tailed test
    if (tailTest == TailTest::Both) {
        p = 2.0 * student_t_cdf(-abs(t), degreesOfFreedom);
        // Numerical recipies simplification does not pass the tests...
        // p = incbeta(df / 2.0, df / 2.0, df / (df + t * t));
    }
    // Right one-tailed test
    else if (tailTest == TailTest::Less) {
        p = student_t_cdf(-t, degreesOfFreedom);
    }
    // Left one-tailed test
    else {
        p = student_t_cdf(t, degreesOfFreedom);
    }

    return p;
}

}  // namespace stats

}  // namespace inviwo
