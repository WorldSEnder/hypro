/*
 * The class handles the conversion from carl::Variable to Parma_Polyhedra_Library::Variable
 * and backwards. It keeps track of the created and used variables.
 * @file   VariablePool.h
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since       2014-04-02
 * @version     2014-04-03
 */

#pragma once

#include "../flags.h"
#include "logging/Logger.h"
#include <carl/core/Variable.h>
#include <carl/core/VariablePool.h>
#include <carl/util/Singleton.h>
#include <map>
#include <vector>
#include <cassert>
#ifdef HYPRO_USE_PPL
#include <ppl.hh>
#endif


namespace hypro {
class VariablePool : public carl::Singleton<VariablePool> {
	friend carl::Singleton<VariablePool>;

  private:
	/**
	 * Typedefs
	 */
	#ifdef HYPRO_USE_PPL
	using carlPplMap = std::map<carl::Variable, Parma_Polyhedra_Library::Variable>;
	using pplCarlMap = std::map<Parma_Polyhedra_Library::Variable, carl::Variable, Parma_Polyhedra_Library::Variable::Compare>;
	#endif

	/**
	 * Members
	 */
	std::vector<carl::Variable> mCarlVariables;
	#ifdef HYPRO_USE_PPL
	std::vector<Parma_Polyhedra_Library::Variable> mPplVariables;
	carlPplMap mCarlToPpl;
	pplCarlMap mPplToCarl;
	#endif
	unsigned mPplId;
	carl::VariablePool& mPool;

  protected:
	/**
	 * Default constructor and destructor
	 */
	VariablePool() : mPplId( 0 ), mPool( carl::VariablePool::getInstance() ) { mPool.clear(); }

  public:
	~VariablePool() {}

	/*
	 * Access
	 */
	#ifdef HYPRO_USE_PPL
	const carl::Variable& variable( const Parma_Polyhedra_Library::Variable& _var ) const {
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		for ( unsigned pos = 0; pos < mPplVariables.size(); ++pos ) {
			if ( _var.id() == mPplVariables[pos].id() ) return mCarlVariables[pos];
		}
		assert( false );
		return carl::Variable::NO_VARIABLE;
	}

	const Parma_Polyhedra_Library::Variable& variable( const carl::Variable& _var ) const {
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		for ( unsigned pos = 0; pos < mCarlVariables.size(); ++pos ) {
			if ( _var == mCarlVariables[pos] ) return mPplVariables[pos];
		}
		assert( false );
		return mPplVariables.back();
	}

	const Parma_Polyhedra_Library::Variable& pplVarByIndex( unsigned _index ) {
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		if ( _index >= mPplId ) {
			for ( unsigned curr = mPplId; curr <= _index; ++curr ) {
				carl::Variable cVar = carl::freshRealVariable();
				Parma_Polyhedra_Library::Variable pVar = Parma_Polyhedra_Library::Variable( mPplId++ );
				mCarlVariables.push_back( cVar );
				mPplVariables.push_back( pVar );
			}
		}
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		return mPplVariables.at( _index );
	}
	#endif

	const carl::Variable& carlVarByIndex( unsigned _index ) {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		if ( _index >= mPplId ) {
			for ( unsigned curr = mPplId; curr <= _index; ++curr ) {
				carl::Variable cVar = carl::freshRealVariable();
				mCarlVariables.push_back( cVar );
				#ifdef HYPRO_USE_PPL
				Parma_Polyhedra_Library::Variable pVar = Parma_Polyhedra_Library::Variable( mPplId++ );
				mPplVariables.push_back( pVar );
				#else
				++mPplId;
				#endif
			}
		}
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		return mCarlVariables.at( _index );
	}

	const carl::Variable& newCarlVariable( std::string _name = "" ) {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		DEBUG("hypro.variablePool", "mCarlVariables.size(): " << int(mCarlVariables.size()) << ", mPplVariables.size(): " << int(mPplVariables.size()) << ", pplId: " << int(mPplId) );
		assert( mPplVariables.size() == mPplId );
		#endif
		carl::Variable cVar = carl::freshRealVariable( _name );
		mCarlVariables.push_back( cVar );
		#ifdef HYPRO_USE_PPL
		Parma_Polyhedra_Library::Variable pVar = Parma_Polyhedra_Library::Variable( mPplId++ );
		mPplVariables.push_back( pVar );
		#else
		++mPplId;
		#endif
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		return mCarlVariables.back();
	}

	#ifdef HYPRO_USE_PPL
	const Parma_Polyhedra_Library::Variable& newPplVariable() {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		carl::Variable cVar = carl::freshRealVariable();
		Parma_Polyhedra_Library::Variable pVar = Parma_Polyhedra_Library::Variable( mPplId++ );
		mCarlVariables.push_back( cVar );
		mPplVariables.push_back( pVar );
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		return mPplVariables.back();
	}
	#endif

	bool hasDimension( const carl::Variable& _var ) const {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		for ( unsigned pos = 0; pos < mCarlVariables.size(); ++pos ) {
			if ( _var == mCarlVariables[pos] ) {
				return true;
			}
		}
		return false;
	}

	#ifdef HYPRO_USE_PPL
	int inline id( const Parma_Polyhedra_Library::Variable& _var ) const {
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		for ( unsigned pos = 0; pos < mPplVariables.size(); ++pos ) {
			if ( _var.id() == mPplVariables[pos].id() ) return pos;
		}
		return -1;
	}
	#endif

	int inline id( const carl::Variable& _var ) const {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		for ( unsigned pos = 0; pos < mCarlVariables.size(); ++pos ) {
			if ( _var == mCarlVariables[pos] ){
				return pos;
			}
		}
		return -1;
	}

	std::vector<carl::Variable> carlVariables() const {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		return mCarlVariables;
	}

	#ifdef HYPRO_USE_PPL
	std::vector<Parma_Polyhedra_Library::Variable> pplVariables() const {
		assert( mCarlVariables.size() == mPplId );
		assert( mPplVariables.size() == mPplId );
		return mPplVariables;
	}
	#endif

	unsigned size() const {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		return mCarlVariables.size();
	}

	void clear() {
		assert( mCarlVariables.size() == mPplId );
		mCarlVariables.clear();
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		mPplVariables.clear();
		#endif
		mPool.clear();
		mPplId = 0;
	}

	void print() const {
		assert( mCarlVariables.size() == mPplId );
		#ifdef HYPRO_USE_PPL
		assert( mPplVariables.size() == mPplId );
		#endif
		for ( unsigned pos = 0; pos < mCarlVariables.size(); ++pos ) {
			std::cout << mCarlVariables[pos] << " -> " << std::endl;  // mPplVariables[pos] << std::endl; //TODO: fix
		}
	}
};

} // namespace hypro
