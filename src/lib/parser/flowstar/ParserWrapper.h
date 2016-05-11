#pragma once
#include <string>

namespace hypro {

	template<typename Number>
	class HybridAutomaton;

	template<typename Number>
	HybridAutomaton<Number> parseFlowstarFile(const std::string& filename);
}
