﻿/*******************************************************************************
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

#ifndef CSR_FORM_HPP
#define CSR_FORM_HPP

#include "sparse_form.hpp"

template<typename data_t, typename index_t> class triplet_form;
template<typename data_t, typename index_t> class csc_form;

template<typename data_t, typename index_t> class csr_form final : public sparse_form<data_t, index_t, csr_form<data_t, index_t>> {
	using sparse_form<data_t, index_t, csr_form<data_t, index_t>>::bin;

	void copy_memory(index_t, const index_t*, const index_t*, const data_t*) override;
public:
	using sparse_form<data_t, index_t, csr_form<data_t, index_t>>::n_rows;
	using sparse_form<data_t, index_t, csr_form<data_t, index_t>>::n_cols;
	using sparse_form<data_t, index_t, csr_form<data_t, index_t>>::n_elem;
	using sparse_form<data_t, index_t, csr_form<data_t, index_t>>::c_size;

	index_t* row_ptr = nullptr; // index storage
	index_t* col_idx = nullptr; // index storage
	data_t* val_idx = nullptr;  // value storage

	csr_form() = default;

	~csr_form() override;

	csr_form(const csr_form&);                // copy ctor
	csr_form(csr_form&&) noexcept;            // move ctor
	csr_form& operator=(const csr_form&);     // copy assignment
	csr_form& operator=(csr_form&&) noexcept; // move assignment

	[[nodiscard]] const index_t* row_mem() const override;
	[[nodiscard]] const index_t* col_mem() const override;
	[[nodiscard]] const data_t* val_mem() const override;

	void reset() const override;
	void zeros() const override;

	[[nodiscard]] data_t max() const override;

	bool init() override;
	bool init(index_t) override;
	bool init(index_t, index_t, index_t) override;
	bool resize() override;
	bool resize(index_t) override;
	bool resize(index_t, index_t, index_t) override;

	void print() const override;

	template<typename T2> csr_form<data_t, index_t> operator*(T2);
	template<typename T2> csr_form<data_t, index_t> operator/(T2);
	template<typename T2> csr_form<data_t, index_t>& operator*=(T2);
	template<typename T2> csr_form<data_t, index_t>& operator/=(T2);

	template<typename in_dt, typename in_it> explicit csr_form(triplet_form<in_dt, in_it>&);
	template<typename in_dt, typename in_it> explicit csr_form(triplet_form<in_dt, in_it>&&);
	template<typename in_dt, typename in_it> csr_form& operator=(const triplet_form<in_dt, in_it>&);

	explicit csr_form(const csc_form<data_t, index_t>&);
	csr_form& operator=(const csc_form<data_t, index_t>&);

	const data_t& operator()(index_t, index_t) const;

	Mat<data_t> operator*(const Col<data_t>&) override;
	Mat<data_t> operator*(const Mat<data_t>&) override;
};

template<typename data_t, typename index_t> void csr_form<data_t, index_t>::copy_memory(const index_t in_size, const index_t* const in_row_ptr, const index_t* const in_col_idx, const data_t* const in_val_idx) {
	if(in_size > n_elem) resize(in_size);

	std::copy(in_row_ptr, in_row_ptr + n_rows + 1, this->row_ptr);
	std::copy(in_col_idx, in_col_idx + in_size, this->col_idx);
	std::copy(in_val_idx, in_val_idx + in_size, this->val_idx);

	access::rw(c_size) = in_size;
}

template<typename data_t, typename index_t> csr_form<data_t, index_t>::~csr_form() { csr_form<data_t, index_t>::reset(); }

template<typename data_t, typename index_t> csr_form<data_t, index_t>::csr_form(const csr_form& in_mat)
	: sparse_form<data_t, index_t, csr_form<data_t, index_t>>(in_mat.n_rows, in_mat.n_cols, in_mat.n_elem) {
	csr_form<data_t, index_t>::init();
	csr_form<data_t, index_t>::copy_memory(in_mat.c_size, in_mat.row_ptr, in_mat.col_idx, in_mat.val_idx);
}

template<typename data_t, typename index_t> csr_form<data_t, index_t>::csr_form(csr_form&& in_mat) noexcept {
	csr_form<data_t, index_t>::reset();
	access::rw(n_rows) = in_mat.n_rows;
	access::rw(n_cols) = in_mat.n_cols;
	access::rw(n_elem) = in_mat.n_elem;
	access::rw(c_size) = in_mat.c_size;
	row_ptr = in_mat.row_ptr;
	col_idx = in_mat.col_idx;
	val_idx = in_mat.val_idx;
	access::rw(in_mat.n_rows) = access::rw(in_mat.n_cols) = access::rw(in_mat.n_elem) = access::rw(in_mat.c_size) = 0;
	in_mat.row_ptr = in_mat.col_idx = nullptr;
	in_mat.val_idx = nullptr;
}

template<typename data_t, typename index_t> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator=(const csr_form& in_mat) {
	if(this != &in_mat) {
		init(in_mat.n_rows, in_mat.n_cols, in_mat.n_elem);
		copy_memory(in_mat.c_size, in_mat.col_idx, in_mat.row_ptr, in_mat.val_idx);
	}

	return *this;
}

template<typename data_t, typename index_t> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator=(csr_form&& in_mat) noexcept {
	reset();
	access::rw(n_rows) = in_mat.n_rows;
	access::rw(n_cols) = in_mat.n_cols;
	access::rw(n_elem) = in_mat.n_elem;
	access::rw(c_size) = in_mat.c_size;
	col_idx = in_mat.col_idx;
	row_ptr = in_mat.row_ptr;
	val_idx = in_mat.val_idx;
	access::rw(in_mat.n_rows) = access::rw(in_mat.n_cols) = access::rw(in_mat.n_elem) = access::rw(in_mat.c_size) = 0;
	in_mat.row_ptr = in_mat.col_idx = nullptr;
	in_mat.val_idx = nullptr;
	return *this;
}

template<typename data_t, typename index_t> const index_t* csr_form<data_t, index_t>::row_mem() const { return row_ptr; }

template<typename data_t, typename index_t> const index_t* csr_form<data_t, index_t>::col_mem() const { return col_idx; }

template<typename data_t, typename index_t> const data_t* csr_form<data_t, index_t>::val_mem() const { return val_idx; }

template<typename data_t, typename index_t> void csr_form<data_t, index_t>::reset() const {
	zeros();
	delete[] col_idx;
	delete[] row_ptr;
	delete[] val_idx;
}

template<typename data_t, typename index_t> void csr_form<data_t, index_t>::zeros() const { access::rw(c_size) = 0; }

template<typename data_t, typename index_t> data_t csr_form<data_t, index_t>::max() const { return *std::max_element(val_idx, val_idx + c_size); }

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::init() {
	reset();
	row_ptr = new(std::nothrow) index_t[n_rows + 1];
	col_idx = new(std::nothrow) index_t[n_elem];
	val_idx = new(std::nothrow) data_t[n_elem];

	if(col_idx == nullptr || row_ptr == nullptr || val_idx == nullptr) {
		reset();
		return false;
	}
	return true;
}

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::init(const index_t in_elem) {
	if(in_elem <= n_elem) {
		zeros();
		return true;
	}
	access::rw(n_elem) = in_elem;
	return init();
}

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::init(const index_t in_row, const index_t in_col, const index_t in_elem) {
	if(n_rows != in_row) access::rw(n_rows) = in_row;
	if(n_cols != in_col) access::rw(n_cols) = in_col;

	return init(in_elem);
}

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::resize() {
	const auto copy = *this;

	if(!init(n_elem == 0 ? 1 : 2 * n_elem)) return false;

	copy_memory(copy.c_size, copy.row_ptr, copy.col_idx, copy.val_idx);

	return true;
}

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::resize(const index_t in_elem) {
	const auto copy = *this;

	if(in_elem <= c_size || !init(in_elem)) return false;

	copy_memory(copy.c_size, copy.row_ptr, copy.col_idx, copy.val_idx);

	return true;
}

template<typename data_t, typename index_t> bool csr_form<data_t, index_t>::resize(const index_t in_row, const index_t in_col, const index_t in_elem) {
	const auto copy = *this;

	if(in_row < n_rows || in_col < n_cols || in_elem < c_size || !init(in_row, in_col, in_elem)) return false;

	copy_memory(copy.c_size, copy.row_ptr, copy.col_idx, copy.val_idx);

	return true;
}

template<typename data_t, typename index_t> void csr_form<data_t, index_t>::print() const {
	suanpan_info("A sparse matrix in triplet form with size of %u by %u, the sparsity of %.3f.\n", static_cast<unsigned>(n_rows), static_cast<unsigned>(n_cols), 100. - static_cast<double>(c_size) / static_cast<double>(n_rows * n_cols) * 100.);
	if(c_size > 1000) {
		suanpan_info("more than 1000 elements exist.\n");
		return;
	}

	index_t c_idx = 1;
	for(index_t I = 0; I < c_size; ++I) {
		if(I >= row_ptr[c_idx]) ++c_idx;
		suanpan_info("(%3u, %3u) ===> %+.4E\n", static_cast<unsigned>(c_idx) - 1, static_cast<unsigned>(col_idx[I]), val_idx[I]);
	}
}

template<typename data_t, typename index_t> template<typename T2> csr_form<data_t, index_t> csr_form<data_t, index_t>::operator*(const T2 scalar) {
	csr_form<data_t, index_t> copy = *this;

#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), copy.c_size, [&](const index_t I) { copy.val_idx[I] *= data_t(scalar); });
#else
	for(auto I = 0; I < copy.c_size; ++I) copy.val_idx[I] *= data_t(scalar);
#endif

	return copy;
}

template<typename data_t, typename index_t> template<typename T2> csr_form<data_t, index_t> csr_form<data_t, index_t>::operator/(const T2 scalar) {
	csr_form<data_t, index_t> copy = *this;

#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), copy.c_size, [&](const index_t I) { copy.val_idx[I] /= data_t(scalar); });
#else
	for(auto I = 0; I < copy.c_size; ++I) copy.val_idx[I] /= data_t(scalar);
#endif

	return copy;
}

template<typename data_t, typename index_t> template<typename T2> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator*=(const T2 scalar) {
#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), c_size, [&](const index_t I) { val_idx[I] *= data_t(scalar); });
#else
	for(auto I = 0; I < c_size; ++I) val_idx[I] *= data_t(scalar);
#endif

	return *this;
}

template<typename data_t, typename index_t> template<typename T2> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator/=(const T2 scalar) {
#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), c_size, [&](const index_t I) { val_idx[I] /= data_t(scalar); });
#else
	for(auto I = 0; I < c_size; ++I) val_idx[I] /= data_t(scalar);
#endif

	return *this;
}

template<typename data_t, typename index_t> template<typename in_dt, typename in_it> csr_form<data_t, index_t>::csr_form(triplet_form<in_dt, in_it>& old_mat) { *this = old_mat; }

template<typename data_t, typename index_t> template<typename in_dt, typename in_it> csr_form<data_t, index_t>::csr_form(triplet_form<in_dt, in_it>&& old_mat) { *this = old_mat; }

template<typename data_t, typename index_t> template<typename in_dt, typename in_it> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator=(const triplet_form<in_dt, in_it>& in_mat) {
	in_mat.csr_condense();

	init(index_t(in_mat.n_rows), index_t(in_mat.n_cols), index_t(in_mat.c_size));

	if(0 == in_mat.c_size) return *this;

	access::rw(c_size) = index_t(in_mat.c_size);

	std::transform(in_mat.col_idx, in_mat.col_idx + in_mat.c_size, col_idx, [](const in_it I) { return index_t(I); });
	std::transform(in_mat.val_idx, in_mat.val_idx + in_mat.c_size, val_idx, [](const in_dt I) { return data_t(I); });

	in_it current_pos = 0, current_row = 0;
	while(current_pos < in_mat.c_size)
		if(in_mat.row_idx[current_pos] < current_row) ++current_pos;
		else row_ptr[current_row++] = index_t(current_pos);

	row_ptr[n_cols] = c_size;

	return *this;
}

template<typename data_t, typename index_t> csr_form<data_t, index_t>::csr_form(const csc_form<data_t, index_t>& in_mat) { *this = in_mat; }

template<typename data_t, typename index_t> csr_form<data_t, index_t>& csr_form<data_t, index_t>::operator=(const csc_form<data_t, index_t>& in_mat) {
	csr_form<data_t, index_t>::init(in_mat.n_rows, in_mat.n_cols, in_mat.c_size);

#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), n_rows + 1, [&](const index_t I) { row_ptr[I] = 0; });
#else
	for(index_t I = 0; I <= n_rows; ++I) row_ptr[I] = 0;
#endif

	for(index_t I = 0; I < in_mat.c_size; ++I) ++row_ptr[in_mat.row_idx[I] + 1];
	for(index_t I = 2; I <= n_rows; ++I) row_ptr[I] += row_ptr[I - 1];

	std::vector<index_t> counter(n_rows, 0);

	index_t c_idx = 1;
	for(index_t I = 0; I < in_mat.c_size; ++I) {
		if(I >= in_mat.col_ptr[c_idx]) ++c_idx;
		const auto& r_idx = in_mat.row_idx[I];
		const auto c_pos = counter[r_idx]++ + row_ptr[r_idx];
		col_idx[c_pos] = c_idx - 1;
		val_idx[c_pos] = in_mat.val_idx[I];
	}

	access::rw(c_size) = in_mat.c_size;

	return *this;
}

template<typename data_t, typename index_t> const data_t& csr_form<data_t, index_t>::operator()(const index_t in_row, const index_t in_col) const {
	if(in_row < n_rows && in_col < n_cols) for(auto I = row_ptr[in_row]; I < row_ptr[in_row + 1]; ++I) if(col_idx[I] == in_col) return val_idx[I];

	access::rw(bin) = 0.;
	return bin;
}

template<typename data_t, typename index_t> Mat<data_t> csr_form<data_t, index_t>::operator*(const Col<data_t>& in_mat) {
	Mat<data_t> out_mat = arma::zeros<Mat<data_t>>(in_mat.n_rows, 1);

#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), static_cast<index_t>(n_rows), [&](const index_t I) { for(auto J = row_ptr[I]; J < row_ptr[I + 1]; ++J) out_mat(I) += val_idx[J] * in_mat(col_idx[J]); });
#else
	for(auto I = 0; I < n_rows; ++I) for(auto J = row_ptr[I]; J < row_ptr[I + 1]; ++J) out_mat(I) += val_idx[J] * in_mat(col_idx[J]);
#endif

	return out_mat;
}

template<typename data_t, typename index_t> Mat<data_t> csr_form<data_t, index_t>::operator*(const Mat<data_t>& in_mat) {
	Mat<data_t> out_mat = arma::zeros<Mat<data_t>>(in_mat.n_rows, in_mat.n_cols);

#ifdef SUANPAN_MT
	tbb::parallel_for(static_cast<index_t>(0), static_cast<index_t>(n_rows), [&](const index_t I) { for(uword K = 0; K < out_mat.n_cols; ++K) for(auto J = row_ptr[I]; J < row_ptr[I + 1]; ++J) out_mat(I, K) += val_idx[J] * in_mat(col_idx[J], K); });
#else
	for(uword K = 0; K < out_mat.n_cols; ++K) for(auto I = 0; I < n_rows; ++I) for(auto J = row_ptr[I]; J < row_ptr[I + 1]; ++J) out_mat(I, K) += val_idx[J] * in_mat(col_idx[J], K);
#endif

	return out_mat;
}

#endif
