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
 * @class MetaMat
 * @brief A MetaMat class that holds matrices.
 *
 * @author tlc
 * @date 08/09/2017
 * @version 0.2.0
 * @file MetaMat.hpp
 * @addtogroup MetaMat
 * @{
 */

#ifndef METAMAT_HPP
#define METAMAT_HPP

#include <Toolbox/debug.h>
#include <Domain/MetaMat/triplet_form.hpp>

enum class Precision { SINGLE, DOUBLE };

template<typename T> class MetaMat {
protected:
	static const char TRAN;

	virtual unique_ptr<MetaMat> factorize();

	virtual unique_ptr<MetaMat> i();
	virtual unique_ptr<MetaMat> inv();
public:
	triplet_form<T, uword> triplet_mat;

	Col<int> IPIV;
	bool factored = false;
	const uword n_rows;
	const uword n_cols;
	const uword n_elem;

	const T* const memory = nullptr;

	Precision precision = Precision::DOUBLE;

	double tolerance = 1E-14;

	MetaMat();
	MetaMat(uword, uword, uword);
	MetaMat(const MetaMat&);
	MetaMat(MetaMat&&) noexcept;
	MetaMat& operator=(const MetaMat&);
	MetaMat& operator=(MetaMat&&) noexcept;
	virtual ~MetaMat();

	[[nodiscard]] virtual bool is_empty() const;
	virtual void init();
	virtual void zeros();
	virtual void reset();

	virtual unique_ptr<MetaMat> make_copy() = 0;

	virtual void unify(uword);

	virtual T max() const;

	virtual const T& operator()(uword, uword) const;
	virtual T& at(uword, uword);

	virtual const T* memptr() const;
	virtual T* memptr();

	virtual void operator+=(const shared_ptr<MetaMat>&);
	virtual void operator-=(const shared_ptr<MetaMat>&);

	virtual Mat<T> operator*(const Mat<T>&) = 0;

	virtual void operator*=(T);

	Mat<T> solve(const Mat<T>&);
	Mat<T> solve(const SpMat<T>&);
	virtual int solve(Mat<T>&, const Mat<T>&) = 0;
	int solve(Mat<T>&, const SpMat<T>&);

	Mat<T> solve_trs(const Mat<T>&);
	Mat<T> solve_trs(const SpMat<T>&);
	virtual int solve_trs(Mat<T>&, const Mat<T>&) = 0;
	int solve_trs(Mat<T>&, const SpMat<T>&);

	[[nodiscard]] virtual int sign_det() const;

	virtual void print();
	virtual void save(const char*);

	virtual void csc_condense();
	virtual void csr_condense();
};

template<typename T> const char MetaMat<T>::TRAN = 'N';

template<typename T> Mat<T> to_mat(const MetaMat<T>& in_mat) {
	Mat<T> out_mat(in_mat.n_rows, in_mat.n_cols, fill::zeros);

	for(uword I = 0; I < in_mat.n_rows; ++I) for(uword J = 0; J < in_mat.n_cols; ++J) out_mat(I, J) = in_mat(I, J);

	return out_mat;
}

template<typename T> Mat<T> to_mat(const shared_ptr<MetaMat<T>>& in_mat) {
	Mat<T> out_mat(in_mat->n_rows, in_mat->n_cols, fill::zeros);

	for(uword I = 0; I < in_mat->n_rows; ++I) for(uword J = 0; J < in_mat->n_cols; ++J) out_mat(I, J) = in_mat->operator()(I, J);

	return out_mat;
}

template<typename T> MetaMat<T>::MetaMat()
	: n_rows(0)
	, n_cols(0)
	, n_elem(0) {}

template<typename T> MetaMat<T>::MetaMat(const uword in_rows, const uword in_cols, const uword in_elem)
	: triplet_mat(in_rows, in_cols)
	, n_rows(in_rows)
	, n_cols(in_cols)
	, n_elem(in_elem) {
	MetaMat<T>::init();
	MetaMat<T>::zeros();
}

template<typename T> MetaMat<T>::MetaMat(const MetaMat& old_mat)
	: triplet_mat(old_mat.triplet_mat)
	, factored(old_mat.factored)
	, n_rows(old_mat.n_rows)
	, n_cols(old_mat.n_cols)
	, n_elem(old_mat.n_elem)
	, precision(old_mat.precision)
	, tolerance(old_mat.tolerance) {
	MetaMat<T>::init();
	if(nullptr != old_mat.memptr()) std::copy(old_mat.memptr(), old_mat.memptr() + old_mat.n_elem, MetaMat<T>::memptr());
}

template<typename T> MetaMat<T>::MetaMat(MetaMat&& old_mat) noexcept
	: triplet_mat(std::forward<triplet_form<T, uword>>(old_mat.triplet_mat))
	, factored(old_mat.factored)
	, n_rows(old_mat.n_rows)
	, n_cols(old_mat.n_cols)
	, n_elem(old_mat.n_elem)
	, precision(old_mat.precision)
	, tolerance(old_mat.tolerance) {
	access::rw(memory) = old_mat.memory;
	access::rw(old_mat.memory) = nullptr;
}

template<typename T> MetaMat<T>& MetaMat<T>::operator=(const MetaMat& old_mat) {
	if(&old_mat != this) {
		factored = old_mat.factored;
		triplet_mat = old_mat.triplet_mat;
		precision = old_mat.precision;
		tolerance = old_mat.tolerance;
		access::rw(n_rows) = old_mat.n_rows;
		access::rw(n_cols) = old_mat.n_cols;
		access::rw(n_elem) = old_mat.n_elem;
		init();
		if(nullptr != old_mat.memptr()) std::copy(old_mat.memptr(), old_mat.memptr() + old_mat.n_elem, memptr());
	}
	return *this;
}

template<typename T> MetaMat<T>& MetaMat<T>::operator=(MetaMat&& old_mat) noexcept {
	if(&old_mat != this) {
		factored = old_mat.factored;
		triplet_mat = old_mat.triplet_mat;
		precision = old_mat.precision;
		tolerance = old_mat.tolerance;
		access::rw(n_rows) = old_mat.n_rows;
		access::rw(n_cols) = old_mat.n_cols;
		access::rw(n_elem) = old_mat.n_elem;
		access::rw(memory) = old_mat.memory;
		access::rw(old_mat.memory) = nullptr;
	}
	return *this;
}

template<typename T> MetaMat<T>::~MetaMat() { if(memory != nullptr) memory::release(access::rw(memory)); }

template<typename T> bool MetaMat<T>::is_empty() const { return 0 == n_elem; }

template<typename T> void MetaMat<T>::init() {
	if(memory != nullptr) memory::release(access::rw(memory));
	access::rw(memory) = is_empty() ? nullptr : memory::acquire<T>(n_elem);
}

template<typename T> void MetaMat<T>::zeros() {
	arrayops::fill_zeros(memptr(), n_elem);
	factored = false;
}

template<typename T> void MetaMat<T>::reset() {
	access::rw(n_rows) = 0;
	access::rw(n_cols) = 0;
	access::rw(n_elem) = 0;
	if(memory != nullptr) memory::release(access::rw(memory));
	access::rw(memory) = nullptr;
	factored = false;
}

template<typename T> void MetaMat<T>::unify(const uword idx) {
#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<uword>(0), n_rows, [&](const uword I) { at(I, idx) = 0.; });
	tbb::parallel_for(static_cast<uword>(0), n_cols, [&](const uword I) { at(idx, I) = 0.; });
#else
	for(uword I = 0; I < n_rows; ++I) at(I, idx) = 0.;
	for(uword I = 0; I < n_cols; ++I) at(idx, I) = 0.;
#endif
	at(idx, idx) = 1.;
}

template<typename T> T MetaMat<T>::max() const { return op_max::direct_max(memptr(), n_elem); }

template<typename T> const T& MetaMat<T>::operator()(const uword in_row, const uword in_col) const { return memory[in_row + in_col * n_rows]; }

template<typename T> T& MetaMat<T>::at(const uword in_row, const uword in_col) { return access::rw(memory[in_row + in_col * n_rows]); }

template<typename T> const T* MetaMat<T>::memptr() const { return memory; }

template<typename T> T* MetaMat<T>::memptr() { return const_cast<T*>(memory); }

template<typename T> void MetaMat<T>::operator+=(const shared_ptr<MetaMat>& M) {
	if(n_rows == M->n_rows && n_cols == M->n_cols && n_elem == M->n_elem) {
		arrayops::inplace_plus(memptr(), M->memptr(), n_elem);
		factored = false;
	}
}

template<typename T> void MetaMat<T>::operator-=(const shared_ptr<MetaMat>& M) {
	if(n_rows == M->n_rows && n_cols == M->n_cols && n_elem == M->n_elem) {
		arrayops::inplace_minus(memptr(), M->memptr(), n_elem);
		factored = false;
	}
}

template<typename T> void MetaMat<T>::operator*=(const T value) { arrayops::inplace_mul(memptr(), value, n_elem); }

template<typename T> Mat<T> MetaMat<T>::solve(const Mat<T>& B) {
	Mat<T> X;
	if(this->solve(X, B) != 0) X.reset();
	return X;
}

template<typename T> Mat<T> MetaMat<T>::solve(const SpMat<T>& B) {
	Mat<T> X;
	if(this->solve(X, Mat<T>(B)) != 0) X.reset();
	return X;
}

template<typename T> int MetaMat<T>::solve(Mat<T>& X, const SpMat<T>& B) { return this->solve(X, Mat<T>(B)); }

template<typename T> Mat<T> MetaMat<T>::solve_trs(const Mat<T>& B) {
	Mat<T> X;
	if(this->solve_trs(X, B) != 0) X.reset();
	return X;
}

template<typename T> Mat<T> MetaMat<T>::solve_trs(const SpMat<T>& B) {
	Mat<T> X;
	if(this->solve_trs(X, Mat<T>(B)) != 0) X.reset();
	return X;
}

template<typename T> unique_ptr<MetaMat<T>> MetaMat<T>::factorize() { throw invalid_argument("not supported"); }

template<typename T> unique_ptr<MetaMat<T>> MetaMat<T>::i() { throw invalid_argument("not supported"); }

template<typename T> unique_ptr<MetaMat<T>> MetaMat<T>::inv() { return i(); }

template<typename T> int MetaMat<T>::solve_trs(Mat<T>& X, const SpMat<T>& B) { return this->solve_trs(X, Mat<T>(B)); }

template<typename T> int MetaMat<T>::sign_det() const {
	auto det_sign = 1;
	for(unsigned I = 0; I < IPIV.n_elem; ++I) if((this->operator()(I, I) < 0.) ^ (static_cast<int>(I) + 1 != IPIV(I))) det_sign = -det_sign;
	return det_sign;
}

template<typename T> void MetaMat<T>::print() {
	for(uword I = 0; I < n_rows; ++I) {
		for(uword J = 0; J < n_cols; ++J) suanpan_info("%+.3E\t", operator()(I, J));
		suanpan_info("\n");
	}
	suanpan_info("\n");
}

template<typename T> void MetaMat<T>::save(const char* name) { if(!to_mat(*this).save(name, raw_ascii)) suanpan_error("cannot save matrix to file.\n"); }

template<typename T> void MetaMat<T>::csc_condense() {}

template<typename T> void MetaMat<T>::csr_condense() {}

#endif

//! @}
