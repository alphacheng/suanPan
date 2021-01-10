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
 * @class BandMatSpike
 * @brief A BandMatSpike class that holds matrices.
 *
 * @author tlc
 * @date 08/01/2020
 * @version 0.1.0
 * @file BandMatSpike.hpp
 * @addtogroup MetaMat
 * @{
 */

// ReSharper disable CppCStyleCast
#ifndef BANDMATSPIKE_HPP
#define BANDMATSPIKE_HPP

extern "C" {
void spikeinit_(int*, int*, int*);
void dspike_tune_(int*);
void dspike_gbsv_(int*, int*, int*, int*, int*, double*, int*, double*, int*, int*);
void dspike_gbtrf_(int*, int*, int*, int*, double*, int*, double*, int*);
void dspike_gbtrs_(int*, const char*, int*, int*, int*, int*, double*, int*, double*, double*, int*);
void sspike_tune_(int*);
void sspike_gbsv_(int*, int*, int*, int*, int*, float*, int*, float*, int*, int*);
void sspike_gbtrf_(int*, int*, int*, int*, float*, int*, float*, int*);
void sspike_gbtrs_(int*, const char*, int*, int*, int*, int*, float*, int*, float*, float*, int*);
}

template<typename T> class BandMatSpike final : public MetaMat<T> {
	using MetaMat<T>::TRAN;

	static T bin;

	const uword l_band;
	const uword u_band;
	const uword m_rows; // memory block layout

	podarray<T> WORK;
	podarray<int> SPIKE = podarray<int>(64);

	void init_spike();
public:
	using MetaMat<T>::memory;
	using MetaMat<T>::n_rows;
	using MetaMat<T>::n_cols;
	using MetaMat<T>::factored;

	BandMatSpike();
	BandMatSpike(uword, uword, uword);

	unique_ptr<MetaMat<T>> make_copy() override;

	const T& operator()(uword, uword) const override;
	T& at(uword, uword) override;

	Mat<T> operator*(const Mat<T>&) override;

	int solve(Mat<T>&, const Mat<T>&) override;
	int solve_trs(Mat<T>&, const Mat<T>&) override;

	[[nodiscard]] int sign_det() const override;
};

template<typename T> T BandMatSpike<T>::bin = 0.;

template<typename T> void BandMatSpike<T>::init_spike() {
	auto N = static_cast<int>(n_rows);
	auto KLU = static_cast<int>(std::max(l_band, u_band));

	spikeinit_(SPIKE.memptr(), &N, &KLU);

	std::is_same<T, float>::value ? sspike_tune_(SPIKE.memptr()) : dspike_tune_(SPIKE.memptr());
}

template<typename T> BandMatSpike<T>::BandMatSpike()
	: MetaMat<T>()
	, l_band(0)
	, u_band(0)
	, m_rows(0) {}

template<typename T> BandMatSpike<T>::BandMatSpike(const uword in_size, const uword in_l, const uword in_u)
	: MetaMat<T>(in_size, in_size, (in_l + in_u + 1) * in_size)
	, l_band(in_l)
	, u_band(in_u)
	, m_rows(in_l + in_u + 1) { init_spike(); }

template<typename T> unique_ptr<MetaMat<T>> BandMatSpike<T>::make_copy() { return make_unique<BandMatSpike<T>>(*this); }

template<typename T> const T& BandMatSpike<T>::operator()(const uword in_row, const uword in_col) const {
	if(in_row > in_col + l_band || in_row + u_band < in_col) return bin = 0.;

	return memory[in_row + u_band + in_col * (m_rows - 1)];
}

template<typename T> T& BandMatSpike<T>::at(const uword in_row, const uword in_col) {
	if(in_row > in_col + l_band || in_row + u_band < in_col) return bin = 0.;

	return access::rw(memory[in_row + u_band + in_col * (m_rows - 1)]);
}

template<typename T> Mat<T> BandMatSpike<T>::operator*(const Mat<T>& X) {
	Mat<T> Y(size(X));

	auto M = static_cast<int>(n_rows);
	auto N = static_cast<int>(n_cols);
	auto KL = static_cast<int>(l_band);
	auto KU = static_cast<int>(u_band);
	T ALPHA = 1.;
	auto LDA = static_cast<int>(m_rows);
	auto INC = 1;
	T BETA = 0.;

#ifdef SUANPAN_MT
	tbb::parallel_for(0llu, X.n_cols, [&](const uword I) {
		if(std::is_same<T, float>::value) {
			using E = float;
			arma_fortran(arma_sgbmv)(&TRAN, &M, &N, &KL, &KU, (E*)&ALPHA, (E*)this->memptr(), &LDA, (E*)X.colptr(I), &INC, (E*)&BETA, (E*)Y.colptr(I), &INC);
		} else if(std::is_same<T, double>::value) {
			using E = double;
			arma_fortran(arma_dgbmv)(&TRAN, &M, &N, &KL, &KU, (E*)&ALPHA, (E*)this->memptr(), &LDA, (E*)X.colptr(I), &INC, (E*)&BETA, (E*)Y.colptr(I), &INC);
		}
	});
#else
	for(uword I = 0; I < X.n_cols; ++I)
		if(std::is_same<T, float>::value) {
			using E = float;
			arma_fortran(arma_sgbmv)(&TRAN, &M, &N, &KL, &KU, (E*)&ALPHA, (E*)this->memptr(), &LDA, (E*)X.colptr(I), &INC, (E*)&BETA, (E*)Y.colptr(I), &INC);
		} else if(std::is_same<T, double>::value) {
			using E = double;
			arma_fortran(arma_dgbmv)(&TRAN, &M, &N, &KL, &KU, (E*)&ALPHA, (E*)this->memptr(), &LDA, (E*)X.colptr(I), &INC, (E*)&BETA, (E*)Y.colptr(I), &INC);
		}
#endif

	return Y;
}

template<typename T> int BandMatSpike<T>::solve(Mat<T>& X, const Mat<T>& B) {
	auto INFO = 0;

	X = B;

	auto N = static_cast<int>(n_rows);
	auto KL = static_cast<int>(l_band);
	auto KU = static_cast<int>(u_band);
	auto NRHS = static_cast<int>(X.n_cols);
	auto LDAB = static_cast<int>(m_rows);
	auto LDB = static_cast<int>(X.n_rows);
	const auto KLU = static_cast<uword>(std::max(KL, KU));

	WORK.zeros(KLU * KLU * SPIKE(9));

	if(std::is_same<T, float>::value) {
		using E = float;
		sspike_gbtrf_(SPIKE.memptr(), &N, &KL, &KU, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), &INFO);
		sspike_gbtrs_(SPIKE.memptr(), &TRAN, &N, &KL, &KU, &NRHS, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), (E*)X.memptr(), &LDB);
	} else {
		using E = double;
		dspike_gbtrf_(SPIKE.memptr(), &N, &KL, &KU, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), &INFO);
		dspike_gbtrs_(SPIKE.memptr(), &TRAN, &N, &KL, &KU, &NRHS, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), (E*)X.memptr(), &LDB);
	}

	if(INFO == 0) factored = true;
	else suanpan_error("solve() receives error code %u from the base driver, the matrix is probably singular.\n", INFO);

	return INFO;
}

template<typename T> int BandMatSpike<T>::solve_trs(Mat<T>& X, const Mat<T>& B) {
	if(!factored) return solve(X, B);

	X = B;

	auto N = static_cast<int>(n_rows);
	auto KL = static_cast<int>(l_band);
	auto KU = static_cast<int>(u_band);
	auto NRHS = static_cast<int>(X.n_cols);
	auto LDAB = static_cast<int>(m_rows);
	auto LDB = static_cast<int>(X.n_rows);

	if(std::is_same<T, float>::value) {
		using E = float;
		sspike_gbtrs_(SPIKE.memptr(), &TRAN, &N, &KL, &KU, &NRHS, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), (E*)X.memptr(), &LDB);
	} else {
		using E = double;
		dspike_gbtrs_(SPIKE.memptr(), &TRAN, &N, &KL, &KU, &NRHS, (E*)this->memptr(), &LDAB, (E*)WORK.memptr(), (E*)X.memptr(), &LDB);
	}

	return SUANPAN_SUCCESS;
}

template<typename T> int BandMatSpike<T>::sign_det() const { throw invalid_argument("not supproted"); }

#endif

//! @}
