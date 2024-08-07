#include<bits/stdc++.h>
#include<mpi.h>
#define BILLION 1000000000L
const double pi = 3.141592;
 
using namespace std;  

/*
*                      Φ = cos(2πy)
*       (-1,1) +--------------------+ (1,1)
*              |                    |
*              |         y          |
*              |         ^          |
* Φ = cos(2πy) |         |          | Φ = cos(2πy)
*              |         ---> x     |
*              |                    |
*              |                    |
*      (-1,-1) +--------------------+ (1,-1)
*                      Φ = cos(2πy)
*/

void saveToCSV(vector<vector<double>>& data) {
    ofstream file("data.csv");
    

    for (const auto& row : data) { 
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) {
                file << ",";
            }
        }
        file << endl;
    } 
 
    file.close();
} 
double f(double x, double y){
    return -1*((x*x)+(y*y));
}

double g(double x, double y){
    if(x==-1.0){
        return cos(2*pi*y);
        // return 0.0;
    }
    if(x==1.0){
        return cos(2*pi*y);
        // return 0.0;
    }
    if(y==-1.0){
        return cos(2*pi*x);
        // return 0.0;
    }
    return cos(2*pi*x); 
        // return 0.0;

}

double closest_boundary_dist(double x, double y, double delta, int n){

    double dist = 0.0;
    dist = min(1.0+x, min(1.0-x,min(1.0+y,1.0-y)));
    return dist; 
}
 
void random_dance(vector<vector<double>> &a, double delta, int N, int nprocs, int myid, int n, double r_min){
    
    int i_myid = 0; 
    for (int i = 0; i < n; i++)
    {
        if((i%nprocs) == myid){
        if(i==0 || i==n-1){
            for (int j = 0; j < a[0].size(); j++)
            {
                a[i_myid][j] = g((i*delta)-1, (j*delta)-1);
            }  
             
        }
        else{ 
        for (int j = 0; j < (a[0].size()); j++)
        {   
            if(j==0 || j==(a[0].size()-1)){
                a[i_myid][j] = g((i*delta)-1, (j*delta)-1);
            }
            else{
            for (int k = 0; k < N; k++) 
            {
                int i1 = i, j1 = j;  
                double xi = (i*delta)-1, yi = (j*delta)-1;
                while(1){

                    double min_dist = closest_boundary_dist(xi, yi, delta, n);
                    a[i_myid][j] += f(xi,yi)*min_dist*min_dist/4;
                    if(min_dist<r_min){

                        if(min_dist == 1.0-xi){
                            a[i_myid][j] += g(1.0, yi);
                        }
                        else if(min_dist == 1.0+xi){
                            a[i_myid][j] += g(-1.0, yi);
                        }
                        else if(min_dist == 1.0-yi){
                            a[i_myid][j] += g(xi, 1.0);
                        }
                        else if(min_dist == 1.0+yi){
                            a[i_myid][j] += g(xi, -1.0);
                        }
                        break;

                    }
                    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                    std::mt19937 generator(seed);
                    std::uniform_real_distribution<double> distribution(0.0, 1.0);
                    double rand_num = distribution(generator); 

                    xi += (min_dist * cos(rand_num*pi*2));
                    yi += (min_dist * sin(rand_num*pi*2));
                    
                }

            }
            a[i_myid][j] /= N; 
            }            
            
        } 
        }
        i_myid++;
  
        }
        
    }
    
}

int32_t main(int argc, char* argv[]){

    int myid, nprocs, proc;

    struct timespec start_t,end_t;
    uint64_t diff;

    clock_gettime(CLOCK_MONOTONIC,&start_t);
    
    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    double delta = 0.0;
    

    int N = 1; 
    if(myid == 0){cin>>delta; cin>>N; }
    MPI_Bcast(&delta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);


    int n = (2.0/delta)+1 ;  
    vector<vector<double>> phi((n/nprocs)+nprocs,vector<double>(n,0));

    double r_min = delta/4;

    random_dance(phi, delta, N, nprocs, myid, n, r_min);

      
    if(myid!=0){
        int limit = (n/nprocs);
        if(myid<(n%nprocs)){limit++;}
        for (int i = 0; i < limit; i++)
        {
            MPI_Send(&phi[i][0], n, MPI_DOUBLE, 0, (i*nprocs)+myid, MPI_COMM_WORLD);
        }
         
    }
    else{  
        vector<vector<double>> final_phi(n,vector<double>(n,0));
        int i_myid_0 = 0;
        for(int i=0; i<n; i++){
 
            if((i%nprocs) == myid){
                for (int j = 0; j < n; j++)
                { 
                    final_phi[i][j] = phi[i_myid_0][j];
                }
                i_myid_0++; 
                
            }  
            else{
                MPI_Recv(&final_phi[i][0], n, MPI_DOUBLE, (i%nprocs), i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }  
    saveToCSV(final_phi);  
    }

    if(myid==0){
        clock_gettime(CLOCK_MONOTONIC,&end_t);

        diff = BILLION*(end_t.tv_sec - start_t.tv_sec) + end_t.tv_nsec - start_t.tv_nsec;
        printf("\n elapsed time = %lf seconds \n",(double)diff/BILLION);
    }
    
    MPI_Finalize(); 
    return 0;
}  