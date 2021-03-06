#include "Location.h"

namespace hypro {

template <typename Number>
Location<Number>::Location( unsigned _id )
	: mFlow(), mExternalInput(), mTransitions(), mInvariant(), mId( _id ) {
}

template <typename Number>
Location<Number>::Location( unsigned _id, const Location &_loc )
	: mFlow( _loc.activityMat() )
	, mExternalInput( _loc.externalInput() )
	, mTransitions( _loc.transitions() )
	, mInvariant( _loc.invariant() )
	, mId( _id ) {
}

template<typename Number>
Location<Number>::Location( unsigned _id, const matrix_t<Number>& _mat )
	: mFlow( _mat )
	, mId( _id )
{}

template <typename Number>
Location<Number>::Location( unsigned _id, const matrix_t<Number>& _mat,
							const transitionSet& _trans, const Location<Number>::Invariant& _inv )
	: mFlow( _mat )
	, mExternalInput()
	, mTransitions( _trans )
	, mInvariant( _inv )
	, mId( _id ) {
}

template <typename Number>
Location<Number>::Location( unsigned _id, const matrix_t<Number>& _mat,
							const transitionSet& _trans, const Location<Number>::Invariant& _inv,
							const matrix_t<Number>& _extInputMat )
	: mFlow( _mat )
	, mExternalInput( _extInputMat )
	, mTransitions( _trans )
	, mInvariant( _inv )
	, mId( _id ) {
}

template <typename Number>
const matrix_t<Number> &Location<Number>::flow() const {
	return mFlow;
}

template <typename Number>
const matrix_t<Number> &Location<Number>::externalInput() const {
	return mExternalInput;
}

template <typename Number>
const typename Location<Number>::Invariant &Location<Number>::invariant() const {
	return mInvariant;
}

template <typename Number>
const std::set<Transition<Number> *> &Location<Number>::transitions() const {
	return mTransitions;
}

template <typename Number>
void Location<Number>::setFlow( const matrix_t<Number>& _mat ) {
	mFlow = _mat;
}

template <typename Number>
void Location<Number>::setExtInputMat( const matrix_t<Number>& _mat ) {
	mExternalInput = _mat;
}

template <typename Number>
void Location<Number>::setInvariant( const Location<Number>::Invariant& _inv ) {
	mInvariant = _inv;
}

template <typename Number>
void Location<Number>::setInvariant( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) {
	mInvariant.vec = _vec;
	mInvariant.mat = _mat;
}

template <typename Number>
void Location<Number>::setTransitions( const transitionSet& _trans ) {
	mTransitions = _trans;
}

template<typename Number>
void Location<Number>::addTransition( Transition<Number>* _trans ) {
	mTransitions.insert(_trans);
}

template<typename Number>
void Location<Number>::addArtificialDimension() {
	// Flow is already augmented.
	// Todo: ExternalInput
	for(auto transition : mTransitions) {
		transition->addArtificialDimension();
	}
	mInvariant.addArtificialDimension();
}

} // namespace hypro
