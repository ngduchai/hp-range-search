
#include "rand.h"
#include "math.h"
#include <iostream>
#include <random>

using namespace std;

zipf_generator::zipf_generator(long min, long max,
		double constant) {
	items = max - min + 1;
	base = min;
	zconstant = constant;
	theta = zconstant;
	zeta2theta = zeta(2, theta);
	alpha = 1.0 / (1.0 - theta);
	zetan = zetastatic(max - min + 1, constant);
	countforzeta = items;
	eta = (1 - pow(2.0/items, 1 - theta)) / (1 - zeta2theta/zetan);
	next();
}

double zipf_generator::zetastatic(long st, long n,
		double theta, double initialsum) {
	double sum = initialsum;
	for (long i = st; i < n; i++) {
		sum += 1 / pow(i + 1, theta);
	}
	return sum;
}

long zipf_generator::next(long itemcount) {
	if (itemcount != countforzeta) {
		if (itemcount > countforzeta) {
			zetan = zeta(countforzeta, itemcount, theta, zetan);
			eta = (1 - pow(2.0/items, 1 - theta)) /
				(1 - zeta2theta/zetan);
		}else if ((itemcount < countforzeta) &&
				allowitemcountdecrease) {
			cout << "Recomput the distribution" << endl;
			zetan = zeta(itemcount, theta);
			eta = (1 - pow(2.0/items, 1 - theta)) /
				(1 - zeta2theta / zetan);
		}
	}
	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_real_distribution<double> unif(0.0, 1.0);
	double u = unif(rng);
	double uz = u * zetan;
	if (uz < 1.0) {
		return base;
	}

	if (uz < 1.0 + pow(0.5, theta)) {
		return base + 1;
	}

	long ret = base + (long)((itemcount) * pow(eta * u - eta + 1, alpha));
	return ret;
}

long zipf_generator::next() {
	return next(items);
}


