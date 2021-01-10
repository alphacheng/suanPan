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

#include "Integrator.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>

Integrator::Integrator(const unsigned T)
	: Tag(T) { suanpan_debug("Integrator %u ctor() called.\n", T); }

Integrator::~Integrator() { suanpan_debug("Integrator %u dtor() called.\n", get_tag()); }

void Integrator::set_domain(const weak_ptr<DomainBase>& D) { if(database.lock() != D.lock()) database = D; }

const weak_ptr<DomainBase>& Integrator::get_domain() const { return database; }

int Integrator::initialize() {
	if(database.lock() == nullptr) {
		suanpan_error("initialize() needs a valid domain.\n");
		return SUANPAN_FAIL;
	}

	return SUANPAN_SUCCESS;
}

void Integrator::set_time_step_switch(const bool T) { time_step_switch = T; }

bool Integrator::allow_to_change_time_step() const { return time_step_switch; }

int Integrator::process_load() const { return database.lock()->process_load(); }

int Integrator::process_constraint() const { return database.lock()->process_constraint(); }

int Integrator::process_criterion() const { return database.lock()->process_criterion(); }

int Integrator::process_modifier() const { return database.lock()->process_modifier(); }

void Integrator::record() const { database.lock()->record(); }

void Integrator::assemble_resistance() {
	const auto& D = database.lock();
	const auto& W = D->get_factory();
	D->assemble_resistance();
	W->set_sushi(W->get_trial_resistance());
}

void Integrator::assemble_matrix() {
	const auto& D = database.lock();
	D->assemble_trial_stiffness();
	auto& W = D->get_factory();
	if(W->get_nlgeom()) {
		D->assemble_trial_geometry();
		W->get_stiffness() += W->get_geometry();
	}
}

mat Integrator::get_force_residual() {
	const auto& W = get_domain().lock()->get_factory();

	return W->get_trial_load() - W->get_sushi();
}

mat Integrator::get_displacement_residual() {
	const auto& W = get_domain().lock()->get_factory();

	return W->get_reference_load() * W->get_trial_load_factor() + W->get_trial_load() - W->get_sushi();
}

sp_mat Integrator::get_reference_load() { return database.lock()->get_factory()->get_reference_load(); }

void Integrator::update_trial_time(const double T) const { database.lock()->get_factory()->update_trial_time(T); }

void Integrator::update_incre_time(const double T) const { database.lock()->get_factory()->update_incre_time(T); }

int Integrator::update_trial_status() { return database.lock()->update_trial_status(); }

int Integrator::update_incre_status() { return database.lock()->update_incre_status(); }

/**
 * \brief must change ninja to the real displacement increment
 * \return success flag
 */
int Integrator::update_internal(const mat&) { return 0; }

mat Integrator::solve(const mat& B) {
	mat X;
	if(solve(X, B) != SUANPAN_SUCCESS) X.reset();
	return X;
}

mat Integrator::solve(const sp_mat& B) {
	mat X;
	if(solve(X, B) != SUANPAN_SUCCESS) X.reset();
	return X;
}

mat Integrator::solve_trs(const mat& B) {
	mat X;
	if(solve_trs(X, B) != SUANPAN_SUCCESS) X.reset();
	return X;
}

mat Integrator::solve_trs(const sp_mat& B) {
	mat X;
	if(solve_trs(X, B) != SUANPAN_SUCCESS) X.reset();
	return X;
}

int Integrator::solve(mat& X, const mat& B) { return database.lock()->get_factory()->get_stiffness()->solve(X, B); }

int Integrator::solve(mat& X, const sp_mat& B) { return database.lock()->get_factory()->get_stiffness()->solve(X, B); }

int Integrator::solve_trs(mat& X, const mat& B) { return database.lock()->get_factory()->get_stiffness()->solve_trs(X, B); }

int Integrator::solve_trs(mat& X, const sp_mat& B) { return database.lock()->get_factory()->get_stiffness()->solve_trs(X, B); }

/**
 * \brief avoid machine error accumulation
 */
void Integrator::erase_machine_error() const { database.lock()->erase_machine_error(); }

void Integrator::commit_status() {
	database.lock()->commit_status();
	update_compatibility();
}

void Integrator::clear_status() {
	database.lock()->clear_status();
	update_compatibility();
}

void Integrator::reset_status() {
	database.lock()->reset_status();
	update_compatibility();
}

void Integrator::update_parameter(double) {}

void Integrator::update_compatibility() const {}
