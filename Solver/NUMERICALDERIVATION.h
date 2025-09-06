///////////////////////////////////
/* Name : NumericalDerivation.h

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

This file defines a class which initializes u( x, t) @ t=0. Also,
it calculates the first and second numerical derivatives of u(x,t)
by x. Finite difference method is used to approximate the derivatives
of our function.
The number of grid points and grid size are set in this class as well.
*/

/////////////////////////////////
#ifndef NUMERICALDERIVATION_H
#define NUMERICALDERIVATION_H
#include <fstream>
#include <string>
#include <iostream>
#include <cmath>

using namespace std;

// number of grids

const int Dim= 1601;

/* includes the values of a function on the grid points and
 calculate their first and second derivation*/

class mygrid {

public:

	mygrid(); //constructor (Initialize the variables 'u')
	int xDim = Dim;
	const double pi = 3.1415926535;
	double u[Dim];
	double Delt_x;
	double first_dev(int i);
	double sec_dev(int i);

}grids;


#endif

// the constructor: initialization
mygrid::mygrid()
{
	Delt_x = 1.0 / (xDim - 1.0);
	
	for (int i = 0; i <= xDim - 1; i++) {

		u[i] = sin(pi*Delt_x*i);

	}

	cout << "The initial function is u( x, t=0)=sin(pi*x). Do you want to use your input file?** Please press 'y' as YES or anyother alphabet as NO.  Then ENTER\n";
	char ans;
	cin >> ans;
	if (ans=='y') {
		   
	   string line;
	   ifstream myfile("example.txt");

	   if (myfile.is_open())
	
	      {
		     int index=0;
		     bool act=false;
		
		     while (getline(myfile, line))
		
		    {
			
			       if (act) {

				           u[index] = stof(line);
				           index++;

			        }
			
			        if (line =="Variable u")   	act = true;
			

		     }
			 
		myfile.close();
		xDim= index;
	   }
    
	   Delt_x = 1.0 / (xDim - 1.0);
	   
	}
	    
		cout << "mygrids initialized\n";
}

// second derivation
double mygrid::sec_dev(int i)
{
	double u_2dev;
	
	u_2dev = (u[i + 1] - 2 * u[i] + u[i - 1]) / (Delt_x*Delt_x);

	return u_2dev;
}

// first derivation 
double mygrid::first_dev(int i)
{
	double u_dev;
	
	u_dev = (u[i + 1] - u[i - 1]) / (2 * Delt_x);

	return u_dev;
}


