/*
 * @file   VPolytope.tpp
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since   2014-02-25
 * @version 2014-02-25
 */

#include "VPolytope.h"

namespace hypro {
template <typename Number, typename Converter>
VPolytopeT<Number, Converter>::VPolytopeT()
	: mVertices(), mReduced( true ), mNeighbors() {
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter>::VPolytopeT( const Point<Number> &point ) {
	mVertices.push_back( point );
	mReduced = true;
	mNeighbors.push_back( std::set<unsigned>() );
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter>::VPolytopeT( const pointVector &points ) {
	for ( const auto point : points ) {
		if(std::find(mVertices.begin(), mVertices.end(), point) == mVertices.end()) {
			mVertices.push_back( point );
			mNeighbors.push_back( std::set<unsigned>() );
		}
	}
	mReduced = false;
	reduceNumberRepresentation();
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter>::VPolytopeT( const std::vector<vector_t<Number>>& rawPoints ) {
	for ( const auto point : rawPoints ) {
		Point<Number> tmpPoint(point);
		if(std::find(mVertices.begin(), mVertices.end(), tmpPoint ) == mVertices.end()) {
			mVertices.push_back( tmpPoint );
			mNeighbors.push_back( std::set<unsigned>() );
		}
	}
	mReduced = false;
	reduceNumberRepresentation();
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter>::VPolytopeT( const matrix_t<Number> &_constraints, const vector_t<Number> _constants ) {
	// calculate all possible Halfspace intersections

	//std::cout << __func__ << ": matrix: " << _constraints << " and vector: " << _constants << std::endl;
	assert(_constraints.rows() == _constants.rows());
	Permutator permutator = Permutator( _constraints.rows(), _constraints.cols() );
	matrix_t<Number> intersection = matrix_t<Number>( _constraints.cols(), _constraints.cols() );
	vector_t<Number> intersectionConstants = vector_t<Number>( _constraints.cols() );
	std::set<vector_t<Number>> possibleVertices;
	std::vector<unsigned> permutation;
	while ( !permutator.end()  ) {
		permutation = permutator();
		unsigned rowCount = 0;
		// std::cout << "Intersect :" << std::endl;
		for ( const auto &rowIndex : permutation ) {
			// std::cout << _constraints.row(rowIndex) << " <= " <<
			// _constants(rowIndex) << std::endl;
			assert(rowCount < _constraints.cols());
			intersection.row( rowCount ) = _constraints.row( rowIndex );
			intersectionConstants( rowCount ) = _constants( rowIndex );
			++rowCount;
		}
		// check if rank is full
		if ( intersection.fullPivLu().rank() == intersection.cols() ) {
			vector_t<Number> vertex = intersection.fullPivLu().solve( intersectionConstants );
			assert(vertex.rows() == _constraints.cols());
			assert(insidePlanes(vertex, intersection, intersectionConstants));
			// avoid duplicate entries
			if(std::find(possibleVertices.begin(), possibleVertices.end(),vertex) == possibleVertices.end()) {
				possibleVertices.emplace( std::move(vertex) );
			}
		}
	}
	TRACE("hypro.representations.vpolytope",": Computed " << possibleVertices.size() << " possible vertices.");

	// check if vertices are true vertices (i.e. they fulfill all constraints)
	for ( auto vertex = possibleVertices.begin(); vertex != possibleVertices.end(); ) {
		// std::cout<<__func__ << " : " <<__LINE__ << " current position : " << i <<
		// std::endl;
		// std::cout<<__func__ << " : " <<__LINE__ << "number of vertices : " <<
		// possibleVertices.size() << std::endl;
		bool deleted = false;
		for ( unsigned rowIndex = 0; rowIndex < _constraints.rows(); ++rowIndex ) {
			Number res = vertex->dot( _constraints.row( rowIndex ) );
			if ( !carl::AlmostEqual2sComplement(res, _constants( rowIndex ), 128) && res > _constants( rowIndex ) ){
				vertex = possibleVertices.erase( vertex );
				deleted = true;
				break;
			}
		}
		if(!deleted){
			++vertex;
		}
	}

	TRACE("hypro.representations.vpolytope","Deleted vertices. Remaining " << possibleVertices.size() << " vertices.");

	// std::cout<<__func__ << " : " <<__LINE__ <<std::endl;
	// finish initialization
	for ( const auto &point : possibleVertices ) {
		mVertices.emplace_back( point );
		mNeighbors.push_back( std::set<unsigned>() );
		// std::cout << "Real vertex " << point.transpose() << std::endl;
	}
	// std::cout<<__func__ << " : " <<__LINE__ <<std::endl;
	mReduced = false;

	//reduceNumberRepresentation();
}

template<typename Number, typename Converter>
VPolytopeT<Number,Converter> VPolytopeT<Number,Converter>::project(const std::vector<unsigned>& dimensions) const {
	if(dimensions.empty()) {
		return Empty();
	}

	std::vector<Point<Number>> projectedVertices;
	for(const auto& vertex : mVertices) {
		projectedVertices.emplace_back(vertex.project(dimensions));
	}

	return VPolytopeT<Number,Converter>(projectedVertices);
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::linearTransformation( const matrix_t<Number> &A ) const {
	// std::cout << __func__ << " A: " << A << ", b: " << b << std::endl;
	VPolytopeT<Number, Converter> result;
	for ( const auto &vertex : mVertices ) {
		Point<Number> tmp(vertex.linearTransformation( A ));
		if(std::find(result.begin(), result.end(), tmp) == result.end()) {
			result.emplace_back( std::move(tmp) );
		}
	}
	result.setCone( mCone.linearTransformation( A ) );
	result.unsafeSetNeighbors( mNeighbors );
	return result;
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::affineTransformation( const matrix_t<Number> &A,
														   const vector_t<Number> &b ) const {
	// std::cout << __func__ << " A: " << A << ", b: " << b << std::endl;
	VPolytopeT<Number, Converter> result;
	for ( const auto &vertex : mVertices ) {
		Point<Number> tmp(vertex.affineTransformation( A, b ));
		if(std::find(result.begin(), result.end(), tmp) == result.end()) {
			result.emplace_back( std::move(tmp) );
		}
	}
	result.setCone( mCone.affineTransformation( A, b ) );
	result.unsafeSetNeighbors( mNeighbors );
	return result;
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::minkowskiSum( const VPolytopeT<Number, Converter> &rhs ) const {
	VPolytopeT<Number, Converter> result;
	// add each rhs-vertex to each vertex of this polytope.
	for ( auto lhsVertex : mVertices ) {
		for ( auto rhsVertex : rhs.mVertices ) {
			result.insert( lhsVertex + rhsVertex );
		}
	}
	result.setCone( mCone.minkowskiSum( rhs.cone() ) );
	return result;
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::intersect( const VPolytopeT<Number, Converter> &rhs ) const {
	// create a set of possible points via combination of all coordinates
	if ( rhs.size() == 0 ) {
		return VPolytopeT<Number, Converter>();
	} else {
		pointVector possibleVertices;
		for ( const auto &lhsVertex : mVertices ) {
			possibleVertices.push_back( lhsVertex );
			for ( unsigned coordIndex = 0; coordIndex < lhsVertex.rawCoordinates().rows(); ++coordIndex ) {
				for ( const auto &rhsVertex : rhs.mVertices ) {
					vector_t<Number> newVertex = rhsVertex.rawCoordinates();
					newVertex( coordIndex ) = lhsVertex.at( coordIndex );
					possibleVertices.push_back( Point<Number>(newVertex) );
					possibleVertices.push_back( Point<Number>(rhsVertex.rawCoordinates()) );
				}
			}
		}
		// remove all vertices, which are only contained in one of the polytopes
		for ( auto vertexIt = possibleVertices.begin(); vertexIt != possibleVertices.end(); ) {
			if ( !( this->contains( *vertexIt ) && rhs.contains( *vertexIt ) ) ) {
				vertexIt = possibleVertices.erase( vertexIt );
			} else {
				++vertexIt;
			}
		}
		return VPolytopeT<Number, Converter>( std::move(possibleVertices) );
	}
}

template<typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::intersectHalfspace( const Halfspace<Number>& rhs ) const {
	auto intermediate = Converter::toHPolytope(*this);
	auto intersection = intermediate.intersectHalfspace(rhs);
	//intersection.removeRedundancy();
	VPolytopeT<Number, Converter> res(Converter::toVPolytope(intersection));
	return res;
}

template<typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::intersectHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const {
	auto intermediate = Converter::toHPolytope(*this);
	auto intersection = intermediate.intersectHalfspaces(_mat, _vec);
	//intersection.removeRedundancy();
	VPolytopeT<Number, Converter> res(Converter::toVPolytope(intersection));
	return res;
}

template<typename Number, typename Converter>
std::pair<bool, VPolytopeT<Number, Converter>> VPolytopeT<Number, Converter>::satisfiesHalfspace( const Halfspace<Number>& rhs ) const {
	auto intermediate = Converter::toHPolytope(*this);
	auto resultPair = intermediate.satisfiesHalfspace(rhs);
	if(resultPair.first){
		//resultPair.second.removeRedundancy();
		VPolytopeT<Number, Converter> res(Converter::toVPolytope(resultPair.second));
		return std::make_pair(true, res);
	}
	return std::make_pair(false, VPolytopeT<Number,Converter>::Empty());
}

template<typename Number, typename Converter>
std::pair<bool, VPolytopeT<Number, Converter>> VPolytopeT<Number, Converter>::satisfiesHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const {
	//std::cout << typeid(*this).name() << "::" << __func__ << ": Matrix: " << _mat << " and vector: " << _vec << std::endl;
	//std::cout << "This VPolytope: " << *this << std::endl;
	auto intermediate = Converter::toHPolytope(*this);
	intermediate.reduceNumberRepresentation(mVertices);
	//std::cout << typeid(*this).name() << "::" << __func__ << ": Intermediate hpoly: " << intermediate << std::endl;
	auto resultPair = intermediate.satisfiesHalfspaces(_mat, _vec);
	if(resultPair.first){
		//resultPair.second.removeRedundancy();
		assert(!resultPair.second.empty());
		TRACE("hypro.representations.vpolytope",": Intermediate hpoly convert back: " << resultPair.second);
		VPolytopeT<Number, Converter> res(Converter::toVPolytope(resultPair.second));
		TRACE("hypro.representations.vpolytope","Re-Converted v-poly: " << res);
		assert(!res.empty());
		return std::make_pair(true, res);
	}
	return std::make_pair(false, VPolytopeT<Number,Converter>::Empty());
}


template <typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::contains( const Point<Number> &point ) const {
	return this->contains( point.rawCoordinates() );
}

template <typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::contains( const vector_t<Number> &vec ) const {
	auto tmpHPoly = Converter::toHPolytope(*this);
	return tmpHPoly.contains(vec);
}

template <typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::contains( const VPolytopeT<Number, Converter> &_other ) const {
	// std::cout << *this<< " " << __func__ << " " << _other << std::endl;
	for ( const auto &vertex : _other.vertices() ) {
		// std::cout << __func__ << " check vertex " << vertex << std::endl;
		if ( !this->contains( vertex ) ) {
			// std::cout << "false" << std::endl;
			return false;
		}
	}
	return true;
}

template <typename Number, typename Converter>
VPolytopeT<Number, Converter> VPolytopeT<Number, Converter>::unite( const VPolytopeT<Number, Converter> &rhs ) const {
	if ( rhs.dimension() == 0 ) {
		return VPolytopeT<Number, Converter>( mVertices );
	} else {
		VPolytopeT<Number,Converter> result;
		//std::cout << __func__ << " : of " << *this << " and " << rhs << std::endl;
		VPolytopeT<Number, Converter>::pointVector points;

		std::set<Point<Number>> pointSet;
		pointSet.insert( this->mVertices.begin(), this->mVertices.end() );
		pointSet.insert( rhs.mVertices.begin(), rhs.mVertices.end() );

		if(pointSet.empty()){
			return VPolytopeT<Number,Converter>();
		}

		unsigned effDim = unsigned(effectiveDimension(pointSet));
		points.insert( points.end(), pointSet.begin(), pointSet.end() );
		assert(!points.empty());
		TRACE("hypro.representations.vpolytope","Effective dimension: " << effDim << ", points dimension: " << points.begin()->dimension());

		if(effDim < points.begin()->dimension()){
			// TODO: We can optimize this here.
			return VPolytopeT<Number,Converter>(points);
		} else if(points.size() > points.begin()->dimension()){
			TRACE("hypro.representations.vpolytope","Using convex hull algorithm to reduce point set.");
			std::vector<std::shared_ptr<Facet<Number>>> facets = convexHull( points ).first;
			std::set<Point<Number>> preresult;
			for ( unsigned i = 0; i < facets.size(); i++ ) {
				for ( unsigned j = 0; j < facets[i]->vertices().size(); j++ ) {
					preresult.insert( facets[i]->vertices().at( j ) );
				}
			}
			VPolytopeT<Number, Converter>::pointVector res;
			for ( const auto &point : preresult ) {
				res.push_back( point );
			}
			result = VPolytopeT<Number, Converter>( res );
		} else {
			result = VPolytopeT<Number, Converter>(points);
		}

		//assert(result.contains(*this));
		//assert(result.contains(rhs));

		return result;
	}
}

template<typename Number, typename Converter>
VPolytopeT<Number,Converter> VPolytopeT<Number,Converter>::unite( const std::vector<VPolytopeT>& rhs ) {
	if ( rhs.size() == 0 ) {
		return VPolytopeT<Number, Converter>::Empty();
	} else {
		VPolytopeT<Number,Converter> result;
		//std::cout << __func__ << " : of " << *this << " and " << rhs << std::endl;
		VPolytopeT<Number, Converter>::pointVector points;

		std::set<Point<Number>> pointSet;
		for(const auto& poly : rhs){
			pointSet.insert( rhs.mVertices.begin(), rhs.mVertices.end() );
		}

		if(pointSet.empty()){
			return VPolytopeT<Number,Converter>();
		}

		unsigned effDim = unsigned(effectiveDimension(pointSet));
		points.insert( points.end(), pointSet.begin(), pointSet.end() );
		assert(!points.empty());

		if(effDim < points.begin()->dimension()){
			pointSet.clear();
			std::vector<Halfspace<Number>> orientedHalfspaces = computeOrientedBox(points);
			//std::cout << "Box has " << orientedHalfspaces.size() << " halfspaces in dimension " << points.begin()->dimension() << std::endl;
			//assert(orientedHalfspaces.size() == points.begin()->dimension());

			// vertex computation of the oriented box
			unsigned dim = points.begin()->dimension();
			Permutator permutator(orientedHalfspaces.size(), dim);
			std::vector<unsigned> permutation;
			while(!permutator.end()) {
				permutation = permutator();

				matrix_t<Number> A( dim, dim );
				vector_t<Number> b( dim );
				unsigned pos = 0;
				for(auto planeIt = permutation.begin(); planeIt != permutation.end(); ++planeIt) {
					A.row(pos) = orientedHalfspaces.at(*planeIt).normal().transpose();
					b(pos) = orientedHalfspaces.at(*planeIt).offset();
					++pos;
				}
				Eigen::FullPivLU<matrix_t<Number>> lu_decomp( A );
				if ( lu_decomp.rank() < A.rows() ) {
					continue;
				}

				vector_t<Number> res = lu_decomp.solve( b );

				// Check if the computed vertex is a real vertex
				bool outside = false;
				for(unsigned planePos = 0; planePos < orientedHalfspaces.size(); ++planePos) {
					bool skip = false;
					for(unsigned permPos = 0; permPos < permutation.size(); ++permPos) {
						if(planePos == permutation.at(permPos)) {
							skip = true;
							break;
						}
					}

					if(!skip) {
						if( orientedHalfspaces.at(planePos).offset() - orientedHalfspaces.at(planePos).normal().dot(res) < 0 ) {
							//std::cout << "Drop vertex: " << res << " because of plane " << planePos << std::endl;
							outside = true;
							break;
						}
					}
				}
				if(!outside) {
					pointSet.emplace(res);
				}
			}
			for ( const auto &point : pointSet ) {
				result.insert( point );
			}
		} else if(points.size() > points.begin()->dimension()){
			std::vector<std::shared_ptr<Facet<Number>>> facets = convexHull( points ).first;
			std::set<Point<Number>> preresult;
			for ( unsigned i = 0; i < facets.size(); i++ ) {
				for ( unsigned j = 0; j < facets[i]->vertices().size(); j++ ) {
					preresult.insert( facets[i]->vertices().at( j ) );
				}
			}
			VPolytopeT<Number, Converter>::pointVector res;
			for ( const auto &point : preresult ) {
				res.push_back( point );
			}
			result = VPolytopeT<Number, Converter>( res );
		} else {
			result = VPolytopeT<Number, Converter>(points);
		}
		//assert(result.contains(*this));
		return result;
	}
}

template <typename Number, typename Converter>
void VPolytopeT<Number, Converter>::clear() {
	mVertices.clear();
}

template <typename Number, typename Converter>
Number VPolytopeT<Number, Converter>::supremum() const {
	Number max = 0;
	for ( auto &point : mVertices ) {
		Number inftyNorm = Point<Number>::inftyNorm( point );
		max = max > inftyNorm ? max : inftyNorm;
	}
	return max;
}

template <typename Number, typename Converter>
void VPolytopeT<Number, Converter>::removeRedundancy() {
	if ( !mReduced ) {
		#ifdef USE_SMTRAT
			std::set<Point<Number>> toDelete;
			smtrat::SimplexSolver simplex;
			simplex.push();

			// create mapping of variables (lambdas') to vertices.
			std::map<Point<Number>, carl::Variable> lambdas;
			for(const auto& vertex : mVertices) {
				carl::Variable lambda = carl::freshRealVariable();
				lambdas.insert(std::make_pair(vertex, lambda));
				//std::cout << "Assigned " << lambdas.find(vertex)->second << " to " << lambdas.find(vertex)->first << std::endl;
			}


			for(auto currVertex = mVertices.begin(); currVertex != mVertices.end(); ){
				//std::cout << currVertex << ": " << std::endl;
				// create constraint for Sum(lambdas) == 1
				carl::MultivariatePolynomial<smtrat::Rational> sumBound;
				for(const auto& vertex : mVertices) {
					if(vertex != *currVertex) {
						carl::Variable lambda = lambdas.find(vertex)->second;
						sumBound += lambda;
						smtrat::FormulaT constr = smtrat::FormulaT(carl::MultivariatePolynomial<smtrat::Rational>(lambda), carl::Relation::GEQ);
						simplex.inform(constr);
						simplex.add(constr);
					}
				}
				sumBound -= smtrat::Rational(1);
				smtrat::FormulaT constr = smtrat::FormulaT(sumBound, carl::Relation::EQ);
				simplex.inform(constr);
				simplex.add(constr);

				for(unsigned dim = 0; dim < this->dimension(); ++dim) {
					carl::MultivariatePolynomial<smtrat::Rational> row;
					for(const auto& vertex : mVertices ) {
						if(*currVertex != vertex) {
							carl::Variable tmp = lambdas.find(vertex)->second;
							row += tmp*carl::convert<Number, smtrat::Rational>(vertex.at(dim));
						} else {
							row -= carl::convert<Number, smtrat::Rational>(vertex.at(dim));
						}
					}
					smtrat::FormulaT constr = smtrat::FormulaT(row, carl::Relation::EQ);
					//std::cout << constr << std::endl;
					simplex.inform(constr);
					simplex.add(constr);
				}

				//std::cout << "Checking: " << std::endl << simplex.formula().toString() << std::endl;

				smtrat::Answer res = simplex.check();

				if(res == smtrat::Answer::SAT) {
					currVertex = mVertices.erase(currVertex);
				} else {
					++currVertex;
				}

				//std::cout << "Is extreme point: " << (res == smtrat::Answer::UNSAT) << std::endl << std::endl << std::endl << std::endl;
				simplex.pop();
				simplex.push();
			}

			mReduced = true;
		#else
			updateNeighbors();
		#endif
	}
}

template <typename Number, typename Converter>
void VPolytopeT<Number, Converter>::updateNeighbors() {
	std::map<Point<Number>, std::set<Point<Number>>> neighbors = convexHull( mVertices ).second;
	mVertices.clear();
	for ( const auto &pointNeighborsPair : neighbors ) {
		mVertices.push_back( pointNeighborsPair.first );
	}
	// we can only set neighbors after all points have been inserted.
	for ( const auto &pointNeighborsPair : neighbors ) {
		this->setNeighbors( pointNeighborsPair.first, pointNeighborsPair.second );
	}
	mReduced = true;
}

/***************************************************************************
 * Auxiliary functions
 **************************************************************************/

template<typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::belowPlanes(const vector_t<Number>& vertex, const matrix_t<Number>& normals, const vector_t<Number>& offsets) {
	for(unsigned rowIndex = 0; rowIndex < normals.rows(); ++rowIndex){
		if(vertex.dot(normals.row(rowIndex)) > offsets(rowIndex)){
			return false;
		}
	}
	return true;
}

template<typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::abovePlanes(const vector_t<Number>& vertex, const matrix_t<Number>& normals, const vector_t<Number>& offsets) {
	for(unsigned rowIndex = 0; rowIndex < normals.rows(); ++rowIndex){
		if(vertex.dot(normals.row(rowIndex)) < offsets(rowIndex)){
			return false;
		}
	}
	return true;
}

template <typename Number, typename Converter>
bool VPolytopeT<Number, Converter>::operator==( const VPolytopeT<Number, Converter> &rhs ) const {
	if ( this->dimension() != rhs.dimension() ) return false;

	// TODO: Highly inefficient!!!
	for ( auto &lPoint : this->vertices() ) {
		if ( !rhs.hasVertex( lPoint ) ) return false;
	}
	for ( auto &rPoint : rhs.vertices() ) {
		if ( !this->hasVertex( rPoint ) ) return false;
	}
	return true;
}

}  // namespace hypro
