/*
 * @file   Polytope.h
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 */

#pragma once

#ifndef INCL_FROM_GOHEADER
	static_assert(false, "This file may only be included indirectly by GeometricObject.h");
#endif

#include "../../flags.h"

#ifdef HYPRO_USE_PPL
#include "util.h"
#include "../GeometricObject.h"
#include "../Polytopes/Cone.h"
#include "../Polytopes/Fan.h"
#include "../../datastructures/Point.h"
#include "../../datastructures/Facet.h"
#include "../../datastructures/Ridge.h"
#include "../../datastructures/Halfspace.h"
#include "../../util/convexHull.h"
#include "../../util/VariablePool.h"
#include <map>
#include <cassert>
#include <string>
#include <sstream>
#include <gmp.h>
CLANG_WARNING_DISABLE("-Wunused-local-typedef")
#include <ppl.hh>
CLANG_WARNING_RESET
#include <carl/core/Variable.h>


using namespace Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;

namespace hypro
{

	/**
	 * @brief      A wrapper class encapsulating PPL-polytopes.
	 *
	 * @tparam     Number     The used number type.
	 * @tparam     Converter  The used converter.
	 */
	template<typename Number, typename Converter>
	class PolytopeT : public GeometricObject<Number, PolytopeT<Number,Converter>>
	{
	private:
		C_Polyhedron mPolyhedron;
		mutable std::vector<Point<Number>> mPoints;
		mutable bool mPointsUpToDate;
		Fan<Number> mFan;

	public:
		PolytopeT();
		PolytopeT(const PolytopeT& orig);
		PolytopeT(unsigned dimension);
		PolytopeT(const Point<Number>& point);
		PolytopeT(const typename std::vector<Point<Number>>& points);
		PolytopeT(const typename std::vector<vector_t<Number>>& points);
		PolytopeT(const matrix_t<Number>& A, const vector_t<Number>& b);
		PolytopeT(const matrix_t<Number>& A);
		PolytopeT(const C_Polyhedron& _rawPoly);

		virtual ~PolytopeT();

		/**
		 * Getters and Setters
		 */

		static representation_name type() {	return representation_name::ppl_polytope; }

		/**
		 * Returns true, if the generatorset is empty.
		 * @return
		 */
		bool empty() const;

		/**
		 * Adds a point to the polytope. Note that if the point lies inside the
		 * current polytope, this has no effect.
		 * @param point
		 */
		void addPoint(const Point<Number>& point);


		void updatePoints() const;

		/**initConvexHull
		 * Returns the set of points which form the polytope.
		 * @return Pointset.
		 */
		std::vector<Point<Number>> vertices( const Location<Number>* = nullptr ) const;

		/*
		 * returns the fan of the Polytope
		 */
		const Fan<Number>& fan();

		/*
		 * returns a reference to the fan of the Polytope
		 */
		Fan<Number>& rFan();

		/*
		 * sets the fan of the Polytope
		 */
		void setFan(const Fan<Number>& _fan);

		void calculateFan();

		/**
		 * Prints the polytopes' generators obtained from the PPL to stdout.
		 */
		void print() const;

		/*
		 *
		 */
		friend std::ostream& operator<<(std::ostream& lhs, const PolytopeT<Number,Converter>& rhs)
		{
			using Parma_Polyhedra_Library::IO_Operators::operator<<;
			lhs << "[";
			lhs << rhs.rawPolyhedron();
			lhs << "]" << std::endl;
			return lhs;
		}

		/**
		 * PPL related functions
		 */
		const C_Polyhedron& rawPolyhedron() const;

		/**
		 * Geometric Object interface
		 */
		std::size_t dimension() const;
		PolytopeT<Number,Converter> linearTransformation(const matrix_t<Number>& A) const;
		PolytopeT<Number,Converter> affineTransformation(const matrix_t<Number>& A, const vector_t<Number>& b) const;
		PolytopeT<Number,Converter> project(const std::vector<unsigned>& dimensions) const;
		PolytopeT<Number,Converter> minkowskiSum(const PolytopeT<Number,Converter>& rhs) const;
		// implemented according to Komei Fukuda 2004
		PolytopeT<Number,Converter> altMinkowskiSum(PolytopeT<Number,Converter>& rhs);
		PolytopeT<Number,Converter> intersect(const PolytopeT<Number,Converter>& rhs) const;
		PolytopeT<Number,Converter> intersectHalfspace(const Halfspace<Number>& rhs) const;
		PolytopeT<Number,Converter> intersectHalfspaces(const matrix_t<Number>& _mat, const vector_t<Number>& _vec) const;
		std::pair<bool, PolytopeT<Number,Converter>> satisfiesHalfspace(const Halfspace<Number>& rhs) const;
		std::pair<bool, PolytopeT<Number,Converter>> satisfiesHalfspaces(const matrix_t<Number>& _mat, const vector_t<Number>& _vec) const;
		PolytopeT<Number,Converter> hull() const;
		bool contains(const Point<Number>& point) const;
		bool contains(const PolytopeT<Number,Converter>& poly) const;
		PolytopeT<Number,Converter> unite(const PolytopeT<Number,Converter>& rhs) const;
		static PolytopeT<Number,Converter> unite(const PolytopeT<Number,Converter>& polytopes);
	  //  static std::vector<Facet<Number>> convexHull(const std::vector<Point<Number>> points);


		/**
		 * Polytope related
		 */
		Number supremum () const;

		void removeRedundancy() {}


		//PolytopeT<Number,Converter>& operator= (const PolytopeT<Number,Converter>& rhs);

		/**
		 * Auxiliary functions
		 */
		int computeMaxVDegree();
		Point<Number> computeMaxPoint();
		Point<Number> computeInitVertex(PolytopeT<Number,Converter> _secondPoly);
		Point<Number> localSearch(Point<Number>& _vertex,  Point<Number>& _sinkMaximizerTarget);

	};
} // namespace

#include "Polytope.tpp"

#endif
