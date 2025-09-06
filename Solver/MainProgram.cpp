////////////////////////
/*Name : MainProgram

Update: 05/19/2017

The MIT License

Copyright (c) <2017> <Mohammad Hashemi>
Mechanical Engineering Department, University of Pittsburgh (US)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Description:

The one dimensional Burgers equation is solved numericaly.
The form of the Burgers equation considered here is:

du       du             d^2 u
-- + u * -- = kinvis  * -----
dt       dx              dx^2

for -1.0 < x < +1.0, and 0.0 < t.

The numerical method used is finite difference method. (second order
in space and first order in time)
Two types of solution for Burgers equation were mentioned here :
1. DNS (direct numerical solution) 
2. LES (large eddy simulation).
In DNS, the fintie difference scheme of Burgers equation is solved in
high resolution to capture all information.
In large eddy simulation (LES), an artificial viscosity is added to
normal viscosity (kinvis+ LESvis). So, the resolution can be reduced 
and save us computational cost.
*/

#include "NUMERICALDERIVATION.h"

int main()
{
	double Delt_t, Delt_t1, Delt_t2, tf , time;
	double kinvis; // kinematic viscousity
	int it;       // numbe of iteration
	int chk, k;  // chk=number of timesteps to write the output, k= outfile index  
	int l;
	double a[Dim];      // intermediate variable
	double u_bar[Dim];  // filtered velocity 
	
	
	bool filter_opt;
    
	// Smagorinsky model parameters 
	int size =4;    // filter size
	int size2 =4;
	int sign = -1;
	double Cs = 0.3;
	double vis_LES [Dim];
	bool LES_opt = false; 
	char chr;
		

	void outputfile(double *p, int xDim, int k, bool LES_opt, bool filter_opt, int size, double time);
	void  filter(double *h, double *l, int Dim, int size);
	
	cout << "////////////////////Numerical Solution of One Dimensional Burgers' Equation////////////////////" << '\n';
	cout << "Please choose the kinematic viscosity of your fluid\n";
	cout << "(The reasonable range of kinematic viscosity is kinvis= [0.001 - 1.0] and lower value of viscosity may lead to divergence of the calculation)\n";
	
	kinvis = 0.001;
	
	cout << "The default value of kinematic viscosity is kinvis=0.001\n";
	char char1 = 'n';
	cout << "If you want to change it, please first press 'y' and then press ENTER. Otherwise, Press anyother alphabet botton\n";
	cin >> char1;
	
	if (char1 == 'y') {
		cout << "Please enter your value and then press enter\n";
		cin >> kinvis;
	}

	cout << '\n';
	cout << '\n';
	
	// final time, time step and iteration
	cout << "Please choose the final time of your solution\n";
	cout << "(The reasonable range of time is t = [0.0 - 2.0] and time does not have any unit since the problem is solved dimensionless)\n";
	
	tf = 1.001399; 
	
	cout << "the default value of final time is time=1.001399\n";
	
	char char2 = 'n';
	cout << "If you want to change it, please first press 'y' and then press enter. Otherwise, press anyother alphabet botton\n";
	cin >> char2;

	if (char2 == 'y') {
		cout << "Please enter your value and then press enter\n";
		cin >> tf;
	}
	
	cout << '\n';
	cout << '\n';

	// Stability condition kinvis*Delt_t/(Delt_x)^2 <0.5
	Delt_t = grids.Delt_x*grids.Delt_x*0.5 / kinvis;
	it = floor(tf / Delt_t);
	chk = 30;
	k = 1;
	cout.precision(15);   // Number of decimal point 

	
	///////////////////// DNS & LES /////////////////////
	chr = 'c';
	while ((chr != 'a') && (chr != 'b')) {
		cout << "////////////////////Type of Solution////////////////////" << '\n';
		cout << "Please choose the type of solution: DNS(a) or LES(b)\n";
		cout << "Please type a or b by your keyboard and then press ENTER\n";
		cin >> chr;
		if (chr == 'b') LES_opt = true;
	}
	////////////////////////////////////////////////////

	filter_opt = false;
	time = 0.0;

	for (int t = 1; t <= it; t++) {

		cout << "Iteration= " << t << '\n';
		cout << "time step= " << Delt_t << '\n'; // timestep is printed, since it changes each time step.



		for (int i = 1; i <= grids.xDim - 2; i++) {
			
			if (grids.first_dev(i) <= 0) {

				sign = -1;
			}
			else {

				sign = 1;
			}


			if (LES_opt) {
				
				vis_LES [i] = 2 * Cs * (grids.Delt_x*grids.Delt_x)*sign*grids.first_dev(i);   
				Delt_t1 = grids.Delt_x*grids.Delt_x*0.5 / (kinvis+vis_LES[i]);
				if (Delt_t> Delt_t1) Delt_t = Delt_t1;

			}

			else {

				vis_LES[i] = 0.0; 
			}

		}
		
		/////////////////////

		for (int i = 1; i <= grids.xDim - 2; i++) {

			

			a[i] = grids.u[i] - Delt_t * grids.u[i] * grids.first_dev(i) + Delt_t * (kinvis + vis_LES[i]) * grids.sec_dev(i); 
	

		}

		for (int i = 1; i <= grids.xDim - 2; i++) {

			grids.u[i] = a[i];
			
		}

		if (t == k*it / chk) {

			if (!LES_opt) {
				filter ( grids.u, u_bar, grids.xDim, size);
				filter_opt = true;
				outputfile(u_bar, grids.xDim, k, LES_opt, filter_opt, size, time);
				filter_opt = false;
			}
			outputfile(grids.u, grids.xDim, k, LES_opt, filter_opt, size, time);
			k++;
		}

		time = time + Delt_t;

	}
  	
	cout << "////////////////////Simulation Setup////////////////////" << '\n';
	cout << "Number of grids: ";
	cout << Dim << '\n';
	cout << "Final time: ";
	cout << time << '\n';
	cout << "Kinematic viscosity: ";
	cout << kinvis << "   Re= 1/kinvis" <<'\n';
	cout << "Output files were produced\n";
	cout << "Please push any alphabet botton and then press enter to exit the program\n";
	cin >> l;

	
	return 0;
}

///////////////////////////////
// Output file
//////////////////////////////

void  outputfile(double *p, int xDim, int k, bool LES_opt, bool filter_opt, int size, double time) {


	string string1, string2, LES_DNS, filter, s;
	int counter;

	filter = "";
	string1 = "output";
	string2 = ".txt";

	if (LES_opt) {
		LES_DNS = "LES";
		counter = 1;
	}
	else
	{
		LES_DNS = "DNS";
		counter = 1;
	}

	if ((filter_opt) && (!LES_opt)) {

		filter = "Filtered";
		counter = size;
	}

	s = to_string(k);
	string1 = string1 + LES_DNS + filter + s + string2;
	ofstream myfile;
	myfile.open(string1);

	
	myfile << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
	myfile << "Outputfile\n";
	myfile << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
	myfile << "\n";
	myfile << "\n";
	myfile << " Time= " << time << '\n';

	myfile << "Variable 'x'\n";


	for (int i = 0; i <= (xDim - 1) / counter; i++) {

		myfile << i*1.0/ (xDim - 1) << '\n';

	}

	myfile << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

	myfile << "Variable 'u'\n";

	for (int i = 0; i <= (xDim - 1) / counter; i++) {

		myfile << p[i*counter] << '\n';

	}

	myfile << "END.\n";

	myfile.close();

}

