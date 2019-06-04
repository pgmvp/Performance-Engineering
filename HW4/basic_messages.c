#include <mpi.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    int numtasks, rank, dest, source, rc, count, tag=1;
    int inmsg[] = {1, 2};
	int outmsg[] = {3, 4};
    MPI_Status Stat;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        dest = 1;
        source = 1;
        rc= MPI_Send(&outmsg, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
        rc= MPI_Recv(&inmsg, 2, MPI_INT, source, tag, MPI_COMM_WORLD,&Stat);
    }
    else if (rank == 1) {
        dest = 0;
        source = 0;
        rc= MPI_Recv(&inmsg, 2, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
        printf ("Task 1: Received '%d' \n", *inmsg);
        rc= MPI_Send(&outmsg, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }  
    rc= MPI_Get_count(&Stat, MPI_CHAR, &count);
    printf("Task%d: Received %d char(s) from task %d with tag %d \n",
    rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);
    MPI_Finalize();
    return(0);
}
