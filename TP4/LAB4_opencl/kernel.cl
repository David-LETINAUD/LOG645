/*__kernel void addKernel(__global int * a, __global int * b, __global int * c, int elements) {
    int id = get_global_id(0);
    
    if(id < elements) {
        c[id] = a[id] + b[id];
    }
}
*/

__kernel void addKernel(int rows, int cols, int iterations, double td, double h, __global double* initial_matrix, __global double* final_matrix) {
	int id = get_global_id(0);


	/*bool condition = id < cols || id + 1 % cols == 0 || id + 1 % cols == 1 || id > cols * (rows - 1);
	if (condition == true)
	{
		final_matrix[id] = 0;
	}
	else if (id < rows*(cols-1))
	{
		
		double c, l, r, t, b;
		double h_square = h * h;

		c = initial_matrix[id];
		t = initial_matrix[id - rows];
		b = initial_matrix[id + rows];
		l = initial_matrix[id - 1];
		r = initial_matrix[id + 1];


		final_matrix[id] = (double)c* (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
	}*/
	
	/*double c, l, r, t, b;
	double h_square = h * h;

	c = initial_matrix[id];
	t = (id - rows) > 0 ? initial_matrix[id - rows] : 0;
	b = (id + rows) < rows * cols ? initial_matrix[id + rows] : 0;
	l = (id + 1) % cols != 1 ? initial_matrix[id - 1] : 0;
	r = (id + 1) % cols != 0 ? initial_matrix[id + 1] : 0;


	final_matrix[id] = (double)c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);*/

	bool condition_gauche_droite = id % cols == 0 || id % cols == cols - 1;
	bool condition_haut_bas = id < cols || id > cols * (rows - 1);
	bool condition = condition_gauche_droite || condition_haut_bas;
	
	double c, l, r, t, b;
	double h_square = h * h;

	/*c = initial_matrix[id];
	t = id >= rows : initial_matrix[id - rows] : 0;
	b = id <= rows * (cols - 1) : initial_matrix[id + rows] : 0;
	l = id > 0 : initial_matrix[id - 1] : 0;
	r = id < rows * cols ? initial_matrix[id + 1] : 0;*/

	c = initial_matrix[id];
	t = initial_matrix[id - cols];
	b = initial_matrix[id + cols];
	l = initial_matrix[id - 1];
	r = initial_matrix[id + 1];

	final_matrix[id] = !condition ? (double)c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square) : 0;

}