#define errCheck(code) { errorCheck(code, __FILE__, __LINE__); }

#include <iostream>
#include <fstream>
#include <CL/opencl.h>

#include "windows.h"
#include "parallel.hpp"
#include "matrix.hpp"

char * readFile(const char * fileName);
//void addWithOpenCl(const int * a, const int * b, int * c, int elements, const char * kernelSource);
void addWithOpenCl(int rows, int cols, int iterations, double td, double h, double* initial_matrix, double* final_matrix, const char* kernelSource);

using std::cout;
using std::flush;
using std::endl;

inline void errorCheck(cl_int code, const char* file, int line) {
	if (CL_SUCCESS != code) {
		std::cout << "[" << file << ", line " << line << "]" << std::flush;
		std::cout << " OpenCL error code <" << code << "> received." << std::endl << std::flush;
		Sleep(3000);
		exit(EXIT_FAILURE);
	}
}

void solvePar(int rows, int cols, int iterations, double td, double h, double* initial_matrix, double* final_matrix, const char * kernelFileName) {
	cout << "Do OpenCl related stuff here!" << endl << flush;

	// Example.
	/*const int matrix_size = (const int)rows * (const int)cols;

	double *initial_matrix = (double*) malloc(matrix_size * sizeof(double));
	double *final_matrix = (double*)malloc(matrix_size * sizeof(double));*/

	//convert_to_1d_matrix(rows, cols, matrix, initial_matrix);

	char * kernelSource = readFile(kernelFileName);
	//printf("%s\n", kernelSource);
	
	Sleep(3000);

	addWithOpenCl(rows, cols, iterations, td, h, initial_matrix, final_matrix, kernelSource);
	//convert_to_2d_matrix(rows, cols, final_matrix, matrix);
}

char * readFile(const char * fileName) {
	int length;

	std::ifstream file(fileName, std::ifstream::in | std::ios::binary);
	file.seekg(0, std::ios::end);
	length = file.tellg();
	file.seekg(0, std::ios::beg);

	char * buffer = (char *) malloc(length + 1);
	file.read(buffer, length);
	file.close();

	buffer[length] = '\0';

	return buffer;
}

void addWithOpenCl(int rows, int cols, int iterations, double td, double h, double* initial_matrix, double* final_matrix, const char* kernelSource) {
	// TO check error_code : https://gist.github.com/bmount/4a7144ce801e5569a0b6

	//int bytes = elements * sizeof(int);
	int matrix_size = (const int)rows * (const int)cols;
	size_t matrix_bytes = sizeof(double) * matrix_size;
	
	cl_int err = CL_SUCCESS;

	///////////////////////////////////////////////////////////////////////////////// Does not change
	// Get execution platform.
	cl_platform_id platform;
	errCheck(clGetPlatformIDs(1, &platform, NULL));

	// Get available gpus on platform.
	cl_device_id device_id;
	errCheck(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));

	// Create an execution context.
	cl_context context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	errCheck(err);

	// Create the command queue.
	cl_command_queue queue = clCreateCommandQueue(context, device_id, NULL, &err);
	errCheck(err);

	// Compile the source program.
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, &err);
	errCheck(err);

	errCheck(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));

	// Setup an execution kernel from the source program.
	cl_kernel kernel = clCreateKernel(program, "addKernel", &err);
	errCheck(err);

	/////////////////////////////////////////////////////////////////////////////////


	// Create device buffers.
	/*cl_mem dev_rows = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(rows), NULL, &err);
	errCheck(err);
	cl_mem dev_cols = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cols), NULL, &err);
	errCheck(err);
	cl_mem dev_it = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(iterations), NULL, &err);
	errCheck(err);
	cl_mem dev_td = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(td), NULL, &err);
	errCheck(err);
	cl_mem dev_h = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(h), NULL, &err);
	errCheck(err);*/
	cl_mem dev_initial_matrix = clCreateBuffer(context, CL_MEM_READ_WRITE, matrix_bytes, NULL, &err);
	errCheck(err);
	cl_mem dev_final_matrix = clCreateBuffer(context, CL_MEM_READ_WRITE, matrix_bytes, NULL, &err);
	errCheck(err);

	/*cl_mem dev_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &err);
	errCheck(err);
	cl_mem dev_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &err);
	errCheck(err);
	cl_mem dev_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, &err);
	errCheck(err);*/

	// Write host data to the device.
	/*errCheck(clEnqueueWriteBuffer(queue, dev_a, CL_TRUE, 0, bytes, a, 0, NULL, NULL));
	errCheck(clEnqueueWriteBuffer(queue, dev_b, CL_TRUE, 0, bytes, b, 0, NULL, NULL));*/

	/*errCheck(clEnqueueWriteBuffer(queue, dev_rows, CL_TRUE, 0, sizeof(rows), rows, 0, NULL, NULL));
	errCheck(clEnqueueWriteBuffer(queue, dev_cols, CL_TRUE, 0, sizeof(cols), cols, 0, NULL, NULL));
	errCheck(clEnqueueWriteBuffer(queue, dev_it, CL_TRUE, 0, sizeof(iterations), iterations, 0, NULL, NULL));
	errCheck(clEnqueueWriteBuffer(queue, dev_td, CL_TRUE, 0, sizeof(td), td, 0, NULL, NULL));
	errCheck(clEnqueueWriteBuffer(queue, dev_h, CL_TRUE, 0, sizeof(h), h, 0, NULL, NULL));*/
	errCheck(clEnqueueWriteBuffer(queue, dev_initial_matrix, CL_TRUE, 0, matrix_bytes, initial_matrix, 0, NULL, NULL));
	//errCheck(clEnqueueWriteBuffer(queue, dev_final_matrix, CL_TRUE, 0, sizeof(double) * matrix_size, final_matrix, 0, NULL, NULL));


	// Setup function arguments.
	/*errCheck(clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_a));
	errCheck(clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_b));
	errCheck(clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_c));
	errCheck(clSetKernelArg(kernel, 3, sizeof(int), &elements));*/
	errCheck(clSetKernelArg(kernel, 0,	sizeof(rows), &rows));
	errCheck(clSetKernelArg(kernel, 1,  sizeof(cols), &cols));
	errCheck(clSetKernelArg(kernel, 2,	sizeof(iterations), &iterations));
	errCheck(clSetKernelArg(kernel, 3,  sizeof(td), &td));
	errCheck(clSetKernelArg(kernel, 4,  sizeof(h), &h));
	errCheck(clSetKernelArg(kernel, 5,  sizeof(cl_mem), &dev_initial_matrix));
	errCheck(clSetKernelArg(kernel, 6,  sizeof(cl_mem), &dev_final_matrix));

	size_t preffered_wg_size;
	errCheck(clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &preffered_wg_size, NULL));

	size_t max_wg_size;
	errCheck(clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &max_wg_size, NULL));

	cout << "TAILLE PREF: " << preffered_wg_size << "\n";
	cout << "TAILLE MAX : " << max_wg_size << "\n";
	// Execute the kernel.
	cout << "MATRIX SIZE: " << matrix_size << "\n";
	//const size_t localSize = matrix_size;
	//const size_t globalSize =  matrix_size;

	int number_of_blocks = matrix_size / max_wg_size;
	if (number_of_blocks % max_wg_size != 0) number_of_blocks++;

	const size_t localSize = max_wg_size;
	const size_t globalSize =  number_of_blocks * max_wg_size;
	
	for (int k = 0; k < iterations; k++) {
		errCheck(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL));

		// Wait for the kernel the terminate.
		errCheck(clFinish(queue));

		// Write device data in our output buffer.
		//errCheck(clEnqueueReadBuffer(queue, dev_c, CL_TRUE, 0, bytes, c, 0, NULL, NULL));
		errCheck(clEnqueueReadBuffer(queue, dev_final_matrix, CL_TRUE, 0, matrix_bytes, final_matrix, 0, NULL, NULL));
		errCheck(clEnqueueWriteBuffer(queue, dev_initial_matrix, CL_TRUE, 0, matrix_bytes, final_matrix, 0, NULL, NULL));
	}

	// Clear memory.
	/*errCheck(clReleaseMemObject(dev_a));
	errCheck(clReleaseMemObject(dev_b));
	errCheck(clReleaseMemObject(dev_c));*/

	/*errCheck(clReleaseMemObject(dev_rows));
	errCheck(clReleaseMemObject(dev_cols));
	errCheck(clReleaseMemObject(dev_it));
	errCheck(clReleaseMemObject(dev_td));
	errCheck(clReleaseMemObject(dev_h));*/
	errCheck(clReleaseMemObject(dev_initial_matrix));
	errCheck(clReleaseMemObject(dev_final_matrix));


	errCheck(clReleaseKernel(kernel));
	errCheck(clReleaseProgram(program));
	errCheck(clReleaseCommandQueue(queue));
	errCheck(clReleaseContext(context));

	/*cout << "c = { " << final_matrix[0] << flush;
	for (int i = 1; i < matrix_size; i++) {
		cout << ", " << final_matrix[i] << flush;
		if (!i % cols)
			cout << '\n' << flush;
	}

	cout << " }" << endl << flush;*/
}