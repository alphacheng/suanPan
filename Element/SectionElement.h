/*******************************************************************************
 * Copyright (C) 2017-2021 Theodore Chang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
/**
 * @class SectionElement
 * @brief The SectionElement class.
 * @author tlc
 * @date 31/10/2017
 * @version 0.1.0
 * @file SectionElement.h
 * @addtogroup Element
 * @{
 */

#ifndef SECTIONELEMENT_H
#define SECTIONELEMENT_H

#include <Element/Element.h>

class SectionElement : public Element {
public:
	explicit SectionElement(unsigned,   // tag
	                        unsigned,   // number of nodes
	                        unsigned,   // number of dofs
	                        uvec&&,     // node encoding
	                        uvec&&,     // section tags
	                        bool,       // nonlinear geometry switch
	                        SectionType // section type
	);
};

class SectionElement1D : public SectionElement {
public:
	explicit SectionElement1D(unsigned, // tag
	                          unsigned, // number of nodes
	                          unsigned, // number of dofs
	                          uvec&&,   // node encoding
	                          uvec&&,   // section tags
	                          bool      // nonlinear geometry switch 
	);
};

class SectionElement2D : public SectionElement {
public:
	explicit SectionElement2D(unsigned, // tag
	                          unsigned, // number of nodes
	                          unsigned, // number of dofs
	                          uvec&&,   // node encoding
	                          uvec&&,   // section tags
	                          bool      // nonlinear geometry switch 
	);
};

class SectionElement3D : public SectionElement {
public:
	explicit SectionElement3D(unsigned, // tag
	                          unsigned, // number of nodes
	                          unsigned, // number of dofs
	                          uvec&&,   // node encoding
	                          uvec&&,   // section tags
	                          bool      // nonlinear geometry switch 
	);
};

#endif

//! @}
