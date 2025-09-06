//////////////////////////
/* Name: Filter.cpp

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

This function filters the result obtained by DNS solution. Therefore,
it can be compared with LES solution.
For instance, the algorithm of filtration for point with index i (xi) and filter size = 2*Delt_x
can be written as :

u_bar[i] = (u[i-2]+u[i-1]+u[i]+u[i+1]+u[i+2])/5

This algorithm assumes that the boundaries are periodic.
*/

//////////////////////
//Filtering      
//////////////////////
void  filter(double *h, double *l, int Dim, int size) {


	double sum;
	int n, p, k;

	n = size + 1;           
	p = size / 2;            

	for (int i = 0; i <= Dim - 1; i++) {

		sum = 0;

		for (int j = 1; j <= n; j++) {

			k = i - p + (j - 1);

			if (k < 0) {

				k = Dim + k;    // periodic boundaries at right_hand_side 

			}

			else if (k > Dim - 1) {

				k = k - Dim;   // periodic boundaries at left_hand_side

			}

			sum = sum + h[k];

		}

		l[i] = sum / n;

	}

}
