#include "ConvexHull.h"

namespace hypro {
	
	template<typename Number>
	ConvexHull<Number>::ConvexHull(const std::vector<Point<Number>> points): mPoints(points) {}
	
	template<typename Number>
	ConvexHull<Number>::ConvexHull(const std::vector<Point<Number>> points, const std::vector<vector_t<Number>> cone):
		mPoints(points),
		mCone(cone) 
		{}
		
	template<typename Number>
	ConvexHull<Number>::ConvexHull(const std::vector<Point<Number>> points, const std::vector<vector_t<Number>> cone,
									const std::vector<vector_t<Number>> linealty):
		mPoints(points),
		mCone(cone),
		mLinealtySpace(linealty)
		{}
	
	template<typename Number>
	std::vector<Point<Number>> ConvexHull<Number>::getPoints() const {
		return mPoints;
	}
	
	template<typename Number>
	std::vector<vector_t<Number>> ConvexHull<Number>::getCone() const {
		return mCone;
	}
	
	template<typename Number>
	std::vector<vector_t<Number>> ConvexHull<Number>::getLinealtySpace() const {
		return mLinealtySpace;
	}
	
	template<typename Number>
	std::vector<Halfspace<Number>> ConvexHull<Number>::getConeHsv() const {
		return mConeHsv;
	}
	
	template<typename Number>
	std::vector<Halfspace<Number>> ConvexHull<Number>::getDualHsv() const {
		return mDualHsv;
	}
	
	template<typename Number>
	std::vector<Halfspace<Number>> ConvexHull<Number>::getHsv() const {
		return mHsv;
	}
	
	template<typename Number>
	void ConvexHull<Number>::findOffset() {
		mOffset = vector_t<Number>::Zero(mPoints[0].rawCoordinates().size());
		vector_t<Number> zero = vector_t<Number>::Zero(mPoints[0].rawCoordinates().size());
		for(const auto& point: mPoints) {
			mOffset+=point.rawCoordinates();
		}
		mOffset = mOffset/mPoints.size();
		unsigned index = 0;
		for(auto& point: mPoints) {
			point.setCoordinates(point.rawCoordinates() - mOffset);
			if(point.rawCoordinates()==zero) {mPoints.erase(mPoints.begin()+index);};
			++index;
		}
	}
	
	template<typename Number>
	void ConvexHull<Number>::toDual() {
		for(const auto& p:mPoints) {
			mDualHsv.push_back(Halfspace<Number>(p.rawCoordinates(),Number(1)));
		}
	}
	
	template<typename Number>
	void ConvexHull<Number>::toPrimal(const std::vector<Point<Number>> points) {
		for(const auto& p:points) {
			mHsv.push_back(Halfspace<Number>(p.rawCoordinates(),Number(1)));
		}
	}
	
	template<typename Number>//output 00000 <=2
	void ConvexHull<Number>::convexHullVertices() {//!!modify the points
		if(mPoints.size()==0) {
			vector_t<Number> h1 = vector_t<Number>(1);
			vector_t<Number> h2 = vector_t<Number>(-1);
			mHsv.push_back(Halfspace<Number>(h1,Number(0)));
			mHsv.push_back(Halfspace<Number>(h2,Number(0)));
		} else {
			unsigned dimension = mPoints[0].rawCoordinates().size();
			findOffset();
			if(mPoints.size()==0) {
				Point<Number> zeroPoint = Point<Number>(vector_t<Number>::Zero(dimension));
				mPoints.push_back(zeroPoint);
				for(unsigned index=0;index<dimension;++index) {
					vector_t<Number> newNormal = vector_t<Number>::Zero(dimension);
					newNormal[index]=1;
					mHsv.push_back(Halfspace<Number>(newNormal,Number(0)));
					newNormal[index]=-1;
					mHsv.push_back(Halfspace<Number>(newNormal,Number(0)));
				}
			} else {
				toDual();
				VertexEnumeration<Number> ev = VertexEnumeration<Number>(mDualHsv);
				ev.enumerateVertices();
				for(const auto& l:ev.getLinealtySpace()) {
					mHsv.push_back(Halfspace<Number>(l,Number(0)));
					mHsv.push_back(Halfspace<Number>(-1*l,Number(0)));
				}
				toPrimal(ev.getPoints());
			}
			translateHsv();
		}
	}
	
	template<typename Number>
	void ConvexHull<Number>::conicHull() {
		std::vector<Point<Number>> points;
		for(const auto& cone:mCone) {
			points.push_back(Point<Number>(cone));
		}
		Point<Number> zero = Point<Number>::Zero(mCone[0].size());
		points.push_back(zero);
		ConvexHull<Number> ch = ConvexHull<Number>(points);
		ch.convexHullVertices();
		for(const auto& hs:ch.getHsv()) {
			if(hs.offset()==0) {mConeHsv.push_back(hs);}
		}
	}
	
	template<typename Number>
	void ConvexHull<Number>::polyhedriclHull() {
		std::vector<vector_t<Number>> newCone;
		for(const auto& p:mPoints) {
			vector_t<Number> aux = vector_t<Number>(p.rawCoordinates().size()+1);
			for(unsigned index=0;index<p.rawCoordinates().size();++index) {
				aux[index]=p.rawCoordinates()[index];
			}
			aux[p.rawCoordinates().size()]=1;
			newCone.push_back(aux);
		}
		for(const auto& c:mCone) {
			vector_t<Number> aux = vector_t<Number>(c.size()+1);
			for(unsigned index=0;index<c.size();++index) {
				aux[index]=c[index];
			}
			aux[c.size()]=0;
			newCone.push_back(aux);
		}
		for(const auto& l:mLinealtySpace) {
			vector_t<Number> aux = vector_t<Number>(l.size()+1);
			vector_t<Number> aux2 = vector_t<Number>(l.size()+1);
			for(unsigned index=0;index<l.size();++index) {
				aux[index]=l[index];
				aux2[index]=-1*l[index];
			}
			aux[l.size()]=0;
			aux2[l.size()]=0;
			newCone.push_back(aux);
			newCone.push_back(aux2);
		}
		std::vector<Point<Number>> empty;
		ConvexHull<Number> ch = ConvexHull<Number>(empty, newCone);
		ch.conicHull();
		std::vector<Halfspace<Number>> hsv = ch.getConeHsv();
		for(auto& hs:hsv) {
			hs.setOffset(hs.offset()-hs.normal()[hs.normal().size()-1]);//projection fonction
			vector_t<Number> aux=vector_t<Number>(hs.normal().size()-1);
			for(unsigned index=0;index<hs.normal().size()-1;++index) {
				aux[index] = hs.normal()[index];
			}
			vector_t<Number> zero=vector_t<Number>::Zero(hs.normal().size()-1);
			hs.setNormal(aux);
			if(aux!=zero) {mHsv.push_back(hs);}
		}
	}

	template<typename Number>
	void ConvexHull<Number>::translateHsv() {
		for(auto& hs:mHsv) {
			for(unsigned index = 0;index<hs.normal().size();++index) {
				hs.setOffset(hs.offset() + hs.normal()[index]*mOffset[index]);
			}
		}
	}
	
	template<typename Number>//not used
	void ConvexHull<Number>::projectOnLinealty() {
		if(mPoints.size()!=0&&mLinealtySpace.size()!=0) {
			unsigned dim = mPoints[0].rawCoordinates().size();
			unsigned index=0;
			std::vector<Number> norms;
			std::vector<vector_t<Number>> projectionBase;
			while(projectionBase.size()<dim&&index<mLinealtySpace.size()) {
				vector_t<Number> nextLineatyVector = mLinealtySpace[index];
				for(unsigned vectorIndex=0;vectorIndex<projectionBase.size();++vectorIndex){
					nextLineatyVector=nextLineatyVector
							-projectionBase[vectorIndex]*((projectionBase[vectorIndex].dot(nextLineatyVector)/norms[vectorIndex]));
				}
				int i=0;
				while(i<nextLineatyVector.size()) {
					if(nextLineatyVector[i]!=0) {break;}
					++i;
				}
				if(i<nextLineatyVector.size()) {
					projectionBase.push_back(nextLineatyVector);
					norms.push_back(nextLineatyVector.dot(nextLineatyVector));
				}
				++index;
			}
			mLinealtySpace = projectionBase;
			for(auto& p:mPoints) {
				index=0;
				for(const auto& pv:projectionBase) {
					p.setCoordinates(p.rawCoordinates()-pv*(pv.dot(p.rawCoordinates())/norms[index]));
					++index;
				}
			}
		}
	}
	
}
