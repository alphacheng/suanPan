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

#include "Stacked.h"
#include <Domain/DomainBase.h>

Stacked::Stacked(const unsigned T, uvec&& MT)
	: Material3D(T, 0.)
	, mat_tag(std::forward<uvec>(MT)) {}

Stacked::Stacked(const Stacked& old_obj)
	: Material3D(old_obj)
	, mat_tag(old_obj.mat_tag) {
	mat_pool.clear();
	mat_pool.reserve(old_obj.mat_pool.size());
	for(const auto& I : old_obj.mat_pool) mat_pool.emplace_back(I->get_copy());
}

void Stacked::initialize(const shared_ptr<DomainBase>& D) {
	initial_stiffness.zeros(6, 6);
	mat_pool.clear();
	mat_pool.reserve(mat_tag.n_elem);
	for(const auto I : mat_tag) {
		if(!D->find<Material>(I) || D->get<Material>(I)->get_material_type() != MaterialType::D3) {
			suanpan_error("Stacked requires 3D host material models.\n");
			D->disable_material(get_tag());
			return;
		}
		mat_pool.emplace_back(D->get<Material>(I)->get_copy());
		mat_pool.back()->Material::initialize(D);
		mat_pool.back()->initialize(D);
		access::rw(density) += mat_pool.back()->get_parameter(ParameterType::DENSITY);
		initial_stiffness += mat_pool.back()->get_initial_stiffness();
	}

	trial_stiffness = current_stiffness = initial_stiffness;
}

unique_ptr<Material> Stacked::get_copy() { return make_unique<Stacked>(*this); }

int Stacked::update_trial_status(const vec& t_strain) {
	incre_strain = (trial_strain = t_strain) - current_strain;

	if(norm(incre_strain) <= datum::eps) return SUANPAN_SUCCESS;

	trial_stress.zeros(6);
	trial_stiffness.zeros(6, 6);
	for(const auto& I : mat_pool) {
		if(I->update_trial_status(trial_strain) != SUANPAN_SUCCESS) return SUANPAN_FAIL;
		trial_stress += I->get_trial_stress();
		trial_stiffness += I->get_trial_stiffness();
	}

	return SUANPAN_SUCCESS;
}

int Stacked::clear_status() {
	current_strain = trial_strain.zeros();
	current_stress = trial_stress.zeros();
	current_stiffness = trial_stiffness = initial_stiffness;
	auto code = 0;
	for(const auto& I : mat_pool) code += I->clear_status();
	return code;
}

int Stacked::commit_status() {
	current_strain = trial_strain;
	current_stress = trial_stress;
	current_stiffness = trial_stiffness;
	auto code = 0;
	for(const auto& I : mat_pool) code += I->commit_status();
	return code;
}

int Stacked::reset_status() {
	trial_strain = current_strain;
	trial_stress = current_stress;
	trial_stiffness = current_stiffness;
	auto code = 0;
	for(const auto& I : mat_pool) code += I->reset_status();
	return code;
}

vector<vec> Stacked::record(const OutputType P) {
	vector<vec> data;

	auto max_size = 0llu;
	for(const auto& I : mat_pool)
		for(const auto& J : I->record(P)) {
			if(J.n_elem > max_size) max_size = J.n_elem;
			data.emplace_back(J);
		}

	for(auto&& I : data) I.resize(max_size);

	return data;
}

void Stacked::print() {
	suanpan_info("A stack wrapper for 3D problems.\n");
	unsigned t_tag = 0;
	for(const auto& I : mat_pool) {
		suanpan_info("component %u: ", ++t_tag);
		I->print();
	}
}
