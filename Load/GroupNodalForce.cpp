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

#include "GroupNodalForce.h"
#include <Domain/DomainBase.h>
#include <Domain/Group.h>

void GroupNodalForce::update_node_tag(const shared_ptr<DomainBase>& D) const {
	if(!nodes.empty()) return;

	vector<uword> tag;

	for(auto& I : groups) if(D->find<Group>(I)) for(auto& J : D->get<Group>(I)->get_pool()) tag.emplace_back(J);

	access::rw(nodes) = unique(uvec(tag));
}

GroupNodalForce::GroupNodalForce(const unsigned T, const unsigned S, const double L, uvec&& N, const unsigned D, const unsigned AT)
	: NodalForce(T, S, L, uvec{}, uvec{D}, AT)
	, groups(std::forward<uvec>(N)) {}

GroupNodalForce::GroupNodalForce(const unsigned T, const unsigned S, const double L, uvec&& N, uvec&& D, const unsigned AT)
	: NodalForce(T, S, L, uvec{}, std::forward<uvec>(D), AT)
	, groups(std::forward<uvec>(N)) {}

int GroupNodalForce::initialize(const shared_ptr<DomainBase>& D) {
	update_node_tag(D);

	return Load::initialize(D);
}
