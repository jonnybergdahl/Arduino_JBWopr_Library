/// @file therickeffect.cpp
/// @author Jonny Bergdahl
/// @brief Header file for the JBWopr library.
/// @details Contains declarations for the JBWoprTheRickEffect class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-11-15
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprtherickeffect.h"

// ============================================
//
// JBWoprTheRickEffect
//
JBWoprTheRickEffect::JBWoprTheRickEffect(JBWoprDevice *woprBoard,
										 uint32_t tempo,
										 uint32_t duration,
										 const std::string name)
		: JBWoprSongEffect(woprBoard,
						   &_theRick,
						   tempo,
						   duration,
						   name) {}
