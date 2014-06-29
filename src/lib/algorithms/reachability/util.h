
#pragma once

//#include "../../util/eigenTypetraits.h"
#include "../../datastructures/hybridAutomata/util.h"
#include "../../representations/Box/Box.h"
#include "carl/core/VariablePool.h"

namespace hypro
{
		/*
		enum operator_e {
			EQ,
			GEQ,
			LEQ,
			NEQ,
			LESS,
			GREATER
		};
		*/

		//typedef std::map<carl::Variable, carl::FLOAT_T<Number> > vector_t;
		//typedef Eigen::Matrix<carl::FLOAT_T<Number>, Eigen::Dynamic, Eigen::Dynamic> matrix_t;
		//typedef std::map<carl::Variable, operator_e> operator_t;
		//typedef std::map<carl::Variable, carl::FLOAT_T<Number>> valuation_t;
		//typedef std::map<carl::Variable, carl::Interval<FLOAT_T<Number>>> valuation_t; //a valuation is simply always handled as an interval

		/**
		 * General purpose functionality
		 */
		template<typename Number>
		bool compare(carl::FLOAT_T<Number> _lhs, carl::FLOAT_T<Number> _rhs, hypro::operator_e _op) {
			bool result = false;

			switch (_op) {
			case EQ:
				if (_lhs == _rhs) {result = true;}
				break;
			case NEQ:
				if (_lhs != _rhs) {result = true;}
				break;
			case GEQ:
				if (_lhs >= _rhs) {result = true;}
				break;
			case LEQ:
				if (_lhs <= _rhs) {result = true;}
				break;
			case LESS:
				if (_lhs < _rhs) {result = true;}
				break;
			case GREATER:
				if (_lhs > _rhs) {result = true;}
				break;
			default:
				result = false;
			}
			return result;
		}

		/**
		 * Functionality in context of a Location
		 */
		/*
		template<typename Number>
		bool checkInvariant(hypro::Location _loc, valuation_t _val) {
			//return mInvariant.mat * _val mInvariant.op mInvariant.vec
			//shorter? for (auto varIt : _val)

			auto vecIt = _loc.mInvariant.vec.begin();

			for (int currentRow=0; currentRow != _loc.mInvariant.mat.rows(); ++currentRow) {  //iterate over all matrix rows
				carl::FLOAT_T<Number> upperSum = 0;										// one matrix row * valuation vector = sum
				carl::FLOAT_T<Number> lowerSum = 0;
				int currentColumn = 0;

				for (auto varIt=_val.begin(); varIt !=_val.end(); ++varIt) {		// for each row iterate over all variables
					carl::FLOAT_T<Number> upperBound = varIt->second.upper();       // upper & lower bounds of the valuation interval for one variable
					carl::FLOAT_T<Number> lowerBound = varIt->second.lower();

					upperSum += _loc.mInvariant.mat(currentRow,currentColumn) * upperBound;   //add (matrix entry * var valuation) to the sums
					lowerSum += _loc.mInvariant.mat(currentRow,currentColumn) * lowerBound;

					currentColumn++; 												//next variable corresponds to next column
				}
				bool tempResult_up = compare(upperSum,vecIt->second,_loc.mInvariant.op); // compare upperSum with the value stored in the vector with respect to the operator
				bool tempResult_low = compare(lowerSum,vecIt->second,_loc.mInvariant.op);  //TODO can the vector contain intervals?

				if (!tempResult_up || !tempResult_low) {return false;}               //return false if a boundary is ever violated TODO possibly add information
			}
			return true;   // a boundary was never violated
		}
		*/

		/**
		 * Functionality in context of a Transition
		 */
		/*
		template<typename Number>
		bool checkGuard(hypro::Transition _trans, valuation_t _val) {
			//return mGuard.mat * _val mGuard.op mGuard.vec

			auto vecIt = _trans.mGuard.vec.begin();

			for (int currentRow=0; currentRow != _trans.mGuard.mat.rows(); ++currentRow) {  //iterate over all matrix rows
				carl::FLOAT_T<Number> upperSum = 0;										// one matrix row * valuation vector = sum
				carl::FLOAT_T<Number> lowerSum = 0;
				int currentColumn = 0;

				for (auto varIt=_val.begin(); varIt !=_val.end(); ++varIt) {		// for each row iterate over all variables
					carl::FLOAT_T<Number> upperBound = varIt->second.upper();       // upper & lower bounds of the valuation interval for one variable
					carl::FLOAT_T<Number> lowerBound = varIt->second.lower();

					upperSum += _trans.mGuard.mat(currentRow,currentColumn) * upperBound;   //add (matrix entry * var valuation) to the sums
					lowerSum += _trans.mGuard.mat(currentRow,currentColumn) * lowerBound;

					currentColumn++; 												//next variable corresponds to next column
				}
				bool tempResult_up = compare(upperSum,vecIt->second,_trans.mGuard.op); // compare upperSum with the value stored in the vector with respect to the operator
				bool tempResult_low = compare(lowerSum,vecIt->second,_trans.mGuard.op);  //TODO can the vector contain intervals?

				if (!tempResult_up || !tempResult_low) {return false;}               //return false if a boundary is ever violated TODO possibly add information
			}
			return true;   // a boundary was never violated
		}
		*/

		/**
		 * Functionality for the Flowpipe
		 */

		template<typename Number>
		hypro::Box<Number> computeBox(unsigned int _dim, Number _radius) {
			hypro::Box<Number> box = new hypro::Box<Number>();
		    carl::VariablePool& pool = carl::VariablePool::getInstance();
	        std::map<const carl::Variable, carl::Interval<Number> > boundaries;

			for (int i=0; i<_dim; ++i) {
				carl::Variable x = pool.getFreshVariable(std::to_string(i));
				boundaries.insert( std::make_pair(x, carl::Interval<Number>(-_radius,_radius)) );
			}
			box.insert(boundaries);
			return box;
		}

		template<typename Number>
		hypro::valuation_t<Number> computePolytope(unsigned int _dim, Number _radius) {
			hypro::matrix_t<Number> mat = hypro::matrix_t<Number>(2*_dim,_dim);
			hypro::vector_t<Number> vec;
			//carl::VariablePool& pool = carl::VariablePool::getInstance();

			for (int i=0; i<2*_dim; ++i) {
				//carl::Variable x = pool.getFreshVariable(std::to_string(i));
		    	//vec.insert( std::make_pair(x, _radius) );
				vec(i) = _radius;
			}

			hypro::valuation_t<Number> poly = hypro::Polytope<Number>(mat,vec);
			return poly;
		}
}

