/** 
 * @file    OrthogonalPolyhedron.tpp
 * @author  Sebastian Junges
 * @author	Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 * @author  Benedikt Seidl
 *
 * @since   2014-03-17
 * @version 2015-07-09
 */


namespace hypro
{
	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron() : 
		//mVertices(),
		mGrid(),
		mBoundaryBox(),
		mBoxUpToDate(false),
		mVariables()
	{}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron(const Vertex<Number>& _vertex) :
		//mVertices(_vertex),
		mGrid(),
		mBoundaryBox(),
		mBoxUpToDate(false),
		mVariables()
	{
		if(_vertex.color())
			mGrid.insert(_vertex.point(), _vertex.color());	
	}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron(const VertexContainer<Number>& _vertices) : 
		//mVertices(_vertices),
		mGrid(_vertices.vertices()),
		mBoundaryBox(),
		mBoxUpToDate(false),
		mVariables()
	{}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron(const std::set<Vertex<Number>>& _vertices) : 
		//mVertices(_vertices),
		mGrid(_vertices),
		mBoundaryBox(),
		mBoxUpToDate(false),
		mVariables()
	{}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron(const std::vector<Vertex<Number>>& _vertices) : 
		//mVertices(_vertices),
		mGrid(_vertices),
		mBoundaryBox(),
		mBoxUpToDate(false),
		mVariables()
	{}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type>::OrthogonalPolyhedron(const OrthogonalPolyhedron<Number, Type>& copy) :
		//mVertices(copy.vertices()),
		mGrid(copy.vertices()),
		mBoundaryBox(copy.boundaryBox()),
		mBoxUpToDate(true),
		mVariables(copy.variables())
	{}

	/**********************************
	 * Getter & setter
	 **********************************/
	
	/**
	 * Returns the list of variables of this polyhedron
	 */
	template<typename Number, ORTHO_TYPE Type>
	std::vector<carl::Variable> OrthogonalPolyhedron<Number, Type>::variables() const {
		return mVariables;
	}

	template<typename Number, ORTHO_TYPE Type>
	std::vector<Vertex<Number>> OrthogonalPolyhedron<Number, Type>::vertices() const {
		return mGrid.vertices();
	}
	
	/**
	 * Returns and if necessary calculates the boundary box.
	 */
	template<typename Number, ORTHO_TYPE Type>
	Box<Number> OrthogonalPolyhedron<Number, Type>::boundaryBox() const {
		if (!mBoxUpToDate) {
			updateBoundaryBox();
		}
		return mBoundaryBox;
	}

	template<typename Number, ORTHO_TYPE Type>
	void OrthogonalPolyhedron<Number, Type>::addVertex(const Vertex<Number>& _vertex){

	}

	template<typename Number, ORTHO_TYPE Type>
	void OrthogonalPolyhedron<Number, Type>::addVertices(const std::vector<Vertex<Number>>& _vertices){

	}

	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::empty() const {
		return mGrid.empty();
	}

	template<typename Number, ORTHO_TYPE Type>
	unsigned OrthogonalPolyhedron<Number, Type>::size() const {
		return mGrid.size();
	}

	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::isVertex(const Point<Number>& _point) const {
		for(unsigned dimension : this->dimension()) {
			if(!isOnIEdge(_point, dimension))
				return false;
		}
		return true;
	}

	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::isOnIEdge(const Point<Number>& _point, unsigned i) const {
		std::vector<Point<Number>> iNeighbors = mGrid.iNeighborhood(_point,i);
		for(const auto& neighbor : iNeighbors) {
			if(mGrid.colorAt(neighbor) != mGrid.colorAt(mGrid.iPredecessor(neighbor,i))) return true;
		}
		return false;
	}

	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::isInternal(const Point<Number>& _point) const {
		return (mGrid.colorAt(_point) == true);
	}

	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::isExternal(const Point<Number>& _point) const {
		return !isInternal(_point);
	}

	template<typename Number, ORTHO_TYPE Type>
	std::vector<Point<Number>> OrthogonalPolyhedron<Number, Type>::iNeighborhood(const Point<Number>& _point, unsigned i) const {
		return mGrid.iNeighborhood(_point,i);
	}

	template<typename Number, ORTHO_TYPE Type>
	std::vector<Point<Number>> OrthogonalPolyhedron<Number, Type>::iNegNeighborhood(const Point<Number>& _point, unsigned i) const {
		return mGrid.iNeighborhood(mGrid.iPredecessor(_point,i),i);
	}

	template<typename Number, ORTHO_TYPE Type>
	std::vector<Point<Number>> OrthogonalPolyhedron<Number, Type>::neighborhood(const Point<Number>& _point) const {
		return std::move(mGrid.neighborhoodInduced(_point));
	}

	template<typename Number, ORTHO_TYPE Type>
	std::vector<Point<Number>> OrthogonalPolyhedron<Number, Type>::iSlice(unsigned i, Number pos) const {
		// TODO: ATTENTION, this is the induced version, transform!
		return std::move(mGrid.iSliceInduced(i,pos));
	}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number,Type> OrthogonalPolyhedron<Number, Type>::iProjection(unsigned i) const {

	}


	/**********************************
	 * Geometric Object functions
	 **********************************/
	
	template<typename Number, ORTHO_TYPE Type>
	unsigned int OrthogonalPolyhedron<Number, Type>::dimension() const {
		return mGrid.dimension();
	}
		
	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type> OrthogonalPolyhedron<Number, Type>::linearTransformation(const matrix_t<Number>& A, const vector_t<Number>& b) const {
		/*
		VertexContainer<Number> newVertices(mVertices);
		newVertices.linearTransformation(A, b);
		OrthogonalPolyhedron<Number, Type> result = OrthogonalPolyhedron<Number, Type>(newVertices);
		// TODO: undefined behavior, does not update colors of vertices nor fix the non-parallel edges
		return result;
		*/
		OrthogonalPolyhedron<Number, Type> result;
		return result;
	}

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type> OrthogonalPolyhedron<Number, Type>::minkowskiSum(const OrthogonalPolyhedron<Number, Type>& rhs) const {
		// TODO
		// Definition: A+B = { a + b | a ∈ A, b ∈ B}
		// Idea: put one polyhedron on all the vertices of the other one
		// Problem: Which vertices to connect, which to remove?
		// Thoughts: do two vertices belong to former neighbors?
		//           store the "color" of a moved polyhedron
		OrthogonalPolyhedron<Number, Type> result;
		return result;
	}
		
	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type> OrthogonalPolyhedron<Number, Type>::intersect(const OrthogonalPolyhedron<Number, Type>& rhs) const {
		/*assert(mVariables == rhs.mVariables);
		
		// first initialize the set of potential vertices
		vSet<Number> potentialVertices;
		calculatePotentialVertices(potentialVertices, mVertices.vertices(), rhs.mVertices.vertices());
		
		// create a temporary grid including all potential vertices
		Grid<Number> tmpGrid(potentialVertices);
		vSet<int> inducedPotentialVertices = tmpGrid.translateToInduced(potentialVertices);
		
		// the container to store the actual vertices
		VertexContainer<Number> vertices;
		typename NeighborhoodContainer::neighborhood neighbors;
		std::map<Point<int>, bool> coloringIntersection;
		
		// check all potential vertices if they are in fact vertices
		for (auto vertexIt : inducedPotentialVertices) {
			// calculate the neighbor coloring intersection
			neighbors = mNeighborhood.getNeighborhood(vertexIt.point(), true); // include the point itself
			coloringIntersection.clear();

			for (auto neighborIt : neighbors) {
				Point<Number> original = tmpGrid.calculateOriginal(neighborIt);
				bool color1 = this->contains(original);
				bool color2 = rhs.contains(original);
				coloringIntersection.insert(std::make_pair(neighborIt, color1 && color2));
			}
			
			// if the point is a vertex, add it with its calculated color
			if (checkVertexCondition(vertexIt, coloringIntersection)) {
				vertices.insert(tmpGrid.calculateOriginal(vertexIt.point()), coloringIntersection.at(vertexIt.point()));
			}
		}

		OrthogonalPolyhedron<Number, Type> result = OrthogonalPolyhedron<Number, Type>(vertices);
		
		return result;*/
	}
		
	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type> OrthogonalPolyhedron<Number, Type>::hull() const {
		if (!mBoxUpToDate) {
			updateBoundaryBox();
		}
		
		int nrofVertices = pow(2, dimension());
		VertexContainer<Number> container;
		Vertex<Number> vertex;
		
		for (int vertexNr = 0; vertexNr < nrofVertices; vertexNr++) {
			int i = 0;
			for (auto variableIt : mVariables) {
				// look if the bit of the current dimension is set
				if ((vertexNr >> i++) & 1) {
					vertex[variableIt] = mBoundaryBox.interval(variableIt).upper();
				} else {
					vertex[variableIt] = mBoundaryBox.interval(variableIt).lower();
				}
			}
			vertex.setColor(vertexNr == 0);
			container.insert(vertex);
		}
		OrthogonalPolyhedron<Number, Type> result = OrthogonalPolyhedron<Number, Type>(container);
		
		return result;
	}
	
	/*	
	template<typename Number, ORTHO_TYPE Type>
	bool OrthogonalPolyhedron<Number, Type>::contains(const Point<Number>& point) const {
		return containsInduced(mGrid.calculateInduced(point));
	}
	*/

	template<typename Number, ORTHO_TYPE Type>
	OrthogonalPolyhedron<Number, Type> OrthogonalPolyhedron<Number, Type>::unite(const OrthogonalPolyhedron<Number, Type>& rhs) const {
		std::vector<Vertex<Number>> vertices;
		vertices.resize(this->size() + rhs.size());

		vertices.insert(vertices.begin(), this->vertices().begin(), this->vertices().end());
		vertices.insert(vertices.begin(), rhs.vertices().begin(), rhs.vertices().end());

		return std::move(OrthogonalPolyhedron<Number, Type>(vertices));
		/*
		assert(mVariables == rhs.mVariables);
		
		// first initialize the set of potential vertices
		vSet<Number> potentialVertices;
		calculatePotentialVertices(potentialVertices, mVertices.vertices(), rhs.mVertices.vertices());
		for (auto vertexIt : this->mVertices.vertices()) {
			if ( !rhs.contains(vertexIt.point()) ) {
				potentialVertices.insert(vertexIt.point());
			}
		}
		for (auto vertexIt : rhs.mVertices.vertices()) {
			if ( !this->contains(vertexIt.point()) ) {
				potentialVertices.insert(vertexIt.point());
			}
		}
		
		// create a temporary grid including all potential vertices
		Grid<Number> tmpGrid(potentialVertices);
		vSet<int> inducedPotentialVertices = tmpGrid.translateToInduced(potentialVertices);
		
		// the container to store the actual vertices
		VertexContainer<Number> vertices;
		typename NeighborhoodContainer::neighborhood neighbors;
		std::map<Point<int>, bool> coloringUnion;
		
		// check all potential vertices if they are in fact vertices
		for (auto vertexIt : inducedPotentialVertices) {
			// calculate the neighbor coloring intersection
			neighbors = mNeighborhood.getNeighborhood(vertexIt.point(), true); // include the point itself
			coloringUnion.clear();

			for (auto neighborIt : neighbors) {
				Point<Number> original = tmpGrid.calculateOriginal(neighborIt);
				bool color1 = this->contains(original);
				bool color2 = rhs.contains(original);
				coloringUnion.insert(std::make_pair(neighborIt, color1 || color2));
			}
			
			// if the point is a vertex, add it with its calculated color
			if (checkVertexCondition(vertexIt, coloringUnion)) {
				vertices.insert(tmpGrid.calculateOriginal(vertexIt.point()), coloringUnion.at(vertexIt.point()));
			}
		}

		OrthogonalPolyhedron<Number, Type> result = OrthogonalPolyhedron<Number, Type>(vertices);
		
		return result;
		*/
	}
		
	/**********************************
	 * Other functions
	 **********************************/

	template<typename Number, ORTHO_TYPE Type>
	std::vector<std::vector<Point<Number>>> OrthogonalPolyhedron<Number, Type>::preparePlot(unsigned _xDim, unsigned _yDim) const {
		std::vector<std::vector<Point<Number>>> result;
		std::vector<Vertex<Number>> vertices = mGrid.vertices();

		// reduce dimensions of vertices
		std::vector<Point<Number>> reduced;
		for(auto& vertex : vertices){
			Point<Number> reduction = vertex.point();
			reduction.reduceToDimensions(std::vector<unsigned>({_xDim,_yDim}));
			reduced.push_back(std::move(reduction));
		}

		// add points
		for(const auto& vertex : reduced){
			Point<Number> lowerRight = vertex;
			lowerRight.incrementInFixedDim(0);
			Point<Number> upperRight = vertex;
			upperRight.incrementInAllDim(1);
			Point<Number> upperLeft = vertex;
			upperLeft.incrementInFixedDim(1);

			//std::cout << "Vertex " << vertex << " corresponds to box " << vertex << " , " << lowerRight << " , " << upperRight << " , " << upperLeft << std::endl;
			result.emplace_back(std::vector<Point<Number>>({vertex, lowerRight, upperRight, upperLeft }));
		}

		return result;
	}

	/***************************************************************************
	 * Private methods
	 ***************************************************************************/
	
	/**
	 * Updates the boundary box
	 */
	template<typename Number, ORTHO_TYPE Type>
	void OrthogonalPolyhedron<Number, Type>::updateBoundaryBox() const {
		// If there are no vertices, the box is empty
		if (mGrid.empty()) {
			mBoundaryBox.clear();
		}
		
		std::vector<Vertex<Number>> vertices = mGrid.vertices();
		mBoundaryBox = Box<Number>(vertices);
		
		mBoxUpToDate = true;
	}
}
