////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017-2021 Theodore Chang
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include "BodyForce.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Element/Element.h>
#include <Load/Amplitude/Amplitude.h>

BodyForce::BodyForce(const unsigned T, const unsigned S, const double L, uvec&& N, const unsigned D, const unsigned AT)
	: Load(T, S, AT, std::forward<uvec>(N), uvec{D}, L) {}

BodyForce::BodyForce(const unsigned T, const unsigned S, const double L, uvec&& N, uvec&& D, const unsigned AT)
	: Load(T, S, AT, std::forward<uvec>(N), std::forward<uvec>(D), L) {}

int BodyForce::process(const shared_ptr<DomainBase>& D) {
	const auto& t_factory = D->get_factory();

	auto& t_load = get_trial_load(t_factory);

	const auto final_load = pattern * magnitude->get_amplitude(t_factory->get_trial_time());

	for(const auto& I : nodes)
		if(auto& t_element = D->get<Element>(I); t_element != nullptr && t_element->is_active()) {
			vec t_body_load(t_element->get_dof_number(), fill::zeros);
			for(const auto& J : dofs) if(J <= t_element->get_dof_number()) t_body_load(J - 1) = final_load;
			const auto& t_body_force = t_element->update_body_force(t_body_load);
			if(!t_body_force.empty()) t_load(t_element->get_dof_encoding()) += t_element->update_body_force(t_body_load);
		}

	t_factory->set_incre_load(t_load - t_factory->get_current_load());

	return SUANPAN_SUCCESS;
}
