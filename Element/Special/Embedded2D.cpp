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

#include "Embedded2D.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Toolbox/shapeFunction.h>

Embedded2D::Embedded2D(const unsigned T, const unsigned ET, const unsigned NT, const double P)
	: Element(T, e_dof, ET, NT)
	, host_tag(ET)
	, alpha(P) {}

void Embedded2D::initialize(const shared_ptr<DomainBase>& D) {
	host_element = D->get<Element>(host_tag);

	if(nullptr == host_element || !host_element->is_active()) {
		D->disable_element(get_tag());
		return;
	}

	access::rw(host_size) = host_element->get_node_number();

	if(4 != host_size) {
		D->disable_element(get_tag());
		return;
	}

	const auto o_coor = get_coordinate(2);
	const mat t_coor = o_coor.tail_rows(host_size);
	const vec n_coor = o_coor.row(0).t();

	vec t_para = zeros(2);

	auto& n = access::rw(iso_n);

	unsigned counter = 0;
	while(++counter <= max_iteration) {
		const vec incre = solve((shape::quad(t_para, 1, 4) * t_coor).t(), n_coor - ((n = shape::quad(t_para, 0, 4)) * t_coor).t());
		if(norm(incre) < 1E-14) break;
		t_para += incre;
	}

	if(max_iteration == counter) D->disable_element(get_tag());

	const auto mean_x = mean(t_coor.col(0));
	const auto mean_y = mean(t_coor.col(1));

	mat jacobian(2, host_size);
	jacobian.row(0).fill(1.);
	jacobian.row(1) = (t_coor.col(0) - mean_x).t();

	vec right(2);
	right(0) = 1.;
	right(1) = n_coor(0) - mean_x;

	access::rw(weight_y) = solve(jacobian, right);

	jacobian.row(1) = (t_coor.col(1) - mean_y).t();
	right(1) = n_coor(1) - mean_y;

	access::rw(weight_x) = solve(jacobian, right);
}

int Embedded2D::update_status() {
	const auto t_disp = get_trial_displacement();

	const vec reaction = alpha * (t_disp.head(2) - reshape(t_disp.tail(t_disp.n_elem - 2), e_dof, host_size) * iso_n.t());

	trial_resistance.set_size(get_total_number());
	trial_stiffness.zeros(get_total_number(), get_total_number());

	const auto idx_x = linspace<uvec>(2llu, 2llu + e_dof * (host_size - 1llu), host_size);
	const auto idx_y = linspace<uvec>(3llu, 3llu + e_dof * (host_size - 1llu), host_size);

	trial_resistance.head(2) = -reaction;
	trial_resistance(idx_x) = weight_x * reaction(0);
	trial_resistance(idx_y) = weight_y * reaction(1);

	trial_stiffness(0, 0) = -alpha;
	trial_stiffness(uvec{0}, idx_x) = alpha * iso_n;
	trial_stiffness(idx_x, uvec{0}) = alpha * weight_x;
	trial_stiffness(idx_x, idx_x) = -alpha * weight_x * iso_n;

	trial_stiffness(1, 1) = -alpha;
	trial_stiffness(uvec{1}, idx_y) = alpha * iso_n;
	trial_stiffness(idx_y, uvec{1}) = alpha * weight_y;
	trial_stiffness(idx_y, idx_y) = -alpha * weight_y * iso_n;

	return SUANPAN_SUCCESS;
}

int Embedded2D::clear_status() {
	trial_resistance.reset();
	trial_stiffness.reset();

	current_resistance = trial_resistance;
	current_stiffness = trial_stiffness;

	return SUANPAN_SUCCESS;
}

int Embedded2D::commit_status() {
	current_resistance = trial_resistance;
	current_stiffness = trial_stiffness;

	return SUANPAN_SUCCESS;
}

int Embedded2D::reset_status() {
	trial_resistance = current_resistance;
	trial_stiffness = current_stiffness;

	return SUANPAN_SUCCESS;
}
