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
 * @class RigidWallPenalty
 * @brief A RigidWall class.
 *
 * @author tlc
 * @date 15/07/2020
 * @version 0.1.0
 * @file RigidWallPenalty.h
 * @addtogroup Constraint
 * @{
 */

#ifndef RIGIDWALLPENALTY_H
#define RIGIDWALLPENALTY_H

#include "Constraint.h"

class RigidWallPenalty : public Constraint {
	const double alpha;
protected:
	const vec edge_a, edge_b;
	const vec origin, norm;
public:
	RigidWallPenalty(unsigned, unsigned, unsigned, vec&&, vec&&, double);
	RigidWallPenalty(unsigned, unsigned, unsigned, vec&&, vec&&, vec&&, double);

	int initialize(const shared_ptr<DomainBase>&) override;

	int process(const shared_ptr<DomainBase>&) override;
};

#endif

//! @}
