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

#include "NodalAcceleration.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Domain/Node.h>
#include <Load/Amplitude/Amplitude.h>

NodalAcceleration::NodalAcceleration(const unsigned T, const unsigned ST, const double L, const unsigned DT, const unsigned AT)
	: Load(T, ST, AT, {}, uvec{DT}, L) {}

NodalAcceleration::NodalAcceleration(const unsigned T, const unsigned ST, const double L, uvec&& DT, const unsigned AT)
	: Load(T, ST, AT, {}, std::forward<uvec>(DT), L) {}

NodalAcceleration::NodalAcceleration(const unsigned T, const unsigned ST, const double L, uvec&& NT, const unsigned DT, const unsigned AT)
	: Load(T, ST, AT, std::forward<uvec>(NT), uvec{DT}, L) {}

int NodalAcceleration::process(const shared_ptr<DomainBase>& D) {
	const auto& t_factory = D->get_factory();

	if(t_factory->get_mass() == nullptr) return SUANPAN_SUCCESS;

	vec ref_acc(t_factory->get_size(), fill::zeros);
	if(nodes.is_empty())
		for(const auto& I : D->get_node_pool()) {
			auto& t_dof = I->get_reordered_dof();
			for(const auto& J : dofs) if(J <= t_dof.n_elem) ref_acc(t_dof(J - 1)) = 1.;
		}
	else
		for(const auto& I : nodes)
			if(D->find<Node>(I))
				if(auto& t_node = D->get<Node>(I); t_node->is_active()) {
					auto& t_dof = t_node->get_reordered_dof();
					for(const auto& J : dofs) if(J <= t_dof.n_elem) ref_acc(t_dof(J - 1)) = 1.;
				}

	t_factory->update_trial_load(t_factory->get_trial_load() + t_factory->get_mass() * ref_acc * pattern * magnitude->get_amplitude(t_factory->get_trial_time()));

	return SUANPAN_SUCCESS;
}
