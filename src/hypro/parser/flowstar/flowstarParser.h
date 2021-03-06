#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif
//#define BOOST_SPIRIT_DEBUG

#pragma once

#ifdef USE_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
#endif

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include "config.h"
#include "common.h"
#include "algorithms/reachability/Settings.h"
#include "datastructures/hybridAutomata/RawState.h"
#include "datastructures/hybridAutomata/LocationManager.h"
#include "datastructures/hybridAutomata/HybridAutomaton.h"
#include "util/logging/Logger.h"
#include "util/VariablePool.h"
#include "symbols.h"
#include "polynomialParser.h"
#include "componentParser.h"

namespace hypro {
namespace parser {

template <typename Number>
struct flowstarParser : qi::grammar<Iterator, Skipper>
{
	variableParser<Iterator> mVariableParser;
	modeParser<Iterator, Number> mModeParser;
	transitionParser<Iterator, Number> mTransitionParser;
	settingsParser<Iterator, Number> mSettingsParser;
	constraintParser<Iterator,Number> mConstraintParser;
	discreteConstraintParser<Iterator,Number> mDiscreteConstraintParser;
	px::function<ErrorHandler> errorHandler;

	LocationManager<Number>& mLocationManager = LocationManager<Number>::getInstance();
	symbol_table mVariableSymbolTable;
	symbol_table mDiscreteVariableSymbolTable;
	symbol_table mModes;
	VariablePool& mVariablePool = VariablePool::getInstance();
	std::set<Transition<Number>*> mTransitions;
	std::vector<RawState<Number>> mInitialStates;
	std::vector<RawState<Number>> mLocalBadStates;
	std::vector<unsigned> mModeIds;
	std::vector<unsigned> mVariableIds;
	std::vector<unsigned> mDiscreteVariableIds;
	hypro::reachability::ReachabilitySettings<Number> mSettings;
	unsigned mDimensionLimit;
	unsigned mDiscreteDimensionLimit;

	flowstarParser();

	qi::rule<Iterator, Skipper> start;
	qi::rule<Iterator, Skipper> continuousStart;
	qi::rule<Iterator, Skipper> hybridStart;
	qi::rule<Iterator, Skipper> continuousVariables;
	qi::rule<Iterator, Skipper> discreteVariables;
	qi::rule<Iterator, Skipper> settings;
	qi::rule<Iterator, qi::unused_type(), spirit::locals<unsigned>, Skipper> init;
	qi::rule<Iterator, qi::unused_type(const unsigned&)> continuousInit;
	qi::rule<Iterator, qi::unused_type(const unsigned&)> discreteInit;
	qi::rule<Iterator, qi::unused_type(), spirit::locals<unsigned>, Skipper> badStates;
	qi::rule<Iterator, qi::unused_type(const unsigned&)> continuousBad;
	qi::rule<Iterator, qi::unused_type(const unsigned&)> discreteBad;
	qi::rule<Iterator, std::vector<std::pair<std::string, Location<Number>*>>(), Skipper> modes;
	qi::rule<Iterator, std::set<Transition<Number>*>(), Skipper> transitions;

	void insertContinuousSymbols(const std::vector<std::string>& _in) {
		for(const auto& varName : _in ) {
			carl::Variable tmp = mVariablePool.newCarlVariable(varName);
			//std::cout << "Mapped var " << varName << " to dimension " << mVariablePool.id(tmp) << std::endl;
			mVariableSymbolTable.add(varName, mVariablePool.id(tmp));
			mVariableIds.push_back(mVariablePool.id(tmp));
			++mDimensionLimit;
			++mDiscreteDimensionLimit;
			//std::cout << "New continuous dimension: " << mDimensionLimit << std::endl;
		}
	}

	void insertDiscreteSymbols(const std::vector<std::string>& _in) {
		for(const auto& varName : _in ) {
			carl::Variable tmp = mVariablePool.newCarlVariable(varName);
			//std::cout << "Mapped var " << varName << " to dimension " << mVariablePool.id(tmp) << std::endl;
			mDiscreteVariableSymbolTable.add(varName, mVariablePool.id(tmp));
			mDiscreteVariableIds.push_back(mVariablePool.id(tmp));
			++mDiscreteDimensionLimit;
			//std::cout << "New discrete dimension: " << mDiscreteDimensionLimit << std::endl;
		}
	}

	void addContinuousState(const std::vector<matrix_t<Number>>& _constraint, unsigned id, std::vector<RawState<Number>>& _states) {
		if(_constraint.size() > 0) {
			//std::cout << "Add continuous state for location " << id << std::endl;
			bool found = false;
			// due to the creation of initial states, which uses this function as well and the occurrence of duplicates in the initial states, we need the last instance of a matching state.
			for(auto stateIt = _states.rbegin(); stateIt != _states.rend(); ++stateIt) {
				if(stateIt->location->id() == id){
					//std::cout << "stateIt->already exists." << std::endl;
					found = true;
					assert(stateIt->discreteAssignment.size() == mDiscreteVariableIds.size());
					unsigned constraintsNum = stateIt->set.first.rows();
					unsigned dimension = constraintsNum > 0 ? stateIt->set.first.cols() : (_constraint.begin()->cols())-1;
					//std::cout << "current constraints: " << boost::get<cPair<Number>>(stateIt->set).first << std::endl;
					cPair<Number> set = stateIt->set;
					//std::cout << "Resize to " << constraintsNum+_constraint.size() << " x " << dimension << std::endl;
					set.first.conservativeResize(constraintsNum+_constraint.size(), dimension);
					set.second.conservativeResize(constraintsNum+_constraint.size());
					//std::cout << "State already existing with " << constraintsNum << " rows." << std::endl;
					for(const auto& row : _constraint) {
						assert(row.rows() == 1);
						assert(row.cols() > 0);
						//std::cout << "add row " << constraintsNum;
						//std::cout << "row properties: " << row.rows() << " , " << row.cols() << std::endl;
						//std::cout << "set properties: " << set.first.rows() << " , " << set.first.cols() << std::endl;
						set.first.row(constraintsNum) = row.block(0,0,1,row.cols()-1);
						//std::cout << " " << convert<Number,double>(set.first.row(constraintsNum)) << " <= ";
						set.second(constraintsNum) = -row(0,row.cols()-1);
						//std::cout << set.second(constraintsNum) << std::endl;
						++constraintsNum;
					}
					stateIt->set = set;
					//std::cout << "New constraints: " << boost::get<cPair<Number>>(stateIt->set).first << std::endl;
					break;
				}
			}
			if(!found){
				RawState<Number> s(mLocationManager.location(id));
				std::pair<matrix_t<Number>, vector_t<Number>> set;
				set.first = matrix_t<Number>(_constraint.size(), _constraint.begin()->cols()-1);
				set.second = vector_t<Number>(_constraint.size());
				unsigned constraintsNum = 0;
				//std::cout << "State newly created with " << _constraint.size() << " rows." << std::endl;
				for(const auto& row : _constraint) {
					assert(row.rows() == 1);
					//std::cout << "add row " << constraintsNum;
					set.first.row(constraintsNum) = row.block(0,0,1,row.cols()-1);
					//std::cout << " " << set.first.row(constraintsNum) << " <= ";
					set.second(constraintsNum) = -row(0,row.cols()-1);
					//std::cout << set.second(constraintsNum) << std::endl;
					++constraintsNum;
				}
				s.set = set;
				// initial setup of the discrete assignment
				for(const auto& id : mDiscreteVariableIds ){
					s.discreteAssignment[VariablePool::getInstance().carlVarByIndex(id)] = carl::Interval<Number>::unboundedInterval();
				}
				_states.emplace_back(s);
			}
		}
	}

	void addDiscreteState(const std::pair<unsigned, carl::Interval<Number>>& _initPair, unsigned id, std::vector<RawState<Number>>& _states) {
		//std::cout << "Add discrete state for location " << id << std::endl;
		bool found = false;
		// due to the creation of initial states, which uses this function as well and the occurrence of duplicates in the initial states, we need the last instance of a matching state.
		for(auto stateIt = _states.rbegin(); stateIt != _states.rend(); ++stateIt) {
			assert(stateIt->discreteAssignment.size() == mDiscreteVariableIds.size());
			if(stateIt->location == mLocationManager.location(id)){
				found = true;
				stateIt->discreteAssignment[VariablePool::getInstance().carlVarByIndex(_initPair.first)] = _initPair.second;
			}
			assert(stateIt->discreteAssignment.size() == mDiscreteVariableIds.size());
			break;
		}
		if(!found){
			RawState<Number> s(mLocationManager.location(id));
			// initialize all discrete assignments to unbounded, when a new state is created
			for(const auto& id : mDiscreteVariableIds ){
				if(id == _initPair.first){
					s.discreteAssignment[VariablePool::getInstance().carlVarByIndex(_initPair.first)] = _initPair.second;
				} else {
					s.discreteAssignment[VariablePool::getInstance().carlVarByIndex(id)] = carl::Interval<Number>::unboundedInterval();
				}
			}
			assert(s.discreteAssignment.size() == mDiscreteVariableIds.size());
			_states.emplace_back(s);
		}
	}

	unsigned addInitState(unsigned _id, std::vector<RawState<Number>>& _states) {
		RawState<Number> s(mLocationManager.location(_id));
		// initial setup of the discrete assignment
		for(const auto& id : mDiscreteVariableIds ){
			s.discreteAssignment[VariablePool::getInstance().carlVarByIndex(id)] = carl::Interval<Number>::unboundedInterval();
		}
		assert(s.discreteAssignment.size() == mDiscreteVariableIds.size());
		_states.emplace_back(s);
		return _id;
	}

	void insertSettings(const hypro::reachability::ReachabilitySettings<Number>& _in) {
		mSettings = _in;
	}

	void insertModes(const std::vector<std::pair<std::string, Location<Number>*>>& _in) {
		for(const auto& modePair : _in) {
			//std::cout << "Found mode " << modePair.first << " mapped to " << modePair.second->id() << std::endl;
			//std::cout << "Mode: " << *modePair.second << std::endl;

			if(mModes.find(modePair.first) == nullptr) {
				mModes.add(modePair.first, modePair.second->id());
				mModeIds.push_back(modePair.second->id());
				TRACE("hypro.parser","Add new mode " << modePair.first << " mapped to id " << modePair.second->id());
			} else {
				TRACE("hypro.parser","Overwrite mode " << modePair.first << " mapped to id " << mModes.at(modePair.first) << " with mode mapped to id " << modePair.second->id());
				unsigned redundantId = mModes.at(modePair.first);
				mModes.at(modePair.first) = modePair.second->id();
				mLocationManager.erase(redundantId);
				// update mModeIds
				auto modeIdIt = mModeIds.begin();
				while(modeIdIt != mModeIds.end() && *modeIdIt != redundantId) { ++modeIdIt; }
				assert(modeIdIt != mModeIds.end());
				if(modeIdIt != mModeIds.end()) {
					mModeIds.erase(modeIdIt);
				}
				mModeIds.push_back(modePair.second->id());
			}
		}
	}

	void insertTransitions(const boost::optional<std::set<Transition<Number>*>>& _transitions) {
		if(_transitions){
			TRACE("hypro.parser","insert "<< _transitions->size() << " transitions.");
			for(const auto& transition :  *_transitions) {
				mTransitions.insert(transition);
			}
		}
	}

	HybridAutomaton<Number> parseInput( const std::string& pathToInputFile );
	bool parse( std::istream& in, HybridAutomaton<Number>& _result );
	HybridAutomaton<Number> createAutomaton();
};

} // namespace parser
} // namespace hypro

#include "flowstarParser.tpp"

#ifdef USE_CLANG
#pragma clang diagnostic pop
#endif
