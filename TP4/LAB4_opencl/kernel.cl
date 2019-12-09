__kernel void addKernel(int rows, int cols, int iterations, double td, double h, __global double* initial_matrix, __global double* final_matrix) {
	int id = get_global_id(0);

	// Border conditions.
	bool condition_gauche_droite = id % cols == 0 || id % cols == cols - 1;
	bool condition_haut_bas = id < cols || id > cols * (rows - 1);
	bool condition = condition_gauche_droite || condition_haut_bas;
	
	double c, l, r, t, b;
	double h_square = h * h;
	
	// Saving dependencies.
	c = initial_matrix[id];
	t = initial_matrix[id - cols];
	b = initial_matrix[id + cols];
	l = initial_matrix[id - 1];
	r = initial_matrix[id + 1];

	// If no borders => calcul otherwise => 0.
	final_matrix[id] = !condition ? (double)c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square) : 0;
}