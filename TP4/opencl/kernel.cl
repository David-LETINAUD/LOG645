__kernel void addKernel(__global int * a, __global int * b, __global int * c, int elements) {
    int id = get_global_id(0);
    
    if(id < elements) {
        c[id] = a[id] + b[id];
    }
}
