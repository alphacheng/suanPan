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

#include "GlobalStiffnessRecorder.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Element/Element.h>

void GlobalStiffnessRecorder::assemble_stiffness(const mat& EK, const uvec& EC, mat& GK) {
	if(EK.is_empty()) return;

	for(unsigned I = 0; I < EC.n_elem; ++I) for(unsigned J = 0; J < EC.n_elem; ++J) GK(EC(J), EC(I)) += EK(J, I);
}

GlobalStiffnessRecorder::GlobalStiffnessRecorder(const unsigned T, const unsigned I, const bool R, const bool H)
	: GlobalRecorder(T, OutputType::K, I, R, H) {}

void GlobalStiffnessRecorder::record(const shared_ptr<DomainBase>& D) {
	if(1 != interval && counter++ != interval) return;

	counter = 1;

	auto& W = D->get_factory();
	auto& C = D->get_color_map();

	const uword S = W->get_size();

	vec stiffness(S * S, fill::zeros);

	mat g_stiffness(stiffness.memptr(), S, S, false, true);

	if(C.empty()) for(const auto& I : D->get_element_pool()) assemble_stiffness(I->get_current_stiffness(), I->get_dof_encoding(), g_stiffness);
	else
		std::for_each(C.begin(), C.end(), [&](const vector<unsigned>& color) {
			suanpan_for_each(color.begin(), color.end(), [&](const unsigned tag) {
				const auto& I = D->get<Element>(tag);
				assemble_stiffness(I->get_current_stiffness(), I->get_dof_encoding(), g_stiffness);
			});
		});

	insert({stiffness}, 0);

	if(if_record_time()) insert(D->get_factory()->get_current_time());
}

void GlobalStiffnessRecorder::print() { suanpan_info("A Global Recorder.\n"); }
