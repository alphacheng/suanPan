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

#include "Degradation.h"
#include <Domain/DomainBase.h>

Degradation::Degradation(const unsigned T, const unsigned MT)
	: Material1D(T, 0.)
	, mat_tag(MT) {}

Degradation::Degradation(const Degradation& old_obj)
	: Material1D(old_obj)
	, mat_tag(old_obj.mat_tag)
	, base(old_obj.base == nullptr ? nullptr : old_obj.base->get_copy()) {}

void Degradation::initialize(const shared_ptr<DomainBase>& D) {
	if(nullptr == D || !D->find_material(mat_tag)) {
		D->disable_material(get_tag());
		return;
	}

	base = D->get_material(mat_tag)->get_copy();

	base->Material::initialize(D);
	base->initialize(D);

	access::rw(density) = base->get_parameter();

	const auto degrade = compute_degradation(0.);
	const auto& d = degrade(0);

	trial_stiffness = current_stiffness = initial_stiffness = d * base->get_initial_stiffness();
}

int Degradation::update_trial_status(const vec& t_strain) {
	trial_strain = t_strain;

	const auto code = base->update_trial_status(t_strain);

	if(SUANPAN_SUCCESS == code) {
		const auto degrade = compute_degradation(t_strain(0));
		const auto& d = degrade(0);
		const auto& dd = degrade(1);

		trial_stress = d * base->get_trial_stress();

		trial_stiffness = d * base->get_trial_stiffness() + dd * base->get_trial_stress();
	}

	return code;
}

int Degradation::clear_status() {
	current_strain.zeros();
	current_stress.zeros();
	current_stiffness = initial_stiffness;
	return base->clear_status();
}

int Degradation::commit_status() {
	current_strain = trial_strain;
	current_stress = trial_stress;
	current_stiffness = trial_stiffness;
	return base->commit_status();
}

int Degradation::reset_status() {
	trial_strain = current_strain;
	trial_stress = current_stress;
	trial_stiffness = current_stiffness;
	return base->reset_status();
}
