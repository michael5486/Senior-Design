#include <stdio.h>

/* RSU == real sense units...they are close to meters, but not exactly meters
   so we have chosen to keep them as it's own measurement */

double convertToInches(double, double);

int main() {
	printf("RSU to Inches converter\n");

	printf("RSU= %f, Pixels=%f Output=%f\n", 1650.0, 112.0, convertToInches(1650, 112));
	printf("RSU= %f, Pixels=%f Output=%f\n", 1800.0, 99.04, convertToInches(1800, 99.04));
	printf("RSU= %f, Pixels=%f Output=%f\n", 2100.0, 88.0, convertToInches(2100.0, 88.0));
	printf("RSU= %f, Pixels=%f Output=%f\n", 2450.0, 74.67, convertToInches(2450.0, 74.67));


}

double convertToInches(double zAxisInRSU, double pixelDistance) {

	double inchesPerPixel = 0.000119 * zAxisInRSU + 0.00308;
	return inchesPerPixel * pixelDistance;

}