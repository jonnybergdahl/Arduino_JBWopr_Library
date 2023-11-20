/// @file jbwoprnokiatune.cpp
/// @author Jonny Bergdahl
/// @brief Source file for the JBWoprNokiaTuneEffect.
/// @details Contains declarations for the JBWoprNokiaTuneEffect class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprnokiatune.h"

JBWoprNokiaTuneEffect::JBWoprNokiaTuneEffect(JBWoprDevice *woprBoard,
											 uint32_t tempo,
											 uint32_t duration,
											 const std::string name)
		: JBWoprSongEffect(woprBoard,
						   &_nokiaTune,
						   tempo,
						   duration,
						   name) {}
