/*__kernel void addKernel(__global int * a, __global int * b, __global int * c, int elements) {
    int id = get_global_id(0);
    
    if(id < elements) {
        c[id] = a[id] + b[id];
    }
}
*/

__kernel void addKernel(int rows, int cols, int iterations, double td, double h, __global double* initial_matrix, __global double* final_matrix) {
	int id = get_global_id(0);

	//int i = id / rows;
	int j = id % rows;
	int index = id + j;
	int one_row = rows + j;

	double c, l, r, t, b;
	double h_square = h * h;

	c = initial_matrix[index];
	t = initial_matrix[index + one_row];
	b = initial_matrix[index - one_row];
	l = initial_matrix[index-1];
	r = initial_matrix[index+1];


	final_matrix[index] = (double)c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);

}