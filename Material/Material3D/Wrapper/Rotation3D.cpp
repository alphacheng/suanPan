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

#include "Rotation3D.h"
#include <Domain/DomainBase.h>
#include <Toolbox/Quaternion.hpp>

void Rotation3D::form_transformation(mat&& R) {
	const auto &R1 = R(0, 0), &R2 = R(0, 1), &R3 = R(0, 2);
	const auto &R4 = R(1, 0), &R5 = R(1, 1), &R6 = R(1, 2);
	const auto &R7 = R(2, 0), &R8 = R(2, 1), &R9 = R(2, 2);

	trans_mat.set_size(6, 6);

	trans_mat(span(0, 2), span(0, 2)) = square(R);

	trans_mat(0, 3) = R1 * R2;
	trans_mat(0, 4) = R2 * R3;
	trans_mat(0, 5) = R3 * R1;

	trans_mat(1, 3) = R4 * R5;
	trans_mat(1, 4) = R5 * R6;
	trans_mat(1, 5) = R6 * R4;

	trans_mat(2, 3) = R7 * R8;
	trans_mat(2, 4) = R8 * R9;
	trans_mat(2, 5) = R9 * R7;

	trans_mat(3, 0) = 2. * R1 * R4;
	trans_mat(3, 1) = 2. * R2 * R5;
	trans_mat(3, 2) = 2. * R3 * R6;
	trans_mat(3, 3) = R1 * R5 + R2 * R4;
	trans_mat(3, 4) = R2 * R6 + R3 * R5;
	trans_mat(3, 5) = R3 * R4 + R1 * R6;

	trans_mat(4, 0) = 2. * R4 * R7;
	trans_mat(4, 1) = 2. * R5 * R8;
	trans_mat(4, 2) = 2. * R6 * R9;
	trans_mat(4, 3) = R4 * R8 + R5 * R7;
	trans_mat(4, 4) = R5 * R9 + R6 * R8;
	trans_mat(4, 5) = R6 * R7 + R4 * R9;

	trans_mat(5, 0) = 2. * R1 * R7;
	trans_mat(5, 1) = 2. * R2 * R8;
	trans_mat(5, 2) = 2. * R3 * R9;
	trans_mat(5, 3) = R1 * R8 + R2 * R7;
	trans_mat(5, 4) = R2 * R9 + R3 * R8;
	trans_mat(5, 5) = R3 * R7 + R1 * R9;
}

Rotation3D::Rotation3D(const unsigned T, const unsigned MT, const double I, const double J, const double K)
	: Material3D(T, 0.)
	, mat_tag(MT) {
	const auto N = sqrt(I * I + J * J + K * K);
	const auto S = sin(.5 * N) / N;
	const auto C = cos(.5 * N);

	form_transformation(Quaternion<double>(C, I * S, J * S, K * S).to_mat());
}

Rotation3D::Rotation3D(const unsigned T, const unsigned MT, mat&& R)
	: Material3D(T, 0.)
	, mat_tag(MT) { form_transformation(std::forward<mat>(R)); }

Rotation3D::Rotation3D(const Rotation3D& old_obj)
	: Material3D(old_obj)
	, mat_tag(old_obj.mat_tag)
	, mat_obj(nullptr == old_obj.mat_obj ? nullptr : old_obj.mat_obj->get_copy())
	, trans_mat(old_obj.trans_mat) {}

void Rotation3D::initialize(const shared_ptr<DomainBase>& D) {
	if(!D->find_material(mat_tag) || D->get_material(mat_tag)->get_material_type() != MaterialType::D3) {
		suanpan_error("Rotation3D requires a 3D host material model.\n");
		D->disable_material(get_tag());
		return;
	}

	mat_obj = D->get_material(mat_tag)->get_copy();

	mat_obj->Material::initialize(D);
	mat_obj->initialize(D);
	access::rw(density) = mat_obj->get_parameter(ParameterType::DENSITY);

	trial_stiffness = current_stiffness = initial_stiffness = trans_mat.t() * mat_obj->get_initial_stiffness() * trans_mat;
}

unique_ptr<Material> Rotation3D::get_copy() { return make_unique<Rotation3D>(*this); }

int Rotation3D::update_trial_status(const vec& t_strain) {
	incre_strain = (trial_strain = t_strain) - current_strain;

	if(norm(incre_strain) <= datum::eps) return SUANPAN_SUCCESS;

	if(mat_obj->update_trial_status(trans_mat * trial_strain) != SUANPAN_SUCCESS) return SUANPAN_FAIL;

	trial_stress = trans_mat.t() * mat_obj->get_trial_stress();
	trial_stiffness = trans_mat.t() * mat_obj->get_trial_stiffness() * trans_mat;

	return SUANPAN_SUCCESS;
}

int Rotation3D::clear_status() {
	current_strain.zeros();
	trial_strain.zeros();
	current_stress.zeros();
	trial_stress.zeros();
	trial_stiffness = current_stiffness = initial_stiffness;
	return mat_obj->clear_status();
}

int Rotation3D::commit_status() {
	current_strain = trial_strain;
	current_stress = trial_stress;
	current_stiffness = trial_stiffness;
	return mat_obj->commit_status();
}

int Rotation3D::reset_status() {
	trial_strain = current_strain;
	trial_stress = current_stress;
	trial_stiffness = current_stiffness;
	return mat_obj->reset_status();
}

vector<vec> Rotation3D::record(const OutputType P) { return mat_obj->record(P); }
