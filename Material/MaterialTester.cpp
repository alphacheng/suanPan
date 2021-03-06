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

#include "MaterialTester.h"

mat material_tester(const shared_ptr<Material>& obj, const vector<unsigned>& idx, const vec& incre) {
	if(!obj->is_initialized()) {
		obj->Material::initialize(nullptr);
		obj->initialize(nullptr);
		obj->set_initialized(true);
	}

	if(obj->get_material_type() == MaterialType::D1 && incre.n_elem != 1) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D2 && incre.n_elem != 3) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D3 && incre.n_elem != 6) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}

	unsigned total_size = 1;
	for(const auto& I : idx) total_size += I;

	mat response(total_size, 2 * incre.n_elem, fill::zeros);

	const span span_a(0, incre.n_elem - 1);
	const span span_b(incre.n_elem, 2 * incre.n_elem - 1);

	auto current_pos = 0;

	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	auto flag = SUANPAN_SUCCESS;
	auto incre_strain = incre;
	for(const auto& I : idx) {
		for(unsigned J = 0; J < I; ++J) {
			if((flag = obj->update_incre_status(incre_strain)) != SUANPAN_SUCCESS) break;
			obj->commit_status();
			response(current_pos, span_a) = obj->get_current_strain().t();
			response(current_pos++, span_b) = obj->get_current_stress().t();
		}
		if(SUANPAN_SUCCESS != flag) break;
		incre_strain = -incre_strain;
	}

	obj->print();
	obj->reset_status();
	obj->clear_status();

	return response;
}

mat material_tester(const shared_ptr<Material>& obj, const vector<unsigned>& idx, const vec& incre, const vec& base) {
	if(!obj->is_initialized()) {
		obj->Material::initialize(nullptr);
		obj->initialize(nullptr);
		obj->set_initialized(true);
	}

	if(obj->get_material_type() == MaterialType::D1 && incre.n_elem != 1) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D2 && incre.n_elem != 3) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D3 && incre.n_elem != 6) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}

	unsigned total_size = 2;
	for(const auto& I : idx) total_size += I;

	mat response(total_size, 2 * incre.n_elem, fill::zeros);

	const span span_a(0, incre.n_elem - 1);
	const span span_b(incre.n_elem, 2 * incre.n_elem - 1);

	auto current_pos = 0;

	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	if(obj->update_incre_status(base) != SUANPAN_SUCCESS) return response;

	obj->commit_status();

	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	auto flag = SUANPAN_SUCCESS;
	auto incre_strain = incre;
	for(const auto& I : idx) {
		for(unsigned J = 0; J < I; ++J) {
			if((flag = obj->update_incre_status(incre_strain)) != SUANPAN_SUCCESS) break;
			obj->commit_status();
			response(current_pos, span_a) = obj->get_current_strain().t();
			response(current_pos++, span_b) = obj->get_current_stress().t();
		}
		if(SUANPAN_SUCCESS != flag) break;
		incre_strain = -incre_strain;
	}

	obj->print();
	obj->reset_status();
	obj->clear_status();

	return response;
}

mat material_tester_by_load(const shared_ptr<Material>& obj, const vector<unsigned>& idx, const vec& incre) {
	if(!obj->is_initialized()) {
		obj->Material::initialize(nullptr);
		obj->initialize(nullptr);
		obj->set_initialized(true);
	}

	if(obj->get_material_type() == MaterialType::D1 && incre.n_elem != 1) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D2 && incre.n_elem != 3) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D3 && incre.n_elem != 6) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}

	unsigned total_size = 1;
	for(const auto& I : idx) total_size += I;

	mat response(total_size, 2 * incre.n_elem, fill::zeros);

	const span span_a(0, incre.n_elem - 1);
	const span span_b(incre.n_elem, 2 * incre.n_elem - 1);

	auto current_pos = 0;

	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	auto info = SUANPAN_SUCCESS;
	auto incre_load = incre;
	vec total_load = zeros(size(incre));
	for(const auto& I : idx) {
		for(unsigned J = 0; J < I; ++J) {
			total_load += incre_load;
			auto counter = 0;
			while(true) {
				const vec incre_strain = solve(obj->get_trial_stiffness(), total_load - obj->get_trial_stress());
				const auto error = norm(incre_strain);
				suanpan_info("local iteration error: %.5E.\n", error);
				if(error < 1E-12) break;
				if(++counter == 10 || obj->update_trial_status(obj->get_trial_strain() + incre_strain) != SUANPAN_SUCCESS) {
					info = SUANPAN_FAIL;
					break;
				}
			}
			if(SUANPAN_FAIL == info) break;
			obj->commit_status();
			response(current_pos, span_a) = obj->get_current_strain().t();
			response(current_pos++, span_b) = obj->get_current_stress().t();
		}
		if(SUANPAN_FAIL == info) break;
		incre_load = -incre_load;
	}

	obj->print();
	obj->reset_status();
	obj->clear_status();

	return response;
}

mat material_tester_by_load(const shared_ptr<Material>& obj, const vector<unsigned>& idx, const vec& incre, const vec& base) {
	if(!obj->is_initialized()) {
		obj->Material::initialize(nullptr);
		obj->initialize(nullptr);
		obj->set_initialized(true);
	}

	if(obj->get_material_type() == MaterialType::D1 && incre.n_elem != 1) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D2 && incre.n_elem != 3) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}
	if(obj->get_material_type() == MaterialType::D3 && incre.n_elem != 6) {
		suanpan_error("the tester cannot be appiled to the given material model.\n");
		return {};
	}

	unsigned total_size = 2;
	for(const auto& I : idx) total_size += I;

	mat response(total_size, 2 * incre.n_elem, fill::zeros);

	const span span_a(0, incre.n_elem - 1);
	const span span_b(incre.n_elem, 2 * incre.n_elem - 1);

	auto current_pos = 0;

	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	auto incre_load = incre;
	auto total_load = base;
	auto info = SUANPAN_SUCCESS;
	auto counter = 0;
	while(true) {
		const vec incre_strain = solve(obj->get_trial_stiffness(), total_load - obj->get_trial_stress());
		const auto error = norm(incre_strain);
		suanpan_info("local iteration error: %.5E.\n", error);
		if(error < 1E-12) break;
		if(++counter == 10 || obj->update_trial_status(obj->get_trial_strain() + incre_strain) != SUANPAN_SUCCESS) {
			info = SUANPAN_FAIL;
			break;
		}
	}
	obj->commit_status();
	response(current_pos, span_a) = obj->get_current_strain().t();
	response(current_pos++, span_b) = obj->get_current_stress().t();

	if(SUANPAN_SUCCESS == info)
		for(const auto& I : idx) {
			for(unsigned J = 0; J < I; ++J) {
				total_load += incre_load;
				counter = 0;
				while(true) {
					const vec incre_strain = solve(obj->get_trial_stiffness(), total_load - obj->get_trial_stress());
					const auto error = norm(incre_strain);
					suanpan_info("local iteration error: %.5E.\n", error);
					if(error < 1E-12) break;
					if(++counter == 10 || obj->update_trial_status(obj->get_trial_strain() + incre_strain) != SUANPAN_SUCCESS) {
						info = SUANPAN_FAIL;
						break;
					}
				}
				if(SUANPAN_FAIL == info) break;
				obj->commit_status();
				response(current_pos, span_a) = obj->get_current_strain().t();
				response(current_pos++, span_b) = obj->get_current_stress().t();
			}
			if(SUANPAN_FAIL == info) break;
			incre_load = -incre_load;
		}

	obj->print();
	obj->reset_status();
	obj->clear_status();

	return response;
}
