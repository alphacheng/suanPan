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

#include "ParticleCollision.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>

vec ParticleCollision::get_postion(const shared_ptr<Node>& node) const {
	vec pos(num_dof, fill::zeros);

	auto& coor = node->get_coordinate();
	auto& t_disp = node->get_trial_displacement();

	for(auto K = 0llu; K < std::min(static_cast<uword>(num_dof), std::min(coor.n_elem, t_disp.n_elem)); ++K) pos(K) = coor(K) + t_disp(K);

	return pos;
}

ParticleCollision::ParticleCollision(const unsigned T, const unsigned S, const unsigned D)
	: Constraint(T, S, 0, {}, {})
	, num_dof(D) {}

int ParticleCollision::initialize(const shared_ptr<DomainBase>& D) {
	if(StorageScheme::FULL != D->get_factory()->get_storage_scheme() && StorageScheme::SPARSE != D->get_factory()->get_storage_scheme()) {
		suanpan_warning("DEM requires full/sparse matrix storage scheme.\n");
		D->disable_constraint(get_tag());
		return SUANPAN_SUCCESS;
	}

	return Constraint::initialize(D);
}

void ParticleCollision::apply_contact(const shared_ptr<DomainBase>& D, const shared_ptr<Node>& node_i, const shared_ptr<Node>& node_j) const {
	const auto& dof_i = node_i->get_reordered_dof();
	const auto& dof_j = node_j->get_reordered_dof();

	vec diff_pos = get_postion(node_j) - get_postion(node_i);
	const auto diff_norm = norm(diff_pos);

	diff_pos /= diff_norm;

	const auto force = compute_f(diff_norm);

	auto& W = D->get_factory();
	auto& t_stiff = W->get_stiffness();
	auto& t_load = get_trial_load(W);

	t_load(dof_i) -= force * diff_pos;
	t_load(dof_j) += force * diff_pos;

	const mat d_norm = (compute_d_f(diff_norm) - force / diff_norm) * diff_pos * diff_pos.t() + force / diff_norm * eye(num_dof, num_dof);

	if(StorageScheme::SPARSE == W->get_storage_scheme())
		for(auto K = 0u; K < num_dof; ++K)
			for(auto L = 0u; L < num_dof; ++L) {
				t_stiff->at(dof_i(K), dof_i(L)) = -d_norm(K, L);
				t_stiff->at(dof_j(K), dof_j(L)) = -d_norm(K, L);
				t_stiff->at(dof_i(K), dof_j(L)) = d_norm(K, L);
				t_stiff->at(dof_j(K), dof_i(L)) = d_norm(K, L);
			}
	else
		for(auto K = 0u; K < num_dof; ++K)
			for(auto L = 0u; L < num_dof; ++L) {
				t_stiff->at(dof_i(K), dof_i(L)) -= d_norm(K, L);
				t_stiff->at(dof_j(K), dof_j(L)) -= d_norm(K, L);
				t_stiff->at(dof_i(K), dof_j(L)) += d_norm(K, L);
				t_stiff->at(dof_j(K), dof_i(L)) += d_norm(K, L);
			}
}
