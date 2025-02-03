#pragma once
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <format>

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

class VectorfAdder {
public:
  VectorfAdder(std::filesystem::path filepath) {
    WithPathToKernel(filepath);
    init();
  }
  ~VectorfAdder() {
    clearAllContexts();
  }

  std::shared_ptr<std::vector<float>> Add(const std::vector<float> &A,
                         const std::vector<float> &B) {
    size_t size = A.size();
    if (size != B.size())
      throw std::runtime_error("A.size is not equal to B.size");

    createBuffers(A, B);
    enqueue(size);
    return waitAndReturn(size);
  }

  void WithPathToKernel(std::filesystem::path filepath) {
    kernel_file_path = filepath;
  }

  void Refresh() {
    clearAllContexts();
    init();
  }

private:
  std::filesystem::path kernel_file_path;
  std::string kernelString;
  size_t kernelStringSize;

  cl_int err;

  cl_platform_id platform_id;
  cl_device_id device_id;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;

  cl_mem buffer_A_h;
  cl_mem buffer_B_h;
  cl_mem buffer_result_h;

  void init() {
    WithPathToKernel(kernel_file_path);
    boilerplate();
  }
  
  void clearAllContexts()
  {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
  }

  void err_check(std::string msg = "none") {
    if (err != CL_SUCCESS)
        throw std::runtime_error(std::format("An exception occured during OpenCL kernel handling: {}. Additional info: {}.", err, msg));
  }

  void boilerplate() {
    readKernelSource();

    err = clGetPlatformIDs(1, &platform_id, nullptr);
    err_check();

    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);
    err_check();

    context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &err);
    err_check();

    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    err_check();

    const char* ks = kernelString.c_str();
    program = clCreateProgramWithSource(context, 1, &ks, &kernelStringSize, &err);
    err_check();

    err = clBuildProgram(program,  1, &device_id, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
      size_t build_log_size = 1000;
      clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &build_log_size);
      std::string build_log;
      build_log.resize(build_log_size);
      clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, build_log_size, build_log.data(), nullptr);
      err_check(build_log);
    }

    kernel = clCreateKernel(program, "_vec_add_float", &err);
    err_check();
  }

  void createBuffers(const std::vector<float> &A,
                         const std::vector<float> &B) {
    size_t size = A.size();
  
    buffer_A_h = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), (void*)A.data(), &err);
    err_check();
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_A_h);

    buffer_B_h = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), (void*)B.data(), &err);
    err_check();
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_B_h);

    buffer_result_h = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(float), nullptr, &err);
    err_check();
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_result_h);
  }

  void readKernelSource() {
    if (std::filesystem::is_regular_file(kernel_file_path)) {
      std::ifstream file(kernel_file_path);
      if (!file.is_open())
        throw std::runtime_error("Failed to open file: " +
                                 kernel_file_path.string());

      std::ostringstream oss;
      oss << file.rdbuf();

      kernelString = oss.str();
      kernelStringSize = kernelString.size();
      return;
    }
    throw std::runtime_error(
        "Passed path is not a valid path to an OpenCL kernel: " +
        kernel_file_path.string());
  }

  void enqueue(size_t size)
  {
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &size, nullptr, 0, nullptr, nullptr);
    err_check();
  }

  std::shared_ptr<std::vector<float>> waitAndReturn(size_t size) {
    std::shared_ptr<std::vector<float>> buf (std::make_shared<std::vector<float>>(size));
    err = clEnqueueReadBuffer(queue, buffer_result_h, CL_TRUE, 0, size * sizeof(float), (void*)&(*buf)[0], 0, NULL, NULL);
    clFinish(queue);
    return buf;
  };
};
