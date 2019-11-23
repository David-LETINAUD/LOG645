#define errCheck(code) { errorCheck(code, __FILE__, __LINE__); }

#include <iostream>
#include <fstream>
#include <CL/opencl.h>

#include "parallel.hpp"

char * readFile(const char * fileName);
void addWithOpenCl(const int * a, const int * b, int * c, int elements, const char * kernelSource);

using std::cout;
using std::flush;
using std::endl;

inline void errorCheck(cl_int code, const char* file, int line) {
    if(CL_SUCCESS != code) {
        std::cout << "[" << file << ", line " << line << "]" << std::flush;
        std::cout << " OpenCL error code <" << code << "> received." << std::endl << std::flush;
        exit(EXIT_FAILURE);
    }
}

void solvePar(int rows, int cols, int iterations, double td, double h, double ** matrix, const char * kernelFileName) {
    cout << "Do OpenCl related stuff here!" << endl << flush;

    // Example.
    int elements = 5;
    const int a[elements] = { 1, 2, 3, 4, 5 }; 
    const int b[elements] = { 5, 4, 3, 2, 1 }; 
    int c[elements] = { 0 }; 

    char * kernelSource = readFile(kernelFileName);
    printf("%s\n", kernelSource);

    addWithOpenCl(a, b, c, elements, kernelSource);
}

char * readFile(const char * fileName) {
    int length;

    std::ifstream file(fileName, std::ifstream::in);
    file.seekg(0, std::ios::end);
    length = file.tellg();
    file.seekg(0, std::ios::beg);

    char * buffer = (char *) malloc(length);
    file.read(buffer, length);
    file.close();

    return buffer;
}

void addWithOpenCl(const int * a, const int * b, int * c, int elements, const char * kernelSource) {
    int bytes = elements * sizeof(int);
    cl_int err = CL_SUCCESS;

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
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &err);
    errCheck(err);

    // Compile the source program.
    cl_program program = clCreateProgramWithSource(context, 1, (const char **) &kernelSource, NULL, &err);
    errCheck(err);

    errCheck(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));

    // Setup an execution kernel from the source program.
    cl_kernel kernel = clCreateKernel(program, "addKernel", &err);
    errCheck(err);

    // Create device buffers.
    cl_mem dev_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &err);
    errCheck(err);
    cl_mem dev_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &err);
    errCheck(err);
    cl_mem dev_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, &err);
    errCheck(err);

    // Write host data to the device.
    errCheck(clEnqueueWriteBuffer(queue, dev_a, CL_TRUE, 0, bytes, a, 0, NULL, NULL));
    errCheck(clEnqueueWriteBuffer(queue, dev_b, CL_TRUE, 0, bytes, b, 0, NULL, NULL));

    // Setup function arguments.
    errCheck(clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_a));
    errCheck(clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_b));
    errCheck(clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_c));
    errCheck(clSetKernelArg(kernel, 3, sizeof(int), &elements));

    // Execute the kernel.
    size_t localSize = elements;
    size_t globalSize = elements;
    errCheck(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL));

    // Wait for the kernel the terminate.
    errCheck(clFinish(queue));

    // Write device data in our output buffer.
    errCheck(clEnqueueReadBuffer(queue, dev_c, CL_TRUE, 0, bytes, c, 0, NULL, NULL));

    // Clear memory.
    errCheck(clReleaseMemObject(dev_a));
    errCheck(clReleaseMemObject(dev_b));
    errCheck(clReleaseMemObject(dev_c));
    errCheck(clReleaseKernel(kernel));
    errCheck(clReleaseProgram(program));
    errCheck(clReleaseCommandQueue(queue));
    errCheck(clReleaseContext(context));

    cout << "c = { " << c[0] << flush;
    for(int i = 1; i < elements; i++) {
        cout << ", " << c[i] << flush;
    }

    cout << " }" << endl << flush;
}
