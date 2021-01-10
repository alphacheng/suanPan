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

#include <Constraint/Constraint>
#include <Domain/DomainBase.h>
#include <Domain/ExternalModule.h>

int create_new_constraint(const shared_ptr<DomainBase>& domain, istringstream& command) {
	string constraint_id;
	if(!get_input(command, constraint_id)) {
		suanpan_error("create_new_constraint() needs constraint type.\n");
		return 0;
	}

	unique_ptr<Constraint> new_constraint = nullptr;

	if(is_equal(constraint_id, "Embed2D")) new_embed2d(new_constraint, command);
	else if(is_equal(constraint_id, "MPC")) new_mpc(new_constraint, command);
	else if(is_equal(constraint_id, "ParticleCollision2D")) new_particlecollision2d(new_constraint, command);
	else if(is_equal(constraint_id, "ParticleCollision3D")) new_particlecollision3d(new_constraint, command);
	else if(is_equal(constraint_id, "RigidWallMultiplier")) new_rigidwall(new_constraint, command, false, false);
	else if(is_equal(constraint_id, "RigidWallPenalty")) new_rigidwall(new_constraint, command, false, true);
	else if(is_equal(constraint_id, "FiniteRigidWallMultiplier")) new_rigidwall(new_constraint, command, true, false);
	else if(is_equal(constraint_id, "FiniteRigidWallPenalty")) new_rigidwall(new_constraint, command, true, true);
	else if(is_equal(constraint_id, "PenaltyBC")) new_bc(new_constraint, command, true, false);
	else if(is_equal(constraint_id, "GroupPenaltyBC")) new_bc(new_constraint, command, true, true);
	else if(is_equal(constraint_id, "MultiplierBC")) new_bc(new_constraint, command, false, false);
	else if(is_equal(constraint_id, "GroupMultiplierBC")) new_bc(new_constraint, command, false, true);
	else load::object(new_constraint, domain, constraint_id, command);

	if(new_constraint != nullptr) new_constraint->set_start_step(domain->get_current_step_tag());

	if(new_constraint == nullptr || !domain->insert(std::move(new_constraint))) suanpan_error("create_new_constraint() fails to create new constraint.\n");

	return 0;
}

void new_bc(unique_ptr<Constraint>& return_obj, istringstream& command, const bool penalty, const bool group) {
	unsigned bc_id;
	if(!get_input(command, bc_id)) {
		suanpan_error("new_bc() needs bc tag.\n");
		return;
	}

	string dof_id;
	if(!get_input(command, dof_id)) {
		suanpan_error("new_bc() needs valid DoFs.\n");
		return;
	}

	uword node;
	vector<uword> node_tag;
	while(get_input(command, node)) node_tag.push_back(node);

	const auto bc_type = suanpan::to_lower(dof_id[0]);

	penalty ? group ? return_obj = make_unique<GroupPenaltyBC>(bc_id, 0, uvec(node_tag), bc_type) : return_obj = make_unique<PenaltyBC>(bc_id, 0, uvec(node_tag), bc_type) : group ? return_obj = make_unique<GroupMultiplierBC>(bc_id, 0, uvec(node_tag), bc_type) : return_obj = make_unique<MultiplierBC>(bc_id, 0, uvec(node_tag), bc_type);
}

void new_embed2d(unique_ptr<Constraint>& return_obj, istringstream& command) {
	unsigned tag;
	if(!get_input(command, tag)) {
		suanpan_error("new_embed2d() needs a valid tag.\n");
		return;
	}

	unsigned element_tag;
	if(!get_input(command, element_tag)) {
		suanpan_error("new_embed2d() needs a valid element tag.\n");
		return;
	}

	unsigned node_tag;
	if(!get_input(command, node_tag)) {
		suanpan_error("new_embed2d() needs a valid node tag.\n");
		return;
	}

	return_obj = make_unique<Embed2D>(tag, 0, element_tag, node_tag);
}

void new_mpc(unique_ptr<Constraint>& return_obj, istringstream& command) {
	unsigned tag;
	if(!get_input(command, tag)) {
		suanpan_error("new_mpc() needs a valid tag.\n");
		return;
	}

	unsigned amplitude;
	if(!get_input(command, amplitude)) {
		suanpan_error("new_mpc() needs a valid amplitude tag.\n");
		return;
	}

	double magnitude;
	if(!get_input(command, magnitude)) {
		suanpan_error("new_mpc() needs a valid magnitude.\n");
		return;
	}

	uword node, dof;
	double weight;
	vector<uword> node_tag, dof_tag;
	vector<double> weight_tag;
	while(!command.eof()) {
		if(!get_input(command, node) || !get_input(command, dof) || !get_input(command, weight)) return;
		node_tag.emplace_back(node);
		dof_tag.emplace_back(dof);
		weight_tag.emplace_back(weight);
	}

	return_obj = make_unique<MPC>(tag, 0, amplitude, uvec(node_tag), uvec(dof_tag), vec(weight_tag), magnitude);
}

void new_particlecollision2d(unique_ptr<Constraint>& return_obj, istringstream& command) {
	unsigned tag;
	if(!get_input(command, tag)) {
		suanpan_error("new_particlecollision2d() needs a valid tag.\n");
		return;
	}

	auto space = 1.;
	if(!command.eof() && !get_input(command, space)) {
		suanpan_error("new_particlecollision2d() needs a valid spacing.\n");
		return;
	}

	auto alpha = 1.;
	if(!command.eof() && !get_input(command, alpha)) {
		suanpan_error("new_particlecollision2d() needs a valid multiplier.\n");
		return;
	}

	return_obj = make_unique<ParticleCollision2D>(tag, 0, space, alpha);
}

void new_particlecollision3d(unique_ptr<Constraint>& return_obj, istringstream& command) {
	unsigned tag;
	if(!get_input(command, tag)) {
		suanpan_error("new_particlecollision3d() needs a valid tag.\n");
		return;
	}

	auto space = 1.;
	if(!command.eof() && !get_input(command, space)) {
		suanpan_error("new_particlecollision3d() needs a valid spacing.\n");
		return;
	}

	auto alpha = 1.;
	if(!command.eof() && !get_input(command, alpha)) {
		suanpan_error("new_particlecollision3d() needs a valid multiplier.\n");
		return;
	}

	return_obj = make_unique<ParticleCollision3D>(tag, 0, space, alpha);
}

void new_rigidwall(unique_ptr<Constraint>& return_obj, istringstream& command, const bool finite, const bool penalty) {
	unsigned tag;
	if(!get_input(command, tag)) {
		suanpan_error("new_rigidwall() needs a valid tag.\n");
		return;
	}

	vec origin(3), norm(3), edge(3);
	for(auto I = 0; I < 3; ++I) if(!get_input(command, origin(I))) return;
	for(auto I = 0; I < 3; ++I) if(!get_input(command, norm(I))) return;

	if(finite) for(auto I = 0; I < 3; ++I) if(!get_input(command, edge(I))) return;

	auto alpha = 1.;
	if(!command.eof() && !get_input(command, alpha)) {
		suanpan_error("new_rigidwall() needs a valid multiplier.\n");
		return;
	}

	return_obj = finite ? penalty ? make_unique<RigidWallPenalty>(tag, 0, 0, std::move(origin), std::move(norm), std::move(edge), alpha) : make_unique<RigidWallMultiplier>(tag, 0, 0, std::move(origin), std::move(norm), std::move(edge), alpha) : penalty ? make_unique<RigidWallPenalty>(tag, 0, 0, std::move(origin), std::move(norm), alpha) : make_unique<RigidWallMultiplier>(tag, 0, 0, std::move(origin), std::move(norm), alpha);
}
