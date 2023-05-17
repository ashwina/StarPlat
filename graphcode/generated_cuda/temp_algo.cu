// FOR BC: nvcc bc_dsl_v2.cu -arch=sm_60 -std=c++14 -rdc=true # HW must support CC 6.0+ Pascal or after
#include "temp_algo.h"

void count_nodes(graph& g)

{
  // CSR BEGIN
  int V = g.num_nodes();
  int E = g.num_edges();

  printf("#nodes:%d\n",V);
  printf("#edges:%d\n",E);
  int* edgeLen = g.getEdgeLen();







  // CSR END
  //LAUNCH CONFIG
  const unsigned threadsPerBlock = 512;
  unsigned numThreads   = (V < threadsPerBlock)? 512: V;
  unsigned numBlocks    = (V+threadsPerBlock-1)/threadsPerBlock;


  // TIMER START
  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  float milliseconds = 0;
  cudaEventRecord(start,0);


  //DECLAR DEVICE AND HOST vars in params

  //BEGIN DSL PARSING 
  int* d_prop;
  cudaMalloc(&d_prop, sizeof(int)*(V));

  int x; // asst in .cu

  int y; // asst in .cu

  int z; // asst in .cu

  int w = 0; // asst in .cu

  y = x + 10;
  z = x;
  initKernel<int> <<<numBlocks,threadsPerBlock>>>(V,d_prop,(int)(x || z));

  cudaMemcpyToSymbol(::y, &y, sizeof(int), 0, cudaMemcpyHostToDevice);
  count_nodes_kernel<<<numBlocks, threadsPerBlock>>>(V,E,d_prop);
  cudaDeviceSynchronize();




  //cudaFree up!! all propVars in this BLOCK!
  cudaFree(d_prop);

  //TIMER STOP
  cudaEventRecord(stop,0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds, start, stop);
  printf("GPU Time: %.6f ms\n", milliseconds);

} //end FUN
