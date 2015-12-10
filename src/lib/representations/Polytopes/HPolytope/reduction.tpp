/**
 * File holding utility functions to keep the main files clean.
 */
#include "HPolytope.h"
namespace hypro {

  /*
   * Compute all neighbors of a facet by calculating the participation at the vertices of the facet
   * @input unsigned facet
   * @return std::vector<Number> res contains all indices of neighbors, sorted
   */
  template <typename Number>
  std::vector<std::vector<unsigned>> HPolytope<Number>::getMembersOfVertices(std::vector<Point<Number>> vertices) const {
    HPolytope<Number> hpolytope = *this;
    std::vector<std::vector<unsigned>> res;

  	for(Point<Number> vertex: vertices) {
  		vector_t<Number> vertexVector = vector_t<Number>(vertex.rawCoordinates());
  		std::vector<unsigned> subRes;

  		for(unsigned i=0; i<hpolytope.size(); i++){
  			if(carl::AlmostEqual2sComplement(vertexVector.dot(hpolytope.constraints().at(i).normal()), hpolytope.constraints().at(i).offset())){
  				if(std::find(subRes.begin(), subRes.end(), i)==subRes.end()) {
  					subRes.push_back(i);
  				}
  			}
  		}
  		std::sort(subRes.begin(), subRes.end());
  		if(!subRes.empty()) res.push_back(subRes);
  	}

  	return res;
  }

  /*
   * Compute all neighbors of a facet by calculating the participation at the vertices of the facet
   * @input unsigned facet, membersOfvertices
   * @return std::vector<unsigned> res contains all indices of neighbors, sorted
   */
  template <typename Number>
  std::vector<unsigned> HPolytope<Number>::getNeighborsOfIndex(unsigned facet, std::vector<std::vector<unsigned>> membersOfvertices) const {
  	std::vector<unsigned> res;

  	for(std::vector<unsigned> membersOfvertex: membersOfvertices) {
  		if(std::find(membersOfvertex.begin(), membersOfvertex.end(), facet)!=membersOfvertex.end()){
  			for(unsigned member: membersOfvertex){
  				if(member!=facet && std::find(res.begin(), res.end(), member)==res.end()){
  					res.push_back(member);
  				}
  			}
  		}
  	}
  	std::sort(res.begin(), res.end());
  	return res;
  }

  /*
   * Get all vertices which have one facet as member
   * @input unsigned facet a, vertices, membersOfvertices
   * @return std::vector<Point<Number>> res contains all vertices which have a as member
   */
  template <typename Number>
  std::vector<Point<Number>> HPolytope<Number>::getVerticesOfIndex(unsigned a, std::vector<Point<Number>> vertices, std::vector<std::vector<unsigned>> membersOfvertices) const {
  	std::vector<Point<Number>> res;

  	for(unsigned i=0; i<vertices.size(); i++) {
  		if(std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), a)!= membersOfvertices.at(i).end()) {
  			res.push_back(vertices.at(i));
  		}
  	}

  	return res;
  }

  /*
   * Get all vertices which have two facets as members
   * @input unsigned facet a, b, vertices, membersOfvertices
   * @return std::vector<Point<Number>> res contains all vertices which have a, b as members
   */
  template <typename Number>
  std::vector<Point<Number>> HPolytope<Number>::getVerticesOf2Indices(unsigned a, unsigned b, std::vector<Point<Number>> vertices, std::vector<std::vector<unsigned>> membersOfvertices) const {
  	std::vector<Point<Number>> res;

  	for(unsigned i=0; i<vertices.size(); i++) {
  		if(std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), a)!= membersOfvertices.at(i).end()
  		&& std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), b)!= membersOfvertices.at(i).end()) {
  			res.push_back(vertices.at(i));
  		}
  	}

  	return res;
  }

  /*
   * Get all vertices which have onyl one of two facets as members
   * @input unsigned facet a, b, vertices, membersOfvertices
   * @return std::vector<Point<Number>> res contains all vertices which have only one of a, b as members
   */
  template <typename Number>
  std::vector<Point<Number>> HPolytope<Number>::getVerticesOf2IndicesAround(unsigned a, unsigned b, std::vector<Point<Number>> vertices, std::vector<std::vector<unsigned>> membersOfvertices) const {
  	std::vector<Point<Number>> res;

  	for(unsigned i=0; i<vertices.size(); i++) {
  		if(((std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), a)!=membersOfvertices.at(i).end()) && (std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), b)==membersOfvertices.at(i).end()))
  		||
  			((std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), b)!=membersOfvertices.at(i).end()) && (std::find(membersOfvertices.at(i).begin(), membersOfvertices.at(i).end(), a)==membersOfvertices.at(i).end()))) {
  			res.push_back(vertices.at(i));
  		}
  	}

  	return res;
  }

  /*
   * Get all permutations for the ridge of the facet
   * @input unsigned facet a, b, vertices
   * @return std::vector<std::vector<Point<Number>>> permutations
   */
  template <typename Number>
  std::vector<std::vector<vector_t<Number>>> HPolytope<Number>::getVerticesPermutationForFacet(unsigned a, unsigned b, std::vector<Point<Number>> vertices) const {
    HPolytope<Number> hpolytope = *this;
    std::vector<std::vector<vector_t<Number>>> res;
  	std::vector<Point<Number>> cutPointsAround_a_b = getVerticesOf2IndicesAround(a, b, vertices, getMembersOfVertices(vertices));

  	if(cutPointsAround_a_b.size()==cutPointsAround_a_b.at(0).dimension()){
  		std::vector<vector_t<Number>> setOfPoints;
  		for(Point<Number> point: cutPointsAround_a_b){
  			setOfPoints.push_back(vector_t<Number>(point.rawCoordinates()));
  		}

  		res.push_back(setOfPoints);
  	} else {
  		polytope::dPermutator permutator(cutPointsAround_a_b.size(), cutPointsAround_a_b.at(0).dimension());
  		std::vector<unsigned> permutation;
  		while(!permutator.end()) {
  			permutation = permutator();
  			std::vector<vector_t<Number>> setOfPoints;
  			for(unsigned index: permutation) {
  				setOfPoints.push_back(vector_t<Number>(cutPointsAround_a_b.at(index).rawCoordinates()));
  			}
  			res.push_back(setOfPoints);
  		}
  	}
  	return res;
  }

  /*
   * Prepare computeNormal with a set of vertices - compute the base vectors
   * @input vertices, test vector a, test vector b
   * @return vector_t<Number> normal vector
   */
  template <typename Number>
  vector_t<Number> HPolytope<Number>::computeNormal(std::vector<vector_t<Number>> vertices, vector_t<Number> check) const {
  	unsigned dimension = vertices.at(0).size();
  	std::vector<vector_t<Number>> baseVectors;
  	for(unsigned i=1; i<vertices.size(); i++){
  		baseVectors.push_back(vertices.at(i)-vertices.at(0));
  	}

  	if(vertices.size()!=dimension){ // check if we have the exact number of vertices for the calculation
  		std::cout << __func__ << " : " << __LINE__ << "\n   Error - the number of vertices does not fit for the calculation.\nActual Number: " << vertices.size() << "\nNeeded Number: " << dimension-1<< std::endl;
  		return check;
  	}

  	vector_t<Number> res = vector_t<Number>::Zero(dimension);

  	for(unsigned i=0; i<dimension; i++){ // iterate through each coordinate of the normal vector
  		// create the matrix and get determinant for each matrix
  		matrix_t<Number> m(dimension-1, dimension-1);
  		for(unsigned r=0; r<dimension; r++) {
  			if(r!=i){
  				unsigned rCorrect = r;
  				if(r>i) rCorrect--;
  				for(unsigned v=0; v<baseVectors.size(); v++){
  					m(rCorrect,v) = baseVectors[v][r];
  				}
  			}
  		}
  		Number determinant = Number(m.determinant());
  		if ((i % 2) == 0) {
  			res[i] = determinant;
  		}
  		else {
  			res[i] = (-1)*determinant;
  		}
  	}

  	if(check.dot(res)<=0){// check direction ...should not be 0?
  		return (-1)*res;
  	}

  	return res;
  }

  /*
   * Get the correct vertex to determine a hyperplane which includes all other facets
   * @input vertices, vector
   * @return Point<Number> vertex for the vector
   */
  template <typename Number>
  Point<Number> HPolytope<Number>::getVertexForVector(vector_t<Number> vector, std::vector<Point<Number>> vertices) const {
  	// try each vertex
  	for(Point<Number> vertex: vertices){
  		bool below=true;
  		Number vector_offset=0;

  		// calculate offset
  		vector_offset = vector.dot(vertex.rawCoordinates());

      // check for each vertex if it lies below the hyperplane
  		for(Point<Number> vertex_test: vertices){
  			Number vector_test_offset=0;

  			if(vertex!=vertex_test){
  				vector_test_offset = vector.dot(vertex_test.rawCoordinates());

  				if(!carl::AlmostEqual2sComplement(vector_test_offset+Number(1), vector_offset+Number(1)) && vector_test_offset>vector_offset){
  					below=false; // vertex lies above
  					break;
  				}
  			}
  		}

  		if(below){
  			return vertex;
  		}
  	}

  	std::cout << "Error - No correct offset " << std::endl;
  	return vertices.at(0);
  }

  /*
   * check if the drop of one facet yields to an unbounded poyltope
   * @input unsigned facet to be droped
   * @return bool isBounded or not
   */
  template <typename Number>
  bool HPolytope<Number>::isBounded(std::vector<vector_t<Number>> evaluations) const {
    HPolytope<Number> hpolytope = *this;

  	for(vector_t<Number> evaluation: evaluations){
  		std::pair<Number, SOLUTION> evaluation_result = hpolytope.evaluate(evaluation);
  		if(evaluation_result.second == INFTY){
  			return false;
  		}
  	}
  	return true;
  }

  /*
   * Compute a |polytope|-(kind of) polytope
   */
   #define PI 3.14159265359
   template <typename Number>
   static std::vector<vector_t<Number>> computeTemplate(unsigned dimension, unsigned polytope) {
  	 double degree = (360/ (double) polytope)* PI / 180.0;
  	 std::vector<vector_t<Number>> templatePolytope, templatePolytope2d;

  	 //create templatePolytope2d
  	 vector_t<Number> templateVector2d = vector_t<Number>::Zero(2); // init templateVector2d
  	 templateVector2d(0) = 1;
  	 templatePolytope2d.push_back(templateVector2d);

  	 matrix_t<Number> m(2, 2); //init matrix
  	 m(0,0) = Number(cos(degree));
  	 m(0,1) = Number((-1)*sin(degree));
  	 m(1,0) = Number(sin(degree));
  	 m(1,1) = Number(cos(degree));


  	 for(unsigned i=0; i<polytope; ++i) {
  		 //Rotate templateVector
  		 templateVector2d = m * templateVector2d;
  		 templatePolytope2d.push_back(templateVector2d);
  	 }

  	 //copy templatePolytope2d into templatePolytope
  	 polytope::dPermutator permutator(dimension, 2);
  	 std::vector<unsigned> permutation;
  	 while(!permutator.end()) {
  		 permutation = permutator();
  		 for(vector_t<Number> vectorOftemplatePolytope2d: templatePolytope2d) {
  			 vector_t<Number> templateVector = vector_t<Number>::Zero(dimension); // init templateVector

  			 templateVector(permutation.at(0)) = vectorOftemplatePolytope2d(0);
  			 templateVector(permutation.at(1)) = vectorOftemplatePolytope2d(1);

  			 if(std::find(templatePolytope.begin(), templatePolytope.end(), templateVector)== templatePolytope.end()) templatePolytope.push_back(templateVector);
  		 }
  	 }

  	 return templatePolytope;
   }

  /*
   * Reduction-Function
   * @Input unsigned strat for the strategy, unsigned a for the facet (Drop, drop_smooth) and first facet for (unite, unite_...), unsigned b for the seconde facet
   * @return the reduced facet or if the result would be unbounded the inital polytope
   */
  template <typename Number>
  HPolytope<Number> HPolytope<Number>::reduce( unsigned a, unsigned b, REDUCTION_STRATEGY strat) const { // REDUCTION_STRATEGY

  	// init
  	HPolytope<Number> res = *this;
  	std::vector<Point<Number>> vertices = res.vertices();
  	std::vector<std::vector<unsigned>> membersOfVertices = getMembersOfVertices(vertices);
  	std::vector<vector_t<Number>> evaluations;

  	std::vector<unsigned> neighborsOf_a = getNeighborsOfIndex(a, membersOfVertices); // get neighbors
  	std::vector<unsigned> neighborsOf_b = getNeighborsOfIndex(b, membersOfVertices); // get neighbors

  	// neighbor test for unite
  	if(strat>1 && strat<6 && std::find(neighborsOf_a.begin(), neighborsOf_a.end(), b)==neighborsOf_a.end()){
  		std::cout << "Error - second facet is no neighbor of first facet" << std::endl;
  		return res;
  	}
    else if(strat>1 && strat<6){
      vector_t<Number> dummy_a = res.constraints().at(a).normal();
      vector_t<Number> dummy_b = res.constraints().at(b).normal();
      dummy_a.normalize();
      dummy_b.normalize();
      std::cout << "Neighbors have a scalarpoduct of " << dummy_a.dot(dummy_b) << " (normalized)" << std::endl;
    }
    if(strat<2){
      std::vector<Point<Number>> verticesOfIndex = getVerticesOfIndex(a, vertices, membersOfVertices);
      vector_t<Number> normVector = verticesOfIndex.at(0).rawCoordinates() - verticesOfIndex.at(1).rawCoordinates();
      std::cout << "Norm for facet " << a << " is " << normVector.norm() << std::endl;
    }


  	// (try to) reduce
  	switch(strat){
  		case REDUCTION_STRATEGY::DROP:
  			{
  				evaluations.push_back(res.constraints().at(a).normal());
  				res.erase(a); // delete facet
  				break;
  			}

  		case REDUCTION_STRATEGY::DROP_SMOOTH:
  			{
  				std::vector<vector_t<Number>> smoothVectors;
  				std::vector<Number> smoothVectors_offset;
  				std::vector<unsigned> smoothVectors_oldIndices;


  				for(unsigned neighbor: neighborsOf_a){
  					smoothVectors.push_back(res.constraints().at(neighbor).normal()+res.constraints().at(a).normal());
  					smoothVectors_offset.push_back(res.constraints().at(neighbor).offset()+res.constraints().at(a).offset());
  					smoothVectors_oldIndices.push_back(neighbor);
  				}

  				neighborsOf_a.push_back(a);
  				std::sort(neighborsOf_a.begin(), neighborsOf_a.end());
  				std::reverse(neighborsOf_a.begin(), neighborsOf_a.end());

  				for(unsigned i=0; i<smoothVectors.size(); i++){
  					res.insert(Hyperplane<Number>(smoothVectors[i], smoothVectors_offset[i])); // update neighbor facets
  				}

  				for(unsigned neighbor: neighborsOf_a){
  					evaluations.push_back(res.constraints().at(neighbor).normal());
  					res.erase(neighbor); // delete not smoothed facet
  				}

  				break;
  			}


  		case REDUCTION_STRATEGY::UNITE:
  			{
  				vector_t<Number> uniteVector = (res.constraints().at(a).normal()+res.constraints().at(b).normal());
  				Number uniteVector_offset = res.constraints().at(a).offset()+res.constraints().at(b).offset();

  				evaluations.push_back(res.constraints().at(a).normal());
  				evaluations.push_back(res.constraints().at(b).normal());

  				res.insert(Hyperplane<Number>(uniteVector, uniteVector_offset)); // insert united facet
  				res.erase(a);
  				res.erase(b);

  				break;
  			}

  		case REDUCTION_STRATEGY::UNITE_SMOOTH:
  			{
  				vector_t<Number> uniteVector = vector_t<Number>::Zero(vertices.at(0).dimension()); // init smooth united facet
  				Number uniteVector_offset;

  				evaluations.push_back(res.constraints().at(a).normal());
  				evaluations.push_back(res.constraints().at(b).normal());

  				// smooth united facet is the sum of all (normalized and smoothed) neighbor facets of a and b
  				// uniteVector
  				for(unsigned neighbor: neighborsOf_a){
  					if(neighbor!=b){
  						vector_t<Number> vector_normalized = res.constraints().at(a).normal()+res.constraints().at(neighbor).normal();
  						vector_normalized.normalize();
  						uniteVector+=vector_normalized;
  					}
  				}
  				for(unsigned neighbor: neighborsOf_b){
  					if(neighbor!=a){
  						vector_t<Number> vector_normalized = res.constraints().at(b).normal()+res.constraints().at(neighbor).normal();
  						vector_normalized.normalize();
  						uniteVector+=vector_normalized;
  					}
  				}

  				// uniteVector_offset
  				Point<Number> point_a_b = getVertexForVector(uniteVector, getVerticesOf2Indices(a, b, vertices, membersOfVertices));

  				// smooth united facet offset is computed with the united facet and cutPoint of facet a and b
  				uniteVector_offset = uniteVector.dot(point_a_b.rawCoordinates());

  				res.insert(Hyperplane<Number>(uniteVector, uniteVector_offset)); // insert united facet
  				res.erase(a);
  				res.erase(b);

  				break;
  			}

  		case REDUCTION_STRATEGY::UNITE_CUT:
  			{
  				vector_t<Number> uniteVector = vector_t<Number>::Zero(vertices.at(0).dimension()); // init cut united facet
  				Number uniteVector_offset;

  				evaluations.push_back(res.constraints().at(a).normal());
  				evaluations.push_back(res.constraints().at(b).normal());

  				// cut united facet is the sum of all possible normals
  				for(std::vector<vector_t<Number>> setOfPoints: getVerticesPermutationForFacet(a, b, vertices)) { // get all set of points which could determine the new facet
  					vector_t<Number> normal = computeNormal(setOfPoints, res.constraints().at(a).normal()); // TODO use a simple hyperplane for this task
  					if(normal!=vector_t<Number>::Zero(vertices.at(0).dimension())) normal.normalize();
  					uniteVector += normal; // add all these candidates
  				}

  				if(uniteVector!=vector_t<Number>::Zero(vertices.at(0).dimension())) uniteVector.normalize();


  				// uniteVector_offset
  				Point<Number> point_a_b = getVertexForVector(uniteVector, getVerticesOf2Indices(a, b, vertices, membersOfVertices));

  				// cut united facet offset is computed with the united facet and cutPoint of facet a and b
  				uniteVector_offset = uniteVector.dot(point_a_b.rawCoordinates());

  				res.insert(Hyperplane<Number>(uniteVector, uniteVector_offset)); // insert united facet
  				res.erase(a);
  				res.erase(b);

  				break;
  			}

  		case REDUCTION_STRATEGY::UNITE_NORM:
  			{
  				// TODO compute weights
  				std::pair<double, double> weights = std::pair<double, double>(5.9, 2);

  				// norm united facet works as unite_normal, but with weights for each component
  				vector_t<Number> uniteVector = res.constraints().at(a).normal()*weights.first + res.constraints().at(b).normal()*weights.second; // weights are calculated by hand
  				Number uniteVector_offset;

  				// uniteVector_offset
  				Point<Number> point_a_b = getVertexForVector(uniteVector, getVerticesOf2Indices(a, b, vertices, membersOfVertices));

  				// norm united facet offset is computed with the united facet and (TODO furthest) cutPoint of facet a and b
					uniteVector_offset = uniteVector.dot(point_a_b.rawCoordinates());

  				break;
  			}
  			default:
  				break;
  	}

  	if(res.isBounded(evaluations)){

  		//check if all vertices are inside the new polytope
  		for(unsigned i=0; i<res.size(); ++i) {
  			for(Point<Number> vertex: vertices){
  				Number value=0;
  				for(unsigned j=0; j<res.constraints().at(i).normal().size(); j++){
  					value+=res.constraints().at(i).normal()[j]*vertex.coordinate(j);
  				}
  				if(!carl::AlmostEqual2sComplement(value, res.constraints().at(i).offset()) && value>res.constraints().at(i).offset()){
  					return *this;
  				}
  			}
  		}

  		return res;
  	}

  	return *this;
  }

  /*
   * Reduction-Function with a special strategy
   * @Input vector_t<Number> directed is the vector in which direction we would like to reduce
   * @return the reduced facet or if the result would be unbounded the inital polytope
   */
  template <typename Number>
  HPolytope<Number> HPolytope<Number>::reduce_directed(std::vector<vector_t<Number>> directions, REDUCTION_STRATEGY strat) const{
  	// init
  	HPolytope<Number> res = *this;

  	//if(res.size()<directions.size()){
  	//	std::cout << "Error - Output would be bigger than reduce_from" << std::endl;
  	//	return poly;
  	//}

  	std::vector<Point<Number>> vertices = res.vertices();
  	std::vector<std::vector<unsigned>> membersOfVertices = getMembersOfVertices(vertices);

  	std::vector<unsigned> facets_erase;
  	if(strat == REDUCTION_STRATEGY::DIRECTED_TEMPLATE){
  			for(unsigned i=0; i<res.size(); i++){ // erase all facets
  				facets_erase.push_back(i);
  			}
  		}
  	std::vector<Hyperplane<Number>> facets_insert;

  	// loop through each direction
  	for(vector_t<Number> directed: directions){
  		Point<Number> point_directed;
  		std::vector<std::pair<unsigned, Number>> scalarproducts;
  		Number directed_offset=0;
  		bool skip=false;

  		// compute scalarproducts
  		vector_t<Number> directed_normalized = directed;
  		directed_normalized.normalize();

  		for(unsigned i=0; i<res.size(); i++){
  			vector_t<Number> temp = res.constraints().at(i).normal();
  			temp.normalize();
  			Number scalarproduct = directed_normalized.dot(temp);

  			//if(carl::AlmostEqual2sComplement(scalarproduct, 1.0)){
  			//	std::cout << "Error - wanted direction does already exist" << std::endl;
  			//	skip=true;
  			//}

  			scalarproducts.push_back(std::pair<unsigned, Number>(i, scalarproduct));
  		}


  		if(!skip){
  			// sort scalarproducts TODO only max?
  			std::sort(scalarproducts.begin(), scalarproducts.end(), [](const std::pair<unsigned,Number> &left, const std::pair<unsigned,Number> &right) {	return left.second > right.second;	});

  			// collect vertex
  			point_directed = getVertexForVector(directed, vertices);

  			// determine offset of directed
				directed_offset = directed.dot(point_directed.rawCoordinates());

  			// collect facets to erase
  			switch(strat){
  				case REDUCTION_STRATEGY::DIRECTED_SMALL:
  					{
  						for(unsigned v=0; v<vertices.size(); v++){
  							if(vertices[v] == point_directed){

  								for(unsigned memberOfVertex: membersOfVertices[v]){
  									vector_t<Number> dummy = res.constraints().at(memberOfVertex).normal();
  									dummy.normalize();
  									Number scalarproduct= dummy.dot(directed_normalized);

  									if(std::find(facets_erase.begin(), facets_erase.end(), memberOfVertex)==facets_erase.end() && !carl::AlmostEqual2sComplement(scalarproduct+1, Number(1.0)) && scalarproduct>0){
  										facets_erase.push_back(memberOfVertex);
  									}
  								}
  							}
  						}
  						break;
  					}
  				case REDUCTION_STRATEGY::DIRECTED_BIG:
  					{
  						for(unsigned i=0; i<res.size(); i++){
  							vector_t<Number> dummy = res.constraints().at(i).normal();
  							dummy.normalize();
  							Number scalarproduct= dummy.dot(directed_normalized);

  							if(std::find(facets_erase.begin(), facets_erase.end(), i)==facets_erase.end() && !carl::AlmostEqual2sComplement(scalarproduct+1, Number(1.0)) && scalarproduct>0){
  								facets_erase.push_back(i);
  							}
  						}
  						break;
  					}
  						default:
  							break;

  			}

  			std::sort(facets_erase.begin(), facets_erase.end(), [](const unsigned left, const unsigned right) {	return left > right;	});

  			facets_insert.push_back(Hyperplane<Number>(directed, directed_offset));
  		}
  	}

  	for(Hyperplane<Number> facet_insert: facets_insert){
  		res.insert(facet_insert);
  	}

  	std::vector<vector_t<Number>> evaluations;
  	for(unsigned facet_erase: facets_erase){
  		evaluations.push_back(res.constraints().at(facet_erase).normal());
  		res.erase(facet_erase);
  	}

  	if(res.isBounded(evaluations)){

  		//check if all vertices are inside the new polytope
  		for(unsigned i=0; i<res.size(); ++i) {
  			for(Point<Number> vertex: vertices){
  				Number value=0;
  				for(unsigned j=0; j<res.constraints().at(i).normal().size(); j++){
  					value+=res.constraints().at(i).normal()[j]*vertex.coordinate(j);
  				}
  				if(!carl::AlmostEqual2sComplement(value, res.constraints().at(i).offset()) && value>res.constraints().at(i).offset()){
  					return *this;
  				}
  			}
  		}

  		return res;
  	}

  	return *this;
  }

} // namespace hypro