
#ifndef RAND_H
#define RAND_H

#define ZIPF_CONSTANT 0.99

class zipf_generator {
	private:
		long items; // Number of items
		long base; // Min item to generate
		double zconstant = ZIPF_CONSTANT; // Zipf constant to use
		/* Computed parameters for generating the distribution */
		double alpha, zetan, eta, theta, zeta2theta;
		/* The number of items used to compute zetan the last time */
		long countforzeta;
		/* Prevent recompute for large item set */
		bool allowitemcountdecrease = false;		
		
		inline static double zetastatic(long n, double theta) {
			return zetastatic(0, n, theta, 0);
		}
		
		inline double zeta(long n, double theta) {
			countforzeta = n;
			return zetastatic(n, theta);
		}

		inline double zeta(long st, long n,
				double theta, double initialsum) {
			countforzeta = n;
			return zetastatic(st, n, theta, initialsum);
		}
		static double zetastatic(long st, long n,
				double theta, double initialsum);
		long next(long itemcount);
	public:
		zipf_generator(long min, long max,
				double constant);
		long next();

};

#endif


