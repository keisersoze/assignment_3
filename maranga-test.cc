#include <iostream>
#include <vector>
#include <ctime>
#include <memory>
#include <thread>

#include"matrix.h"
#include"matrix_wrap.h"
#include"operations.h"


template<typename T, unsigned h, unsigned w>
void initializeCells(matrix<T, h, w> &m, long rowMultiplier, long colMultiplier) {
	for (unsigned row = 0; row < m.get_height(); ++row) {
		for (unsigned col = 0; col < m.get_width(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

int main() {
	matrix<long long, 400, 5000> mA;
	initializeCells(mA, 12, 5);
	matrix<long long, 400, 5000> mB;
	initializeCells(mB, 7, 13);

	matrix<long long, 5000, 2000> mC;
	initializeCells(mC, 3, 8);
	matrix<long long, 5000, 2000> mD;
	initializeCells(mD, 4, 7);

	matrix<long long, 2000, 400> mE;
	initializeCells(mE, 8, 9);
	matrix<long long, 2000, 400> mF;
	initializeCells(mF, 5, 11);

	matrix<long long, 400, 1001> mG;
	initializeCells(mG, 54, 11);
	matrix<long long, 1001, 400> mH;
	initializeCells(mH, 66, 11);

	/*std::cout << "Matrix A" << std::endl;
	mA.print("%2d");

	std::cout << "Matrix B" << std::endl;
	mB.print("%2d");

	std::cout << "Matrix C" << std::endl;
	mC.print("%2d");

	std::cout << "Matrix D" << std::endl;
	mD.print("%2d");

	std::cout << "Matrix E" << std::endl;
	mE.print("%2d");

	std::cout << "Matrix F" << std::endl;
	mF.print("%2d");*/

	// auto multiplication = mB * mC;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	matrix<long long> multiplication = (mA + mB) * (mC + mD) * (mE + mF);
	long first = multiplication(0, 0);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "The first element is " << first << ", and it took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0 << " seconds" << std::endl;

	std::cout << "((A + B) * (C + D) * (E + F)) + (G * H)" << std::endl;
	std::cout << "Oks" << std::endl;

	return 0;
}
