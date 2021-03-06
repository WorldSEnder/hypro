/**
 * Contains utilities for conversion functions. (Mainly underapproximation)
 * @file util.h
 * @author Simon Froitzheim
 *
 * @since	2016-03-17
 * @version	2016-03-17
 */

#pragma once
#include "../../datastructures/Point.h"
#include "../../datastructures/Halfspace.h"
#include "../SupportFunction/SupportFunction.h"
#include "../../util/linearOptimization/EvaluationResult.h"

namespace hypro {
/**
 * Is the caller function for the recursive method that computes exactly one boundary point for each direction that it gets (via support function)
 */

template <typename Number, typename Converter>
std::vector<Point<Number>> computeBoundaryPointsExpensive (const SupportFunctionT<Number,Converter>& sf, const matrix_t<Number>& directions) {
 //determines how many directions need to be checked
    unsigned numberOfDirections = directions.rows();
    //gets dimension in which is currently computed
    unsigned dim = directions.cols();
    //only continue if directions and object match dimensionwise
    assert (dim == sf.dimension());
    //generates an empty PointVector for the return value
    std::vector<Point<Number>> res;
    //if the function has an object that is not yet certainly a singleton (i.e. dimension is greater than zero)
    if (dim > 0){
        //generates an empty PointVector for the return values of the recursive calls
        std::vector<Point<Number>> recursiveSolutions;
        for(unsigned i=0; i<numberOfDirections; ++i){
            //determines current evaluation direction
            vector_t<Number> curNormal = directions.row(i);

            //lets the support function evaluate the offset of the halfspace for the current direction
            EvaluationResult<Number> offset = sf.evaluate(curNormal);
            //if result is infinity, try the next direction
            if(offset.errorCode == SOLUTION::INFTY){
                continue;
            }
            Number constant = offset.supportValue;

            //creates a Halfspacevector containing only the recently created Halfspace
            std::vector<Halfspace<Number>> curPlaneVector;
            curPlaneVector.emplace_back(curNormal, constant);
            curPlaneVector.emplace_back(-curNormal, -constant);
            assert(curPlaneVector.size() == 2);

            //intersects the current support function with the Halfspace
            SupportFunctionT<Number,Converter> curPlaneSup = SupportFunctionT<Number,Converter>(curPlaneVector);
            std::cout << "current supportplane:" << std::endl;
            curPlaneSup.print();
            SupportFunctionT<Number,Converter> curFace = sf.intersect(curPlaneSup);
            std::cout << "current face:" << std::endl;
            curFace.print();
            //only continue if face has still the same dimension as the source object (although it is technically now a dim-1 object at most)
            assert(curFace.dimension() == dim);

            //call of the recursive sub-function for the current face
            recursiveSolutions.push_back(computeBoundaryPointsExpensiveRecursive(curFace, directions, dim-1));
            }
        //removes duplicate points in order to enable the arithmetic mean to yield best possible results
        recursiveSolutions = Point<Number>::removeDuplicatePoints(recursiveSolutions);
        res = recursiveSolutions;
   //kickoff function gets called with a point for some reason
   } else { // dim == 0
        //evaluates the object in the first direction (any direction produces the same result)
        EvaluationResult<Number> point = sf.evaluate(directions.row(1));
        //there needs to be a result here, otherwise something went terribly wrong
        assert(point.errorCode != INFEAS && point.errorCode != UNKNOWN);
        res.emplace_back(point.optimumValue);
        assert(sf.contains(Point<Number>(point.optimumValue)));
   }
   return res;
}

    /*
     *Recursively computes some boundary points that lie relatively central for each face of the object, this function is constructed to only be called by computeBoundaryPoints
     */
template <typename Number, typename Converter>
Point<Number> computeBoundaryPointsExpensiveRecursive (const SupportFunctionT<Number,Converter>& sf, const matrix_t<Number>& directions, const unsigned curDim) {
    //determines how many directions need to be checked
    unsigned numberOfDirections = directions.rows();
    //only continue if directions and object match dimensionwise
    assert (unsigned(directions.cols()) == sf.dimension());
    //generates an empty Point for the return value
    Point<Number> res;
    //if the function has an object that is not yet certainly a singleton (i.e. dimension is greater than zero)
    if (curDim > 0){
        //generates an empty PointVector for the return values of the recursive calls
        std::vector<Point<Number>> recursiveSolutions;
        for(unsigned i=0; i<numberOfDirections; ++i){
            //determines current evaluation direction
            vector_t<Number> curNormal = directions.row(i);

            //lets the support function evaluate the offset of the halfspace for the current direction
            EvaluationResult<Number> offset = sf.evaluate(curNormal);
            //if result is infinity, try the next direction
            if(offset.errorCode == SOLUTION::INFTY){
                continue;
            }
            Number constant = offset.supportValue;

            //creates a Halfspacevector containing only the recently created Halfspace
            std::vector<Halfspace<Number>> curPlaneVector;
            curPlaneVector.emplace_back(curNormal, constant);
            curPlaneVector.emplace_back(-curNormal, -constant);
            assert(curPlaneVector.size() == 2);

            //intersects the current support function with the Halfspace
            SupportFunctionT<Number,Converter> curPlaneSup = SupportFunctionT<Number,Converter>(curPlaneVector);
            std::cout << "current supportplane:" << std::endl;
            curPlaneSup.print();
            SupportFunctionT<Number,Converter> curFace = sf.intersect(curPlaneSup);
            std::cout << "current face:" << std::endl;
            curFace.print();
            //only continue if face has still the same dimension as the source object (although it is technically now a dim-1 object at most)
            assert(curFace.dimension() == unsigned(directions.cols()));

            //recursive call of this function for the current face
            recursiveSolutions.push_back(computeBoundaryPointsExpensiveRecursive(curFace, directions, curDim-1));
            assert(recursiveSolutions.back().rawCoordinates().rows() > 0);

        }
        //removes duplicate points in order to enable the arithmetic mean to yield best possible results
        recursiveSolutions = Point<Number>::removeDuplicatePoints(recursiveSolutions);
        assert(!recursiveSolutions.empty());

        //computes the arithmetic mean as an approximation of the centroid
        res = Point<Number>(computeArithmeticMeanPoint(recursiveSolutions));
        std::cout << "merged point:" << res << std::endl;
   //call has only a point as source object (deepest recursion layer)
   } else { // curDim == 0
        //evaluates the object in the first direction (any direction produces the same result)
        EvaluationResult<Number> point = sf.evaluate(directions.row(1));
        //there needs to be a result here, otherwise something went terribly wrong
        assert(point.errorCode != INFEAS && point.errorCode != UNKNOWN);
        res = Point<Number>(point.optimumValue);
        assert(sf.contains(point.optimumValue));
        std::cout << "deepest floor point:" << point.optimumValue << std::endl;
   }
   return res;
}

/*
 *Computes the arithmetic mean for a given Point Vector
 */

template <typename Number>
vector_t<Number> computeArithmeticMeanPoint(const std::vector<Point<Number>>& pointVec){
    //defines an empty solution vector
    if(!pointVec.empty()){
    	vector_t<Number> res = vector_t<Number>::Zero(pointVec.at(0).rawCoordinates().rows());

		//computes the arithmetic mean by first building the sum of all points and then dividing it by the number of points
		for (unsigned i=0; i < pointVec.size(); ++i){
			assert(res.rows() == pointVec.at(i).rawCoordinates().rows());
			res += pointVec.at(i).rawCoordinates();
		}
		res = res*( ((Number) 1)/pointVec.size());
		return res;
    }
    return vector_t<Number>::Zero(0);
}

} // namespace hypro
