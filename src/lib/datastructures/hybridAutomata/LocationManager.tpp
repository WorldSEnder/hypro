#include "LocationManager.h"

namespace hypro {
template <typename Number>
Location<Number> *LocationManager<Number>::create() {
	Location<Number> *loc = new Location<Number>( mId++ );
	mLocations[mId - 1] = loc;
	return loc;
}

template <typename Number>
Location<Number> *LocationManager<Number>::create( const Location<Number> *_loc ) {
	Location<Number> *loc = new Location<Number>( mId++, _loc );
	mLocations[mId - 1] = loc;
	return loc;
}

template <typename Number>
Location<Number> *LocationManager<Number>::create( const hypro::matrix_t<Number> _mat,
												   const hypro::vector_t<Number> _vec,
												   const typename Location<Number>::transitionSet _trans,
												   const struct Location<Number>::Invariant _inv ) {
	Location<Number> *loc = new Location<Number>( mId++, _mat, _vec, _trans, _inv );
	mLocations[mId - 1] = loc;
	return loc;
}

template <typename Number>
Location<Number> *LocationManager<Number>::create( const hypro::matrix_t<Number> _mat,
												   const hypro::vector_t<Number> _vec,
												   const typename Location<Number>::transitionSet _trans,
												   const struct Location<Number>::Invariant _inv,
												   const hypro::matrix_t<Number> _extInputMat ) {
	Location<Number> *loc = new Location<Number>( mId++, _mat, _vec, _trans, _inv, _extInputMat );
	mLocations[mId - 1] = loc;
	return loc;
}
}
