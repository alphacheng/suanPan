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

#include "Node.h"
#include <Domain/DomainBase.h>
#include <Recorder/OutputType.h>

Node::Node(const unsigned T)
	: Tag(T) { suanpan_debug("Node %u ctor() called.\n", T); }

Node::Node(const unsigned T, vec&& C)
	: Tag(T) {
	num_dof = static_cast<unsigned>(C.n_elem);
	coordinate = std::forward<vec>(C);
	suanpan_debug("Node %u ctor() called.\n", T);
}

/**
 * \brief initialize `num_dof` and set the size of `coordinate` to `num_dof`.
 * \param T `unique_tag`
 * \param D `num_dof`
 */
Node::Node(const unsigned T, const unsigned D)
	: Tag(T) {
	num_dof = D;
	coordinate.zeros(D);
	suanpan_debug("Node %u ctor() called.\n", T);
}

/**
 * \brief initialize `num_dof` and `coordinate`.
 * \param T `unique_tag`
 * \param D `num_dof`
 * \param C `coordinate`
 */
Node::Node(const unsigned T, const unsigned D, vec&& C)
	: Tag(T) {
	num_dof = D;
	coordinate = std::forward<vec>(C);
	suanpan_debug("Node %u ctor() called.\n", T);
}

/**
 * \brief default destructor.
 */
Node::~Node() { suanpan_debug("Node %u dtor() called.\n", get_tag()); }

/**
 * \brief This method should be called after Element objects are set. Element
 * objects will set the minimum number of DoFs for all related Node objects.
 * This method initialize all member variables with the size of `num_dof` and
 * fill `original_dof` with `-1` to indicated it should be omitted from the
 * system. Finally check if the size of `coordinate` is the same of `num_dof`,
 * if not, resize it to `num_dof`. This will be necessary for beam/plate/shell
 * problems which have more DoFs than coordinates.
 */
void Node::initialize(const shared_ptr<DomainBase>& D) {
	if(initialized || !is_active()) return;

	if(num_dof != 0) {
		original_dof.zeros(num_dof);
		original_dof.fill(static_cast<uword>(-1));

		reordered_dof.reset();

		current_displacement.resize(num_dof);
		current_velocity.resize(num_dof);
		current_acceleration.resize(num_dof);

		incre_displacement.resize(num_dof);
		incre_velocity.resize(num_dof);
		incre_acceleration.resize(num_dof);

		trial_displacement.resize(num_dof);
		trial_velocity.resize(num_dof);
		trial_acceleration.resize(num_dof);
	} else {
		suanpan_debug("Node %u is not used in the problem, now disable it.\n", get_tag());
		D->disable_node(get_tag());
	}

	initialized = true;
}

void Node::set_initialized(const bool B) { initialized = B; }

bool Node::get_initialized() const { return initialized; }

void Node::set_dof_number(const unsigned D) {
	if(num_dof == D) return;

	num_dof = D;
	initialized = false;
}

unsigned Node::get_dof_number() const { return num_dof; }

void Node::set_original_dof(unsigned& F) {
	if(!is_active()) return;

	for(unsigned I = 0; I < num_dof; ++I, ++F)
		if(original_dof(I) != F) {
			original_dof(I) = F;
			initialized = false;
		}
}

void Node::set_original_dof(const uvec& D) {
	if(!is_active() || original_dof.size() == D.size() && sum(abs(D - original_dof)) == 0) return;

	original_dof = D;
	initialized = false;
}

const uvec& Node::get_original_dof() const { return original_dof; }

void Node::set_reordered_dof(const uvec& R) { reordered_dof = R; }

const uvec& Node::get_reordered_dof() const { return reordered_dof.is_empty() ? original_dof : reordered_dof; }

void Node::set_coordinate(const vec& C) { coordinate = C; }

const vec& Node::get_coordinate() const { return coordinate; }

void Node::set_current_resistance(const vec& R) { current_resistance = R; }

void Node::set_current_damping_force(const vec& R) { current_damping_force = R; }

void Node::set_current_inertial_force(const vec& R) { current_inertial_force = R; }

void Node::set_current_displacement(const vec& D) { current_displacement = D; }

void Node::set_current_velocity(const vec& V) { current_velocity = V; }

void Node::set_current_acceleration(const vec& A) { current_acceleration = A; }

void Node::set_incre_resistance(const vec& R) { incre_resistance = R; }

void Node::set_incre_damping_force(const vec& R) { incre_damping_force = R; }

void Node::set_incre_inertial_force(const vec& R) { incre_inertial_force = R; }

void Node::set_incre_displacement(const vec& D) { incre_displacement = D; }

void Node::set_incre_velocity(const vec& V) { incre_velocity = V; }

void Node::set_incre_acceleration(const vec& A) { incre_acceleration = A; }

void Node::set_trial_resistance(const vec& R) { trial_resistance = R; }

void Node::set_trial_damping_force(const vec& R) { trial_damping_force = R; }

void Node::set_trial_inertial_force(const vec& R) { trial_inertial_force = R; }

void Node::set_trial_displacement(const vec& D) { trial_displacement = D; }

void Node::set_trial_velocity(const vec& V) { trial_velocity = V; }

void Node::set_trial_acceleration(const vec& A) { trial_acceleration = A; }

const vec& Node::get_current_resistance() const { return current_resistance; }

const vec& Node::get_current_damping_force() const { return current_damping_force; }

const vec& Node::get_current_inertial_force() const { return current_inertial_force; }

const vec& Node::get_current_displacement() const { return current_displacement; }

const vec& Node::get_current_velocity() const { return current_velocity; }

const vec& Node::get_current_acceleration() const { return current_acceleration; }

const vec& Node::get_incre_resistance() const { return incre_resistance; }

const vec& Node::get_incre_damping_force() const { return incre_damping_force; }

const vec& Node::get_incre_inertial_force() const { return incre_inertial_force; }

const vec& Node::get_incre_displacement() const { return incre_displacement; }

const vec& Node::get_incre_velocity() const { return incre_velocity; }

const vec& Node::get_incre_acceleration() const { return incre_acceleration; }

const vec& Node::get_trial_resistance() const { return trial_resistance; }

const vec& Node::get_trial_damping_force() const { return trial_damping_force; }

const vec& Node::get_trial_inertial_force() const { return trial_inertial_force; }

const vec& Node::get_trial_displacement() const { return trial_displacement; }

const vec& Node::get_trial_velocity() const { return trial_velocity; }

const vec& Node::get_trial_acceleration() const { return trial_acceleration; }

void Node::update_current_resistance(const vec& R) {
	trial_resistance = current_resistance = R;
	incre_resistance.zeros(R.size());
}

void Node::update_current_damping_force(const vec& R) {
	trial_damping_force = current_damping_force = R;
	incre_damping_force.zeros(R.size());
}

void Node::update_current_inertial_force(const vec& R) {
	trial_inertial_force = current_inertial_force = R;
	incre_inertial_force.zeros(R.size());
}

void Node::update_current_displacement(const vec& D) {
	current_displacement = trial_displacement = D;
	incre_displacement.zeros(D.size());
}

void Node::update_current_velocity(const vec& V) {
	current_velocity = trial_velocity = V;
	incre_velocity.zeros(V.size());
}

void Node::update_current_acceleration(const vec& A) {
	current_acceleration = trial_acceleration = A;
	incre_acceleration.zeros(A.size());
}

void Node::update_incre_resistance(const vec& R) {
	incre_resistance = R;
	if(current_resistance.empty()) current_resistance.zeros(R.size());
	else current_resistance.resize(R.size());
	trial_resistance = current_resistance + incre_resistance;
}

void Node::update_incre_damping_force(const vec& R) {
	incre_damping_force = R;
	if(current_damping_force.empty()) current_damping_force.zeros(R.size());
	else current_damping_force.resize(R.size());
	trial_damping_force = current_damping_force + incre_damping_force;
}

void Node::update_incre_inertial_force(const vec& R) {
	incre_inertial_force = R;
	if(current_inertial_force.empty()) current_inertial_force.zeros(R.size());
	else current_inertial_force.resize(R.size());
	trial_inertial_force = current_inertial_force + incre_inertial_force;
}

void Node::update_incre_displacement(const vec& D) {
	incre_displacement = D;
	if(current_displacement.empty()) current_displacement.zeros(D.size());
	else current_displacement.resize(D.size());
	trial_displacement = current_displacement + incre_displacement;
}

void Node::update_incre_velocity(const vec& V) {
	incre_velocity = V;
	if(current_velocity.empty()) current_velocity.zeros(V.size());
	else current_velocity.resize(V.size());
	trial_velocity = current_velocity + incre_velocity;
}

void Node::update_incre_acceleration(const vec& A) {
	incre_acceleration = A;
	if(current_acceleration.empty()) current_acceleration.zeros(A.size());
	else current_acceleration.resize(A.size());
	trial_acceleration = current_acceleration + incre_acceleration;
}

void Node::update_trial_resistance(const vec& R) {
	trial_resistance = R;
	if(current_resistance.empty()) current_resistance.zeros(R.size());
	else current_resistance.resize(R.size());
	incre_resistance = trial_resistance - current_resistance;
}

void Node::update_trial_damping_force(const vec& R) {
	trial_damping_force = R;
	if(current_damping_force.empty()) current_damping_force.zeros(R.size());
	else current_damping_force.resize(R.size());
	incre_damping_force = trial_damping_force - current_damping_force;
}

void Node::update_trial_inertial_force(const vec& R) {
	trial_inertial_force = R;
	if(current_inertial_force.empty()) current_inertial_force.zeros(R.size());
	else current_inertial_force.resize(R.size());
	incre_inertial_force = trial_inertial_force - current_inertial_force;
}

void Node::update_trial_displacement(const vec& D) {
	trial_displacement = D;
	if(current_displacement.empty()) current_displacement.zeros(D.size());
	else current_displacement.resize(D.size());
	incre_displacement = trial_displacement - current_displacement;
}

void Node::update_trial_velocity(const vec& V) {
	trial_velocity = V;
	if(current_velocity.empty()) current_velocity.zeros(V.size());
	else current_velocity.resize(V.size());
	incre_velocity = trial_velocity - current_velocity;
}

void Node::update_trial_acceleration(const vec& A) {
	trial_acceleration = A;
	if(current_acceleration.empty()) current_acceleration.zeros(A.size());
	else current_acceleration.resize(A.size());
	incre_acceleration = trial_acceleration - current_acceleration;
}

void Node::update_current_status(const vec& D) {
	trial_displacement = current_displacement = D(reordered_dof);
	incre_displacement.zeros(reordered_dof.size());
}

void Node::update_current_status(const vec& D, const vec& V) {
	trial_velocity = current_velocity = V(reordered_dof);
	incre_velocity.zeros(reordered_dof.size());
	update_current_status(D);
}

void Node::update_current_status(const vec& D, const vec& V, const vec& A) {
	trial_acceleration = current_acceleration = A(reordered_dof);
	incre_acceleration.zeros(reordered_dof.size());
	update_current_status(D, V);
}

void Node::update_incre_status(const vec& D) {
	incre_displacement = D(reordered_dof);
	if(current_displacement.empty()) current_displacement.zeros(reordered_dof.size());
	else current_displacement.resize(reordered_dof.size());
	trial_displacement = current_displacement + incre_displacement;
}

void Node::update_incre_status(const vec& D, const vec& V) {
	incre_velocity = V(reordered_dof);
	if(current_velocity.empty()) current_velocity.zeros(reordered_dof.size());
	else current_velocity.resize(reordered_dof.size());
	trial_velocity = current_velocity + incre_velocity;
	update_incre_status(D);
}

void Node::update_incre_status(const vec& D, const vec& V, const vec& A) {
	incre_acceleration = A(reordered_dof);
	if(current_acceleration.empty()) current_acceleration.zeros(reordered_dof.size());
	else current_acceleration.resize(reordered_dof.size());
	trial_acceleration = current_acceleration + incre_acceleration;
	update_incre_status(D, V);
}

void Node::update_trial_status(const vec& D) {
	trial_displacement = D(reordered_dof);
	if(current_displacement.empty()) current_displacement.zeros(reordered_dof.size());
	else current_displacement.resize(reordered_dof.size());
	incre_displacement = trial_displacement - current_displacement;
}

void Node::update_trial_status(const vec& D, const vec& V) {
	trial_velocity = V(reordered_dof);
	if(current_velocity.empty()) current_velocity.zeros(reordered_dof.size());
	else current_velocity.resize(reordered_dof.size());
	incre_velocity = trial_velocity - current_velocity;
	update_trial_status(D);
}

void Node::update_trial_status(const vec& D, const vec& V, const vec& A) {
	trial_acceleration = A(reordered_dof);
	if(current_acceleration.empty()) current_acceleration.zeros(reordered_dof.size());
	else current_acceleration.resize(reordered_dof.size());
	incre_acceleration = trial_acceleration - current_acceleration;
	update_trial_status(D, V);
}

void Node::commit_status() {
	if(!trial_resistance.is_empty()) {
		current_resistance = trial_resistance;
		incre_resistance.zeros();
	}
	if(!trial_damping_force.is_empty()) {
		current_damping_force = trial_damping_force;
		incre_damping_force.zeros();
	}
	if(!trial_inertial_force.is_empty()) {
		current_inertial_force = trial_inertial_force;
		incre_inertial_force.zeros();
	}
	if(!trial_displacement.is_empty()) {
		current_displacement = trial_displacement;
		incre_displacement.zeros();
	}
	if(!trial_velocity.is_empty()) {
		current_velocity = trial_velocity;
		incre_displacement.zeros();
	}
	if(!trial_acceleration.is_empty()) {
		current_acceleration = trial_acceleration;
		incre_acceleration.zeros();
	}
}

void Node::reset_status() {
	if(!current_resistance.is_empty()) {
		trial_resistance = current_resistance;
		incre_resistance.zeros();
	}
	if(!current_damping_force.is_empty()) {
		trial_damping_force = current_damping_force;
		incre_damping_force.zeros();
	}
	if(!current_inertial_force.is_empty()) {
		trial_inertial_force = current_inertial_force;
		incre_inertial_force.zeros();
	}
	if(!current_displacement.is_empty()) {
		trial_displacement = current_displacement;
		incre_displacement.zeros();
	}
	if(!current_velocity.is_empty()) {
		trial_velocity = current_velocity;
		incre_velocity.zeros();
	}
	if(!current_acceleration.is_empty()) {
		trial_acceleration = current_acceleration;
		incre_acceleration.zeros();
	}
}

void Node::clear_status() {
	if(!current_resistance.is_empty()) {
		current_resistance.zeros();
		incre_resistance.zeros();
		trial_resistance.zeros();
	}
	if(!current_damping_force.is_empty()) {
		current_damping_force.zeros();
		incre_damping_force.zeros();
		trial_damping_force.zeros();
	}
	if(!current_inertial_force.is_empty()) {
		current_inertial_force.zeros();
		incre_inertial_force.zeros();
		trial_inertial_force.zeros();
	}
	if(!current_displacement.is_empty()) {
		current_displacement.zeros();
		incre_displacement.zeros();
		trial_displacement.zeros();
	}
	if(!current_velocity.is_empty()) {
		current_velocity.zeros();
		incre_velocity.zeros();
		trial_velocity.zeros();
	}
	if(!current_acceleration.is_empty()) {
		current_acceleration.zeros();
		incre_acceleration.zeros();
		trial_acceleration.zeros();
	}

	set_initialized(false);
}

vector<vec> Node::record(const OutputType L) const {
	vector<vec> data;

	if(L == OutputType::RF) data.push_back(current_resistance);
	else if(L == OutputType::DF) data.push_back(current_damping_force);
	else if(L == OutputType::IF) data.push_back(current_inertial_force);
	else if(L == OutputType::U) data.push_back(current_displacement);
	else if(L == OutputType::V) data.push_back(current_velocity);
	else if(L == OutputType::A) data.push_back(current_acceleration);
	else if(L == OutputType::U1 && current_displacement.n_elem >= 1) data.emplace_back(vec{current_displacement(0)});
	else if(L == OutputType::U2 && current_displacement.n_elem >= 2) data.emplace_back(vec{current_displacement(1)});
	else if(L == OutputType::U3 && current_displacement.n_elem >= 3) data.emplace_back(vec{current_displacement(2)});
	else if(L == OutputType::UR1 && current_displacement.n_elem >= 4) data.emplace_back(vec{current_displacement(3)});
	else if(L == OutputType::UR2 && current_displacement.n_elem >= 5) data.emplace_back(vec{current_displacement(4)});
	else if(L == OutputType::UR3 && current_displacement.n_elem >= 6) data.emplace_back(vec{current_displacement(5)});
	else if(L == OutputType::V1 && current_velocity.n_elem >= 1) data.emplace_back(vec{current_velocity(0)});
	else if(L == OutputType::V2 && current_velocity.n_elem >= 2) data.emplace_back(vec{current_velocity(1)});
	else if(L == OutputType::V3 && current_velocity.n_elem >= 3) data.emplace_back(vec{current_velocity(2)});
	else if(L == OutputType::VR1 && current_velocity.n_elem >= 4) data.emplace_back(vec{current_velocity(3)});
	else if(L == OutputType::VR2 && current_velocity.n_elem >= 5) data.emplace_back(vec{current_velocity(4)});
	else if(L == OutputType::VR3 && current_velocity.n_elem >= 6) data.emplace_back(vec{current_velocity(5)});
	else if(L == OutputType::A1 && current_acceleration.n_elem >= 1) data.emplace_back(vec{current_acceleration(0)});
	else if(L == OutputType::A2 && current_acceleration.n_elem >= 2) data.emplace_back(vec{current_acceleration(1)});
	else if(L == OutputType::A3 && current_acceleration.n_elem >= 3) data.emplace_back(vec{current_acceleration(2)});
	else if(L == OutputType::AR1 && current_acceleration.n_elem >= 4) data.emplace_back(vec{current_acceleration(3)});
	else if(L == OutputType::AR2 && current_acceleration.n_elem >= 5) data.emplace_back(vec{current_acceleration(4)});
	else if(L == OutputType::AR3 && current_acceleration.n_elem >= 6) data.emplace_back(vec{current_acceleration(5)});
	else if(L == OutputType::RF1 && current_resistance.n_elem >= 1) data.emplace_back(vec{current_resistance(0)});
	else if(L == OutputType::RF2 && current_resistance.n_elem >= 2) data.emplace_back(vec{current_resistance(1)});
	else if(L == OutputType::RF3 && current_resistance.n_elem >= 3) data.emplace_back(vec{current_resistance(2)});
	else if(L == OutputType::RM1 && current_resistance.n_elem >= 4) data.emplace_back(vec{current_resistance(3)});
	else if(L == OutputType::RM2 && current_resistance.n_elem >= 5) data.emplace_back(vec{current_resistance(4)});
	else if(L == OutputType::RM3 && current_resistance.n_elem >= 6) data.emplace_back(vec{current_resistance(5)});
	else if(L == OutputType::DF1 && current_damping_force.n_elem >= 1) data.emplace_back(vec{current_damping_force(0)});
	else if(L == OutputType::DF2 && current_damping_force.n_elem >= 2) data.emplace_back(vec{current_damping_force(1)});
	else if(L == OutputType::DF3 && current_damping_force.n_elem >= 3) data.emplace_back(vec{current_damping_force(2)});
	else if(L == OutputType::DM1 && current_damping_force.n_elem >= 4) data.emplace_back(vec{current_damping_force(3)});
	else if(L == OutputType::DM2 && current_damping_force.n_elem >= 5) data.emplace_back(vec{current_damping_force(4)});
	else if(L == OutputType::DM3 && current_damping_force.n_elem >= 6) data.emplace_back(vec{current_damping_force(5)});
	else if(L == OutputType::IF1 && current_inertial_force.n_elem >= 1) data.emplace_back(vec{current_inertial_force(0)});
	else if(L == OutputType::IF2 && current_inertial_force.n_elem >= 2) data.emplace_back(vec{current_inertial_force(1)});
	else if(L == OutputType::IF3 && current_inertial_force.n_elem >= 3) data.emplace_back(vec{current_inertial_force(2)});
	else if(L == OutputType::IM1 && current_inertial_force.n_elem >= 4) data.emplace_back(vec{current_inertial_force(3)});
	else if(L == OutputType::IM2 && current_inertial_force.n_elem >= 5) data.emplace_back(vec{current_inertial_force(4)});
	else if(L == OutputType::IM3 && current_inertial_force.n_elem >= 6) data.emplace_back(vec{current_inertial_force(5)});

	return data;
}

vec Node::save() const {
	auto size = coordinate.size();
	size += 2 * original_dof.size();
	size += current_resistance.size();
	size += current_displacement.size();
	size += current_velocity.size();
	size += current_acceleration.size();

	vec data(size + 10);

	data(0) = is_active();
	data(1) = get_tag();
	data(2) = initialized;
	data(3) = num_dof;
	data(4) = static_cast<double>(coordinate.size());
	data(5) = static_cast<double>(original_dof.size());
	data(6) = static_cast<double>(current_resistance.size());
	data(7) = static_cast<double>(current_displacement.size());
	data(8) = static_cast<double>(current_velocity.size());
	data(9) = static_cast<double>(current_acceleration.size());

	uword s_pos = 10, e_pos = s_pos + coordinate.size() - 1;
	if(!coordinate.empty()) data(span(s_pos, e_pos)) = coordinate;

	s_pos = e_pos + 1;
	e_pos = s_pos + original_dof.size() - 1;
	if(!original_dof.empty()) data(span(s_pos, e_pos)) = conv_to<vec>::from(original_dof);
	s_pos = e_pos + 1;
	e_pos = s_pos + reordered_dof.size() - 1;
	if(!reordered_dof.empty()) data(span(s_pos, e_pos)) = conv_to<vec>::from(reordered_dof);

	s_pos = e_pos + 1;
	e_pos = s_pos + current_resistance.size() - 1;
	if(!current_resistance.empty()) data(span(s_pos, e_pos)) = current_resistance;
	s_pos = e_pos + 1;
	e_pos = s_pos + current_displacement.size() - 1;
	if(!current_displacement.empty()) data(span(s_pos, e_pos)) = current_displacement;
	s_pos = e_pos + 1;
	e_pos = s_pos + current_velocity.size() - 1;
	if(!current_velocity.empty()) data(span(s_pos, e_pos)) = current_velocity;
	s_pos = e_pos + 1;
	e_pos = s_pos + current_acceleration.size() - 1;
	if(!current_acceleration.empty()) data(span(s_pos, e_pos)) = current_acceleration;

	return data;
}

void Node::load(const vec& data) {
	static_cast<bool>(data(0)) ? enable() : disable();
	set_tag(static_cast<unsigned>(data(1)));

	initialized = static_cast<bool>(data(2));
	num_dof = static_cast<unsigned>(data(3));

	uword s_pos = 10, e_pos = s_pos + static_cast<uword>(data(4)) - 1;
	if(0. != data(4)) coordinate = data(span(s_pos, e_pos));

	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(5)) - 1;
	if(0. != data(5)) original_dof = conv_to<uvec>::from(data(span(s_pos, e_pos)));
	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(5)) - 1;
	if(0. != data(5)) reordered_dof = conv_to<uvec>::from(data(span(s_pos, e_pos)));

	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(6)) - 1;
	if(0. != data(6)) current_resistance = data(span(s_pos, e_pos));
	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(7)) - 1;
	if(0. != data(7)) current_displacement = data(span(s_pos, e_pos));
	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(8)) - 1;
	if(0. != data(8)) current_velocity = data(span(s_pos, e_pos));
	s_pos = e_pos + 1;
	e_pos = s_pos + static_cast<uword>(data(9)) - 1;
	if(0. != data(9)) current_acceleration = data(span(s_pos, e_pos));
}

void Node::print() {
	suanpan_info("Node %u:\n", get_tag(), is_active() ? "" : " is currently inactive");
	coordinate.t().print();
	suanpan_info("Displacement:\n");
	current_displacement.t().print();
	suanpan_info("Resistance:\n");
	current_resistance.t().print();
	if(accu(current_velocity) != 0.) {
		suanpan_info("Velocity:\n");
		current_velocity.t().print();
	}
	if(accu(current_acceleration) != 0.) {
		suanpan_info("Acceleration:\n");
		current_acceleration.t().print();
	}
}
