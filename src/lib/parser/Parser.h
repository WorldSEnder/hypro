/* 
 * This file defines the parser for different input files.
 * @file   parser.h
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since   2014-05-14
 * @version 2014-05-14
 */

#pragma once

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include "boost/variant.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "Utils.h"

namespace hypro{
namespace parser{
    
namespace spirit = boost::spirit;    
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef spirit::istream_iterator BaseIteratorType;
typedef spirit::line_pos_iterator<BaseIteratorType> PositionIteratorType;
typedef PositionIteratorType Iterator;
typedef qi::space_type Skipper;
typedef std::vector<boost::variant<State, Transition> > Automaton;



template<typename Iterator>
struct MatrixParser : public qi::grammar<Iterator, std::vector<std::vector<double>>(), Skipper>
{
    MatrixParser() : MatrixParser::base_type(start)
    {
        start =  "[" >> ((qi::double_ % qi::no_skip[qi::char_(' ',',')]) % ';') >> "]";
    }
    
    qi::rule<Iterator,std::vector<std::vector<double>>(), Skipper> start;
};

template<typename Iterator>
struct StateParser : public qi::grammar<Iterator, State(), Skipper>
{
    MatrixParser<Iterator> mMatrixParser;
    
    qi::debug(start);
    
    StateParser() : StateParser::base_type(start)
    {
       start = qi::lit("states") > "{" > qi::int_ > "}." > qi::char_("AbB")  > "=" > mMatrixParser > ";";
    }
    
    qi::rule<Iterator, State(), Skipper> start;
};

template<typename Iterator>
struct TransitionParser : public qi::grammar<Iterator, Transition(), Skipper>
{
    MatrixParser<Iterator> mMatrixParser;
    
    TransitionParser(): TransitionParser::base_type(start)
    {
        start = qi::lit("transitions") > "{" > qi::int_ > "}." > qi::string("eguards_dir") > "=" > mMatrixParser > ";";
    }
    
    qi::rule<Iterator, Transition(), Skipper> start;
};

template<typename Iterator>
struct MainParser : public qi::grammar<Iterator, Automaton, Skipper>
{
    StateParser<Iterator> mStateParser;
    TransitionParser<Iterator> mTransitionParser;
    
    MainParser() : MainParser::base_type(main)
    {
        //using phoenix::construct;
        using phoenix::val;
        
        qi::debug(main);
        
        main = *(mStateParser | mTransitionParser);
        //main = *(mStateParser);
        
        qi::on_error<qi::fail>
        (
        main
        , std::cout
            << val("Error! Expecting ")
            << qi::_4                               // what failed?
            << val(" here: \"")
            //<< construct<std::string>(qi::_3, qi::_2)   // iterators to error-pos, end
            << val("\"")
            << std::endl
        );
    }    
    
    qi::rule<Iterator, Automaton, Skipper> main;
};


class HyproParser : public qi::grammar<Iterator, Automaton, Skipper>
{   
    private:
    // Rules
    MainParser<Iterator> mMainParser;
    qi::rule<Iterator, Automaton, Skipper> main;
    
    qi::debug(main);
    
    public:
    HyproParser() : HyproParser::base_type(main)
    {
        main = mMainParser;
    }    

    void parseInput(const std::string& pathToInputFile);
    bool parse(std::istream& in, const std::string& filename);
};

class Automaton_visitor
    : public boost::static_visitor<>
{
public:

    State& operator()(State& i) const
    {
        return i;
    }

    Transition& operator()(Transition& i) const
    {
        return i;
    }
};

std::ostream& operator<<(std::ostream& lhs, const Automaton& rhs)
{
    std::vector<const State*> states;
    std::vector<const Transition*> transitions;
    std::cout << rhs.size() << std::endl;
    for(auto& item : rhs)
    {
        std::cout << "item" << std::endl;
        if(const State* i = boost::get<State>(&item))
        {
            std::cout << "state..." << std::endl;
            states.insert(states.end(), i);
        }
        if(const Transition* i = boost::get<Transition>(&item))
        {
            std::cout << "Transition..." << std::endl;
            transitions.insert(transitions.end(), i);
        }
    }
    for(auto& state : states)
    {
        lhs << state;
    }
    for(auto& transition : transitions)
    {
        lhs << transition;
    }
    return lhs;
}


}
}