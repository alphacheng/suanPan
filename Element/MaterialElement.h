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
 * @class MaterialElement
 * @brief The MaterialElement class.
 * @author tlc
 * @date 31/10/2017
 * @version 0.1.0
 * @file MaterialElement.h
 * @addtogroup Element
 * @{
 */

#ifndef MATERIALELEMENT_H
#define MATERIALELEMENT_H

#include <Element/Element.h>

class MaterialElement : public Element {
public:
	explicit MaterialElement(unsigned,    // tag
	                         unsigned,    // number of nodes
	                         unsigned,    // number of dofs
	                         uvec&&,      // node encoding
	                         uvec&&,      // material tags
	                         bool,        // nonlinear geometry switch
	                         MaterialType // material type
	);
};

class MaterialElement1D : public MaterialElement {
public:
	explicit MaterialElement1D(unsigned, // tag
	                           unsigned, // number of nodes
	                           unsigned, // number of dofs
	                           uvec&&,   // node encoding
	                           uvec&&,   // material tags
	                           bool      // nonlinear geometry switch
	);

};

class MaterialElement2D : public MaterialElement {
public:
	explicit MaterialElement2D(unsigned, // tag
	                           unsigned, // number of nodes
	                           unsigned, // number of dofs
	                           uvec&&,   // node encoding
	                           uvec&&,   // material tags
	                           bool      // nonlinear geometry switch
	);

};

class MaterialElement3D : public MaterialElement {
public:
	explicit MaterialElement3D(unsigned = 0, // tag
	                           unsigned = 0, // number of nodes
	                           unsigned = 0, // number of dofs
	                           uvec&& = {},  // node encoding
	                           uvec&& = {},  // material tags
	                           bool = false  // nonlinear geometry switch 
	);

};

#endif

//! @}
