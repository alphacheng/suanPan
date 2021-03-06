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

#include "Material.h"

Material::Material(const unsigned T, const MaterialType MT, const double D)
	: MaterialData{1E-14, fabs(D), MT, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}}
	, Tag(T) { suanpan_debug("Material %u ctor() called.\n", T); }

Material::~Material() { suanpan_debug("Material %u dtor() called.\n", get_tag()); }

void Material::initialize(const shared_ptr<DomainBase>&) {
	if(initialized) return;

	const auto size = static_cast<unsigned>(material_type);

	if(current_strain.is_empty()) current_strain.zeros(size);
	if(trial_strain.is_empty()) trial_strain.zeros(size);

	if(current_stress.is_empty()) current_stress.zeros(size);
	if(trial_stress.is_empty()) trial_stress.zeros(size);

	if(initial_stiffness.is_empty()) initial_stiffness.zeros(size, size);
	if(current_stiffness.is_empty()) current_stiffness.zeros(size, size);
	if(trial_stiffness.is_empty()) trial_stiffness.zeros(size, size);
}

void Material::initialize_history(const unsigned size) {
	if(initial_history.empty()) initial_history.zeros(size);
	else if(static_cast<uword>(size) > initial_history.size()) initial_history.resize(size);

	trial_history = current_history = initial_history;
}

void Material::set_initial_history(const vec& H) { initial_history = H; }

void Material::set_initialized(const bool F) const { access::rw(initialized) = F; }

void Material::set_symmetric(const bool F) const { access::rw(symmetric) = F; }

bool Material::is_initialized() const { return initialized; }

bool Material::is_symmetric() const { return symmetric; }

MaterialType Material::get_material_type() const { return material_type; }

double Material::get_parameter(const ParameterType T) const { return T == ParameterType::DENSITY ? density : 0.; }

const vec& Material::get_trial_strain() { return trial_strain; }

const vec& Material::get_trial_strain_rate() { return trial_strain_rate; }

const vec& Material::get_trial_stress() { return trial_stress; }

const mat& Material::get_trial_stiffness() { return trial_stiffness; }

const mat& Material::get_trial_secant() {
	// BFGS type secant stiffness update
	const vec elastic_stress = current_stiffness * trial_strain;
	trial_stiffness = current_stiffness + trial_stress * trial_stress.t() / dot(trial_stress, trial_strain) - elastic_stress * elastic_stress.t() / dot(trial_strain, elastic_stress);

	return trial_stiffness;
}

const mat& Material::get_trial_damping() { return trial_damping; }

const vec& Material::get_current_strain() { return current_strain; }

const vec& Material::get_current_strain_rate() { return current_strain_rate; }

const vec& Material::get_current_stress() { return current_stress; }

const mat& Material::get_current_stiffness() { return current_stiffness; }

const mat& Material::get_current_secant() { return current_stiffness; }

const mat& Material::get_current_damping() { return current_damping; }

const vec& Material::get_initial_history() const { return initial_history; }

const mat& Material::get_initial_stiffness() const { return initial_stiffness; }

const mat& Material::get_initial_damping() const { return initial_damping; }

unique_ptr<Material> Material::get_copy() { throw invalid_argument("hidden method get_copy() called"); }

int Material::update_incre_status(const double i_strain) {
	const vec i_vec_strain{i_strain};
	return update_incre_status(i_vec_strain);
}

int Material::update_incre_status(const double i_strain, const double i_strain_rate) {
	const vec i_vec_strain{i_strain};
	const vec i_vec_strain_rate{i_strain_rate};
	return update_incre_status(i_vec_strain, i_vec_strain_rate);
}

int Material::update_trial_status(const double t_strain) {
	const vec t_vec_strain{t_strain};
	return update_trial_status(t_vec_strain);
}

int Material::update_trial_status(const double t_strain, const double t_strain_rate) {
	const vec t_vec_strain{t_strain};
	const vec t_vec_strain_rate{t_strain_rate};
	return update_trial_status(t_vec_strain, t_vec_strain_rate);
}

int Material::update_incre_status(const vec& i_strain) { return update_trial_status(current_strain + i_strain); }

int Material::update_incre_status(const vec& i_strain, const vec& i_strain_rate) { return update_trial_status(current_strain + i_strain, current_strain_rate + i_strain_rate); }

int Material::update_trial_status(const vec&) { throw invalid_argument("hidden method update_trial_status() called"); }

int Material::update_trial_status(const vec& t_strain, const vec& t_strain_rate) {
	trial_strain_rate = t_strain_rate;
	return update_trial_status(t_strain);
}

int Material::clear_status() {
	if(!current_strain.is_empty()) current_strain.zeros();
	if(!current_strain_rate.is_empty()) current_strain_rate.zeros();
	if(!current_stress.is_empty()) current_stress.zeros();

	if(!trial_strain.is_empty()) trial_strain.zeros();
	if(!trial_strain_rate.is_empty()) trial_strain_rate.zeros();
	if(!trial_stress.is_empty()) trial_stress.zeros();

	if(initial_history.is_empty()) {
		if(!current_history.is_empty()) current_history.zeros();
		if(!trial_history.is_empty()) trial_history.zeros();
	} else trial_history = current_history = initial_history;

	if(!initial_stiffness.is_empty()) trial_stiffness = current_stiffness = initial_stiffness;

	return SUANPAN_SUCCESS;
}

int Material::commit_status() {
	if(!trial_strain.is_empty()) current_strain = trial_strain;
	if(!trial_strain_rate.is_empty()) current_strain_rate = trial_strain_rate;
	if(!trial_stress.is_empty()) current_stress = trial_stress;
	if(!trial_history.is_empty()) current_history = trial_history;
	if(!trial_stiffness.is_empty()) current_stiffness = trial_stiffness;

	return SUANPAN_SUCCESS;
}

int Material::reset_status() {
	if(!trial_strain.is_empty()) trial_strain = current_strain;
	if(!trial_strain_rate.is_empty()) trial_strain_rate = current_strain_rate;
	if(!trial_stress.is_empty()) trial_stress = current_stress;
	if(!trial_history.is_empty()) trial_history = current_history;
	if(!trial_stiffness.is_empty()) trial_stiffness = current_stiffness;

	return SUANPAN_SUCCESS;
}

vector<vec> Material::record(const OutputType) { return {}; }

void ConstantDamping(MaterialData* M) {
	M->current_damping = mat(M->initial_damping.memptr(), M->initial_damping.n_rows, M->initial_damping.n_cols, false, true);
	M->trial_damping = mat(M->initial_damping.memptr(), M->initial_damping.n_rows, M->initial_damping.n_cols, false, true);
}

void ConstantStiffness(MaterialData* M) {
	M->current_stiffness = mat(M->initial_stiffness.memptr(), M->initial_stiffness.n_rows, M->initial_stiffness.n_cols, false, true);
	M->trial_stiffness = mat(M->initial_stiffness.memptr(), M->initial_stiffness.n_rows, M->initial_stiffness.n_cols, false, true);
}

void PureWrapper(MaterialData* M) {
	M->current_strain.reset();
	M->current_strain_rate.reset();
	M->current_stress.reset();

	M->trial_strain.reset();
	M->trial_strain_rate.reset();
	M->trial_stress.reset();

	M->incre_strain.reset();
	M->incre_strain_rate.reset();
	M->incre_stress.reset();

	M->initial_history.reset();
	M->current_history.reset();
	M->trial_history.reset();

	M->initial_stiffness.reset();
	M->current_stiffness.reset();
	M->trial_stiffness.reset();

	M->initial_damping.reset();
	M->current_damping.reset();
	M->trial_damping.reset();
}

unique_ptr<Material> suanpan::make_copy(const shared_ptr<Material>& P) { return nullptr == P ? nullptr : P->get_copy(); }

unique_ptr<Material> suanpan::make_copy(const unique_ptr<Material>& P) { return nullptr == P ? nullptr : P->get_copy(); }
