/* 
 * File:   Utility.h
 * Author: jongan
 *
 * Created on October 27, 2014, 11:53 AM
 */

#ifndef UTILITY_H
#define	UTILITY_H

#include <eigen3/Eigen/Dense>
#include <cmath>
#include "../../config.h"

namespace ZUtility {
// Options for checking for intersect
    typedef enum {
        ALAMO = 1,
        NDPROJECTION = 2,
        DICHOTOMIC2D = 3
    } IntersectionMethod_t;

    // Structure for Options used in running reachability analysis
    struct Options {
        IntersectionMethod_t intersectMethod;
        int testCase;
    };

    template<typename Number>
    bool compareVectors(const hypro::vector_t<Number>& v1, const hypro::vector_t<Number>& v2) 
    {
        Number v1_sum = v1.array().abs().matrix().sum();
        Number v2_sum = v2.array().abs().matrix().sum();
        Number v1_inf = v1.array().abs().maxCoeff();
        Number v2_inf = v2.array().abs().maxCoeff();

        return (v1_sum-v1_inf) < (v2_sum-v2_inf);
    }




    template <typename Number>
    bool compareColumnVectors(const hypro::vector_t<Number>& colvec1, 
                              const hypro::vector_t<Number>& colvec2) 
    {
        Number x1(colvec1(0)), x2(colvec2(0)), y1(colvec1(1)), y2(colvec2(1));

        Number ang1 = x1/sqrt(x1*x1 + y1*y1);
        Number ang2 = x2/sqrt(x2*x2 + y2*y2);

        return (ang1 < ang2);
    }

    template <typename Number> 
    bool compareYVal(const hypro::vector_t<Number>& colvec1, 
                     const hypro::vector_t<Number>& colvec2)
    {
        return (colvec1(1) < colvec2(1));
    }

    template<typename Number>
    bool compareXVal(const hypro::vector_t<Number>& colvec1, 
                     const hypro::vector_t<Number>& colvec2)
    {
        return (colvec1(0) < colvec2(0));
    }


    template <typename Number>
    struct Line_t {
        Line_t() {};
        Line_t(Eigen::Matrix<Number, 2,1> pt_,
             Eigen::Matrix<Number, 2,1> direction_) 
        {
            point = pt_;
            direction = direction_;
        }

        Eigen::Matrix<Number, 2,1> point;
        Eigen::Matrix<Number, 2,1> direction;
    };

}


#endif	/* UTILITY_H */

