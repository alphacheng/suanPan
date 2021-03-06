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

#include "NonlinearPeric.h"
#include <Toolbox/tensorToolbox.h>
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>

const double NonlinearPeric::root_three_two = sqrt(1.5);
const mat NonlinearPeric::unit_dev_tensor = tensor::unit_deviatoric_tensor4();
const double NonlinearPeric::unit_time = 1.;

NonlinearPeric::NonlinearPeric(const unsigned T, const double E, const double V, const double MU, const double EPS, const double R)
	: DataNonlinearPeric{E, V, MU, EPS}
	, Material3D(T, R) {}

void NonlinearPeric::initialize(const shared_ptr<DomainBase>& D) {
	incre_time = D == nullptr ? &unit_time : &D->get_factory()->get_incre_time();

	trial_stiffness = current_stiffness = initial_stiffness = tensor::isotropic_stiffness(elastic_modulus, poissons_ratio);

	initialize_history(1);
}

double NonlinearPeric::get_parameter(const ParameterType P) const {
	if(ParameterType::DENSITY == P) return density;
	if(ParameterType::ELASTICMODULUS == P || ParameterType::YOUNGSMODULUS == P || ParameterType::E == P) return elastic_modulus;
	if(ParameterType::SHEARMODULUS == P || ParameterType::G == P) return shear_modulus;
	if(ParameterType::POISSONSRATIO == P) return poissons_ratio;
	return 0.;
}

int NonlinearPeric::update_trial_status(const vec& t_strain) {
	trial_stress = current_stress + (trial_stiffness = initial_stiffness) * (incre_strain = (trial_strain = t_strain) - current_strain);

	trial_history = current_history;
	auto& plastic_strain = trial_history(0);

	const auto dev_stress = tensor::dev(trial_stress);
	const auto eqv_stress = root_three_two * tensor::stress::norm(dev_stress);

	auto residual = eqv_stress - std::max(0., compute_k(plastic_strain));

	if(residual < 0.) return SUANPAN_SUCCESS;

	auto gamma = 0., dk = 0., pow_term = 1., denom = *incre_time;
	unsigned counter = 0;
	while(++counter < max_iteration) {
		const auto gradient = (triple_shear + factor_a * (eqv_stress - triple_shear * gamma) / denom) * pow_term - (dk = compute_dk(plastic_strain));
		const auto incre_gamma = residual / gradient;
		suanpan_extra_debug("NonlinearPeric local iteration error: %.5E.\n", incre_gamma);
		if(fabs(incre_gamma) <= tolerance) break;
		plastic_strain = current_history(0) + (gamma += incre_gamma);
		denom = *incre_time + mu * gamma;
		pow_term = pow(*incre_time / denom, epsilon);
		residual = (eqv_stress - triple_shear * gamma) * pow_term - std::max(0., compute_k(plastic_strain));
	}

	if(max_iteration == counter) {
		suanpan_error("NonlinearPeric cannot converge within %u iterations.\n", max_iteration);
		return SUANPAN_FAIL;
	}

	trial_stress -= gamma * triple_shear / eqv_stress * dev_stress;

	trial_stiffness += triple_shear * triple_shear * (gamma / eqv_stress - 1. / (triple_shear + dk / pow_term + factor_a / denom * (eqv_stress - triple_shear * gamma))) / eqv_stress / eqv_stress * dev_stress * dev_stress.t() - triple_shear * double_shear * gamma / eqv_stress * unit_dev_tensor;

	return SUANPAN_SUCCESS;
}

int NonlinearPeric::clear_status() {
	current_strain.zeros();
	current_stress.zeros();
	current_history = initial_history;
	current_stiffness = initial_stiffness;
	return reset_status();
}

int NonlinearPeric::commit_status() {
	current_strain = trial_strain;
	current_stress = trial_stress;
	current_history = trial_history;
	current_stiffness = trial_stiffness;
	return SUANPAN_SUCCESS;
}

int NonlinearPeric::reset_status() {
	trial_strain = current_strain;
	trial_stress = current_stress;
	trial_history = current_history;
	trial_stiffness = current_stiffness;
	return SUANPAN_SUCCESS;
}

void NonlinearPeric::print() { suanpan_info("A 3D bilinear hardening viscoplasticity model using Perzyna rule.\n"); }
