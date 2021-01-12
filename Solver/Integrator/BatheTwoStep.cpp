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

#include "BatheTwoStep.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Domain/Node.h>

void BatheTwoStep::assemble_resistance() {
	const auto& D = get_domain().lock();
	auto& W = D->get_factory();

	D->assemble_resistance();
	D->assemble_inertial_force();
	D->assemble_damping_force();

	W->set_sushi(W->get_trial_resistance() + W->get_trial_damping_force() + W->get_trial_inertial_force());
}

void BatheTwoStep::assemble_matrix() {
	const auto& D = get_domain().lock();
	auto& W = D->get_factory();

	D->assemble_trial_stiffness();
	D->assemble_trial_mass();
	D->assemble_trial_damping();

	auto& t_stiff = W->get_stiffness();

	if(W->get_nlgeom()) {
		D->assemble_trial_geometry();
		t_stiff += W->get_geometry();
	}

	t_stiff += FLAG::TRAP == step_flag ? C6 * W->get_mass() + C3 * W->get_damping() : C5 * W->get_mass() + C2 * W->get_damping();
}

int BatheTwoStep::update_trial_status() {
	const auto& D = get_domain().lock();
	auto& W = D->get_factory();

	if(FLAG::TRAP == step_flag) {
		W->update_incre_acceleration(C6 * W->get_incre_displacement() - C4 * W->get_current_velocity() - 2. * W->get_current_acceleration());
		W->update_incre_velocity(C3 * W->get_incre_displacement() - 2. * W->get_current_velocity());
	} else {
		W->update_trial_velocity(C2 * W->get_incre_displacement() + C1 * (W->get_pre_displacement() - W->get_current_displacement()));
		W->update_trial_acceleration(C1 * W->get_pre_velocity() - C3 * W->get_current_velocity() + C2 * W->get_trial_velocity());
	}

	return D->update_trial_status();
}

void BatheTwoStep::commit_status() {
	const auto& D = get_domain().lock();
	auto& W = D->get_factory();

	if(FLAG::TRAP == step_flag) {
		step_flag = FLAG::EULER;
		set_time_step_switch(false);
	} else {
		step_flag = FLAG::TRAP;
		set_time_step_switch(true);
	}

	W->commit_pre_displacement();
	W->commit_pre_velocity();

	Integrator::commit_status();
}

void BatheTwoStep::clear_status() {
	step_flag = FLAG::TRAP;
	set_time_step_switch(true);

	Integrator::clear_status();
}

/**
 * \brief update acceleration and velocity for zero displacement increment
 */
void BatheTwoStep::update_compatibility() const {
	const auto& D = get_domain().lock();
	auto& W = D->get_factory();

	if(FLAG::TRAP == step_flag) {
		W->update_incre_acceleration(-C4 * W->get_current_velocity() - 2. * W->get_current_acceleration());
		W->update_incre_velocity(-2. * W->get_current_velocity());
	} else {
		W->update_trial_velocity(C1 * (W->get_pre_displacement() - W->get_current_displacement()));
		W->update_trial_acceleration(C1 * W->get_pre_velocity() - C3 * W->get_current_velocity() + C2 * W->get_trial_velocity());
	}

	auto& trial_dsp = W->get_trial_displacement();
	auto& trial_vel = W->get_trial_velocity();
	auto& trial_acc = W->get_trial_acceleration();

	auto& t_node_pool = D->get_node_pool();

	suanpan_for_each(t_node_pool.cbegin(), t_node_pool.cend(), [&](const shared_ptr<Node>& t_node) { t_node->update_trial_status(trial_dsp, trial_vel, trial_acc); });
}

void BatheTwoStep::update_parameter(const double NT) {
	if(fabs(C0 - NT) <= 1E-12) return;

	C0 = NT;
	C1 = .5 / C0;
	C2 = 3. * C1;
	C3 = 4. * C1;
	C4 = 2. * C3;
	C5 = C2 * C2;
	C6 = C4 / C0;
}

void BatheTwoStep::print() { suanpan_info("A BatheTwoStep solver.\n"); }
