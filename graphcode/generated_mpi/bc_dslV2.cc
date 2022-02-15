#include"bc_dslV2.h"

void Compute_BC(graph g,float* BC,std::vector<int> sourceSet)
{
  int my_rank,np,part_size,startv,endv;
  struct timeval start, end, start1, end1;
  long seconds,micros;
  mpi::communicator world;
  my_rank = world.rank();
  np = world.size();
  int max_degree,num_nodes;
  int *index,*rev_index, *weight,*edgeList;
  int *local_index,*local_rev_index, *local_weight,*local_edgeList;
  int dest_pro;
  if(my_rank == 0)
  {
    gettimeofday(&start, NULL);
    g.parseGraph();
    gettimeofday(&end, NULL);
    seconds = (end.tv_sec - start.tv_sec);
    micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    printf("The graph loading time = %ld micro secs.\n",micros);
    max_degree = g.max_degree();
    weight = g.getEdgeLen();
    edgeList = g.getEdgeList();
    index = g.getIndexofNodes();
    rev_index = g.rev_indexofNodes;
    num_nodes = g.num_nodes();
    part_size = g.num_nodes()/np;
    MPI_Bcast (&max_degree,1,MPI_INT,my_rank,MPI_COMM_WORLD);
    MPI_Bcast (&num_nodes,1,MPI_INT,my_rank,MPI_COMM_WORLD);
    MPI_Bcast (&part_size,1,MPI_INT,my_rank,MPI_COMM_WORLD);
    local_index = new int[part_size+1];
    local_rev_index = new int[part_size+1];
    for(int i=0;i<part_size+1;i++) {
      local_index[i] = index[i];
      local_rev_index[i] = rev_index[i];
    }
    int num_ele = local_index[part_size]-local_index[0];
    local_weight = new int[num_ele];
    for(int i=0;i<num_ele;i++)
    local_weight[i] = weight[i];
    local_edgeList = new int[num_ele];
    for(int i=0;i<num_ele;i++)
    local_edgeList[i] = edgeList[i];
    for(int i=1;i<np;i++)
    {
      int pos = i*part_size;
      MPI_Send (index+pos,part_size+1,MPI_INT,i,0,MPI_COMM_WORLD);
      MPI_Send (rev_index+pos,part_size+1,MPI_INT,i,1,MPI_COMM_WORLD);
      int start = index[pos];
      int end = index[pos+part_size];
      int count_ = end - start;
      MPI_Send (weight+start,count_,MPI_INT,i,2,MPI_COMM_WORLD);
      MPI_Send (edgeList+start,count_,MPI_INT,i,3,MPI_COMM_WORLD);
    }
    delete [] weight;
    delete [] edgeList;
    delete [] index;
  }
  else
  {
    MPI_Bcast (&max_degree,1,MPI_INT,0,MPI_COMM_WORLD); 
    MPI_Bcast (&num_nodes,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast (&part_size,1,MPI_INT,0,MPI_COMM_WORLD);
    local_index = new int[part_size+1];
    MPI_Recv (local_index,part_size+1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    local_rev_index = new int[part_size+1];
    MPI_Recv (local_rev_index,part_size+1,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    int num_ele = local_index[part_size]-local_index[0];
    local_weight = new int[num_ele];
    MPI_Recv (local_weight,num_ele,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    local_edgeList = new int[num_ele];
    MPI_Recv (local_edgeList,num_ele,MPI_INT,0,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  }
  for (int t = 0; t < g.num_nodes(); t ++) 
  {
    BC[t] = 0;
  }
  MPI_Barrier(MPI_COMM_WORLD);
  gettimeofday(&start, NULL);
  for (int i = 0; i < sourceSet.size(); i++)
  {
    int src = sourceSet[i];
    double* sigmafloat* delta=new float[g.num_nodes()];
    for (int t = 0; t < g.num_nodes(); t ++) 
    {
      delta[t] = 0;
    }
    for (int t = 0; t < g.num_nodes(); t ++) 
    {
      sigma[t] = 0;
    }
    sigma[src] = 1;
    int phase = 0 ;
    vector <int> active;
    vector<int> active_next;
    vector <vector<int>> p (part_size);
    int* d = new int[part_size];
    for (int t = 0; t < part_size; t++)
    {
      d[t] = -1;
    }
    active.push_back(src);
    if(src >= startv && src <= endv)
    {
      d[src] = 0;
    }
    while(is_finished(startv,endv,active))
    {
      vector < vector <float> > send_data(np);
      vector < vector <float> > receive_data(np);
      while (active.size() > 0)
      {
        int v = active.back();
        active.pop_back();
        if(v >=startv && v<= endv)
        {
          for (int edge = g.indexofNodes[v]; edge < g.indexofNodes[v+1]; edge ++) 
          {
            int w = g.edgeList[edge] ;
            if(w >= startv && w <= endv)
            {
              if (d[w-startv] < 0)
              {
                active_next.push_back(w);
                d[w-startv] = d[v-startv] + 1;
              }
              if (d[w-startv] == d[v-startv] + 1)
                {
                  p[w-startv].push_back(v);
                  sigma[w] = sigma[w-startv] + sigma[v-startv];
                }
              }
              else
              {
                dest_pro = w / part_size;
                send_data[dest_pro].push_back(d[v-startv]);
                send_data[dest_pro].push_back(w);
                send_data[dest_pro].push_back(v);
                send_data[dest_pro].push_back(sigma[v-startv]);
              }
            }
          }
        }
        all_to_all(world, send_data, receive_data);
        for(int t=0;t<np;t++)
        {
          if(t != my_rank)
          {
            for (int x=0; x < receive_data[t].size();x+=4)
            {
              int d_v = receive_data[t][x];
              int w = receive_data[t][x+1];
              int v = receive_data[t][x+2];
              double sigma_v = receive_data[t][x+3];
              if (d[w-startv] < 0 )
              {
                active_next.push_back(w);
                d[w-startv] = d_v + 1;
              }
              if (d[w-startv] == d_v+1)
              {
                p[w-startv].push_back(v);
                sigma[w-startv] = sigma[w-startv] + sigma_v;
              }
            }
          }
        }
        active.swap(active_next);
        active_next.clear();
        MPI_Barrier(MPI_COMM_WORLD);
        send_data.clear();
        receive_data.clear();
        phase = phase + 1 ;
      }
      phase = phase -1 ;
      bool* modified = new bool[g.num_nodes()];
      for (int t = 0; t < g.num_nodes(); t++)
      {
        modified[t] = false;
      }
      MPI_Barrier(MPI_COMM_WORLD);
      while (phase > 0)
      {
        vector <vector <float> > send_data(np);
        vector <vector <float> > receive_data(np);
        for(int v=startv; v<=endv; v++)
        {
          if(d[v-startv] == phase)
          {
            modified[v-startv] = true;
            for (int edge = g.indexofNodes[v]; edge < g.indexofNodes[v+1]; edge ++) 
            {
              int w = g.edgeList[edge] ;
              if(w >= startv && w <= endv)
              {
                delta[v] = delta[v-startv] + ( sigma[v-startv] / sigma[w-startv])  * ( 1 + delta[w-startv]) ;
              }
            }
          }
          if(d[v-startv] == (phase+1))
          {
            for (int j=0;j<p[v-startv].size();j++)
            {
              int w = p[v-startv][j];
              if(!(w >= startv && w <= endv))
              {
                dest_pro = w / part_size;
                send_data[dest_pro].push_back(v);
                send_data[dest_pro].push_back(w);
                send_data[dest_pro].push_back(delta[v-startv]);
                send_data[dest_pro].push_back(sigma[v-startv]);
              }
            }
          }
        }
        all_to_all(world, send_data, receive_data);
        for(int t=0;t<np;t++)
        {
          if(t != my_rank)
          {
            for (int x=0; x < receive_data[t].size();x+=4)
            {
              int w = receive_data[t][x];
              int v = receive_data[t][x+1];
              float delta_w = receive_data[t][x+2];
              double sigma_w = receive_data[t][x+3];
              delta[v-startv] = delta[v-startv] + ( sigma[v-startv] / sigma_w)  * ( 1 + delta_w) ;
            }
          }
        }
        for (int v=startv;v<=endv;v++)
        {
          if( v != src && modified[v] == true)
          {
            modified[v] = false;
            BC[v] = BC[v-startv] + delta[v-startv];
          }
        }
        phase--;
        MPI_Barrier(MPI_COMM_WORLD);
        send_data.clear();
        receive_data.clear();
      }
    }

    gettimeofday(&end, NULL);
    seconds = (end.tv_sec - start.tv_sec);
    micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    if(my_rank==0)
    {
      printf("The iteration time = %ld micro secs.\n",micros);
      printf("The iteration time = %ld secs.\n",seconds);
    }
    MPI_Finalize();
}