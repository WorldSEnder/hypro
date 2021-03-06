/**
 * Testfile for hybrid automata.
 * Author: ckugler
 */

#include "gtest/gtest.h"
#include "../defines.h"
#include "datastructures/hybridAutomata/LocationManager.h"
#include "datastructures/hybridAutomata/Transition.h"
#include "datastructures/hybridAutomata/HybridAutomaton.h"
#include "datastructures/hybridAutomata/RawState.h"
#include "representations/GeometricObject.h"
#include "carl/core/VariablePool.h"

using namespace hypro;
using namespace carl;

template <typename Number>
using valuation_t = VPolytope<Number>;

template<typename Number>
class HybridAutomataTest : public ::testing::Test
{
/**
 * Test Setup:
 * one hybrid automaton which consists of two locations that are connected by one transition
 */
protected:
    virtual void SetUp()
    {
      	/*
		 * Location Setup
		 */

		loc1 = locMan.create();
    	loc2 = locMan.create();


    	trans = new hypro::Transition<Number>();

		invariantVec(0) = 10;
		invariantVec(1) = 20;

		invariantMat(0,0) = 2;
		invariantMat(0,1) = 0;
		invariantMat(1,0) = 0;
		invariantMat(1,1) = 3;

		loc1->setInvariant(invariantMat,invariantVec);

		inv.mat = invariantMat;
		inv.vec = invariantVec;

		loc2->setInvariant(inv);

		locationMat(0,0) = 2;
		locationMat(0,1) = 0;
		locationMat(1,0) = 0;
		locationMat(1,1) = 1;

		loc1->setFlow(locationMat);
		loc2->setFlow(locationMat);

		/*
		 * Transition Setup
		 */
		guard.mat = inv.mat;
		guard.vec = inv.vec;

		reset.vec = inv.vec;
		reset.mat = inv.mat;

		trans->setGuard(guard);
		trans->setSource(loc1);
		trans->setTarget(loc2);
		trans->setReset(reset);

		/*
		 * Hybrid Automaton Setup
		 */
		locations[0] = loc1;
		locations[1] = loc2;

		locSet = std::set<hypro::Location<Number>*>(locations, locations+2);
		init[0] = loc1;
		initLocSet = std::set<hypro::Location<Number>*>(init, init+1);

		//Polytope for InitialValuation & Guard Assignment
		coordinates(0) = 2;
		coordinates(1) = 3;

    	std::vector< vector_t<Number> > vecSet;
    	vecSet.push_back(coordinates);
		poly = valuation_t<Number>(vecSet);
		auto hpoly = hypro::Converter<Number>::toHPolytope(poly);

		hybrid.setLocations(locSet);
		for(auto loc : initLocSet) {
			RawState<Number> initState(loc, std::make_pair(hpoly.matrix(), hpoly.vector()));
			hybrid.addInitialState(initState);
		}

		transition[0] = trans;
		transSet = std::set<hypro::Transition<Number>*>(transition, transition+1);

		hybrid.setTransitions(transSet);
		loc1->setTransitions(transSet);
    }

    virtual void TearDown()
    {
    	//delete loc1;
    	//delete loc2;
    	//delete trans;
    }

    //Hybrid Automaton Objects: Locations, Transitions, Automaton itself

    LocationManager<Number>& locMan = LocationManager<Number>::getInstance();

    Location<Number>* loc1;
    Location<Number>* loc2;
    hypro::Transition<Number>* trans;
    HybridAutomaton<Number> hybrid;

    //Other Objects: Vectors, Matrices, Guards...
    vector_t<Number> invariantVec = vector_t<Number>(2,1);
    matrix_t<Number> invariantMat = matrix_t<Number>(2,2);
	struct Location<Number>::Invariant inv;
	matrix_t<Number> locationMat = matrix_t<Number>(2,2);

    struct hypro::Transition<Number>::Guard guard;

    struct hypro::Transition<Number>::Reset reset;

    hypro::Location<Number>* locations[2];
    std::set<hypro::Location<Number>*> locSet;

    hypro::Location<Number>* init[1];
    std::set<hypro::Location<Number>*> initLocSet;

    hypro::Transition<Number>* transition[1];
	std::set<hypro::Transition<Number>*> transSet;

	vector_t<Number> coordinates = vector_t<Number>(2,1);
    valuation_t<Number> poly;
};

/**
 * Location Test
 */
TYPED_TEST(HybridAutomataTest, LocationTest)
{
    //invariant: vector
    EXPECT_EQ(this->loc1->invariant().vec, this->invariantVec);
    EXPECT_EQ(this->loc2->invariant().vec, this->invariantVec);

	vector_t<TypeParam> invariantVec2(2,1);
	invariantVec2(0) = 10;
	invariantVec2(1) = 10;
	EXPECT_NE(this->loc1->invariant().vec, invariantVec2);

	//invariant: matrix
	EXPECT_EQ(this->loc1->invariant().mat, this->invariantMat);
	EXPECT_EQ(this->loc2->invariant().mat, this->invariantMat);

	matrix_t<TypeParam> invariantMat2(2,2);
	invariantMat2(0,0) = 1;
	invariantMat2(0,1) = 0;
	invariantMat2(1,0) = 0;
	invariantMat2(1,1) = 3;
	EXPECT_NE(this->loc1->invariant().mat, invariantMat2);

	//location: matrix
	EXPECT_EQ(this->loc1->flow(), this->locationMat);

	matrix_t<TypeParam> locationMat2(2,2);
	locationMat2(0,0) = 1;
	locationMat2(0,1) = 0;
	locationMat2(1,0) = 0;
	locationMat2(1,1) = 1;
	EXPECT_NE(this->loc1->flow(), locationMat2);

	//location: set of outgoing transitions
	EXPECT_EQ(this->loc1->transitions(), this->transSet);

	EXPECT_TRUE(*this->loc1 < *this->loc2);
	EXPECT_FALSE(*this->loc2 < *this->loc1);

	EXPECT_TRUE(locPtrComp<TypeParam>()(this->loc1, this->loc2));
}

/**
 * Transition Test
 */
TYPED_TEST(HybridAutomataTest, TransitionTest)
{
	//transition: Start Location
	EXPECT_EQ(this->trans->source(), this->loc1);

	//transition: End Location
	EXPECT_EQ(this->trans->target(),this->loc2);

	//transition: Assignment
	EXPECT_EQ(this->trans->reset().vec, this->reset.vec);
	EXPECT_EQ(this->trans->reset().mat, this->reset.mat);

	//transition: Guard
	EXPECT_EQ(this->trans->guard().vec, this->guard.vec);
	EXPECT_EQ(this->trans->guard().mat, this->guard.mat);

	// creation of transitions from source and target
	Transition<TypeParam>* t = new Transition<TypeParam>(this->loc1, this->loc2);
	EXPECT_EQ(t->source(), this->loc1);
	EXPECT_EQ(t->target(), this->loc2);
	EXPECT_EQ(t->aggregation(), Aggregation::boxAgg);
	EXPECT_FALSE(t->isTimeTriggered());

	t->setAggregation(Aggregation::none);
	EXPECT_EQ(t->aggregation(), Aggregation::none);

	t->setTriggerTime(TypeParam(1));
	EXPECT_TRUE(t->isTimeTriggered());
	EXPECT_EQ(t->triggerTime(), TypeParam(1));
}

/**
 * Hybrid Automaton Test
 */
TYPED_TEST(HybridAutomataTest, HybridAutomatonTest)
{
	// construct a new hybrid automaton.
	HybridAutomaton<TypeParam> h1;

	h1.addLocation(this->loc1);
	h1.addLocation(this->loc2);

	EXPECT_TRUE(std::find(h1.locations().begin(), h1.locations().end(), this->loc1) != h1.locations().end());
	EXPECT_TRUE(std::find(h1.locations().begin(), h1.locations().end(), this->loc2) != h1.locations().end());

	h1.addTransition(this->trans);
	EXPECT_TRUE(std::find(h1.transitions().begin(), h1.transitions().end(), this->trans) != h1.transitions().end());

	matrix_t<TypeParam> matr = matrix_t<TypeParam>::Identity(2,2);
	vector_t<TypeParam> vec = vector_t<TypeParam>(2);
	vec << 1,2;

	h1.addInitialState(RawState<TypeParam>(this->loc1, std::make_pair(matr, vec)));

	// copy assignment operator
	HybridAutomaton<TypeParam> h2 = h1;
	EXPECT_EQ(h1, h2);
}

TYPED_TEST(HybridAutomataTest, LocationManagerTest)
{
	matrix_t<TypeParam> flow = matrix_t<TypeParam>::Identity(2,2);
	Location<TypeParam>* loc = this->locMan.create(flow);
	EXPECT_EQ(loc->flow(), flow);

	unsigned id = this->locMan.id(loc);
	EXPECT_EQ(this->locMan.location(id), loc);
}


TYPED_TEST(HybridAutomataTest, RawState) {
	// Constructors
	RawState<TypeParam> s1(this->loc1);

	matrix_t<TypeParam> matr = matrix_t<TypeParam>::Identity(2,2);
	vector_t<TypeParam> vec = vector_t<TypeParam>(2);
	vec << 1,2;
	RawState<TypeParam> s2(this->loc1, std::make_pair(matr, vec));

	EXPECT_EQ(s1.location->id(), this->loc1->id());
	EXPECT_EQ(s2.location->id(), this->loc1->id());
	EXPECT_EQ(s2.set.first, matr);
	EXPECT_EQ(s2.set.second, vec);
}
