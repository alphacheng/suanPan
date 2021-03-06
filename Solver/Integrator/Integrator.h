/*******************************************************************************
 * Copyright (C) 2017-2021 Theodore Chang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
/**
 * @class Integrator
 * @brief The Integrator class is basically a warpper of the DomainBase class
 * with regard to some status changing methods.
 *
 * By default, the Step object calls DomainBase(Workshop) object to update
 * displacement/resistance/stiffness independently. When it comes to dynamic
 * analysis (time integration is involved), it is necessary to compute the
 * equivalent load/stiffness by combining several quantities.
 *
 * The Integrator object is acting like an agent between Workshop and Step, that
 * can modify corresponding quantities to account for dynamic effect.
 *
 * @author tlc
 * @date 27/08/2017
 * @version 0.1.2
 * @file Integrator.h
 * @addtogroup Integrator
 * @ingroup Analysis
 * @{
 */

#ifndef INTERGRATOR_H
#define INTERGRATOR_H

#include <Domain/Tag.h>

class DomainBase;

class Integrator : public Tag {
	bool time_step_switch = true;

	weak_ptr<DomainBase> database;
public:
	explicit Integrator(unsigned = 0);
	Integrator(const Integrator&) = delete;            // copy forbidden
	Integrator(Integrator&&) = delete;                 // move forbidden
	Integrator& operator=(const Integrator&) = delete; // assign forbidden
	Integrator& operator=(Integrator&&) = delete;      // assign forbidden
	virtual ~Integrator();

	void set_domain(const weak_ptr<DomainBase>&);
	[[nodiscard]] const weak_ptr<DomainBase>& get_domain() const;

	virtual int initialize();

	// ! some multi-step integrators may require fixed time step for some consecutive sub-steps
	void set_time_step_switch(bool);
	[[nodiscard]] bool allow_to_change_time_step() const;

	[[nodiscard]] virtual int process_load() const;
	[[nodiscard]] virtual int process_constraint() const;
	[[nodiscard]] virtual int process_criterion() const;
	[[nodiscard]] virtual int process_modifier() const;

	void record() const;

	virtual void assemble_resistance();
	virtual void assemble_matrix();

	virtual mat get_force_residual();
	virtual mat get_displacement_residual();
	virtual sp_mat get_reference_load();

	void update_trial_time(double);
	void update_incre_time(double);
	virtual int update_trial_status();
	virtual int update_incre_status();

	virtual int update_internal(const mat&);

	mat solve(const mat&);
	mat solve(const sp_mat&);
	mat solve_trs(const mat&);
	mat solve_trs(const sp_mat&);
	virtual int solve(mat&, const mat&);
	virtual int solve(mat&, const sp_mat&);
	virtual int solve_trs(mat&, const mat&);
	virtual int solve_trs(mat&, const sp_mat&);

	virtual void erase_machine_error() const;

	virtual void commit_status();
	virtual void clear_status();
	virtual void reset_status();

	virtual void update_parameter(double);
	virtual void update_compatibility() const;
};

#endif

//! @}
