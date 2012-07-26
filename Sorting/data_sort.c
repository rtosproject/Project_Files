#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <errno.h>

#define LIST_SIZE 30

typedef struct {
  int proc_id;
  double start_time;
  double end_time;
} data_store;

#include "array.h"

int main( int argc, char *argv[] ){

  if(argc != 2){
    printf("provide <input file>\n");
    return EXIT_FAILURE;
  }
  
  int tdata_tv_sec, tdata_tv_nsec;
  int param_sched_priority;

  char sdata[0x400];
  

  data_store initial[LIST_SIZE];    //first array to store initial unsorted data structs

  int i, c;
  int struct_counter = 0;          //used to keep track for sorting

  for(c=1; c<argc; c++){

    //open data files
    FILE *data_in = fopen(argv[c], "r");
  

    if(data_in == NULL){
      perror( "Error opening data file");
    }


    //Cuts out first line of file (which starts with finish time)
    fgets(sdata, sizeof(sdata), data_in);
    sscanf( sdata, "{ prio:%*i, event:'finish',sec:%*i,nsec:%*i }\n" , &param_sched_priority, &tdata_tv_sec, &tdata_tv_nsec );



    //read in from data files and place in array of structs
    for(i = (0 + (c-1)*LIST_SIZE); i < (LIST_SIZE-1 + ((c-1)*LIST_SIZE)); i++){

      fgets(sdata, sizeof(sdata), data_in);
      sscanf( sdata, "{ prio:%i, event:'start', sec:%i, nsec:%i }\n" , &param_sched_priority, &tdata_tv_sec, &tdata_tv_nsec );
      //printf("Param_Sched: %i, Sec: %i, Nsec: %i, Loop Count: %i\n", param_sched_priority, tdata_tv_sec, tdata_tv_nsec, i);

      //Store start time and process id in struct
      initial[i].proc_id = param_sched_priority;    //where is process id found?
      initial[i].start_time = tdata_tv_sec + tdata_tv_nsec*(0.000000001);

      fgets(sdata, sizeof(sdata), data_in);
      sscanf( sdata, "{ prio:%i, event:'finish',sec:%i,nsec:%i }\n" , &param_sched_priority, &tdata_tv_sec, &tdata_tv_nsec );
      //printf("Param_Sched: %i, Sec: %i, Nsec: %i, Loop Count: %i\n", param_sched_priority, tdata_tv_sec, tdata_tv_nsec, i);
      
      //Store end time in struct
      initial[i].end_time = tdata_tv_sec + tdata_tv_nsec*(0.000000001);

      struct_counter++;
    }

    

    
    fclose(data_in);
  }

  //debugging
  /*  
      for(i=0; i < (LIST_SIZE - 1); i++){
      printf("Process id: %i, Start Time: %.9f, End Time: %.9f\n", initial[i].proc_id, initial[i].start_time, initial[i].end_time);
    }
  */


  //bubble sort "initial" struct array
  int j;
  data_store temp;

  for (i = 0; i < (struct_counter - 1); i++){
    for (j = 0; j < (struct_counter - 1 - i); j++ ){
      if (initial[j].start_time > initial[j+1].start_time){
	temp = initial[j+1];
	initial[j+1] = initial[j];
	initial[j] = temp;
      }
    }
  }

  //debugging
  /*
  for(i=0; i < (LIST_SIZE - 1); i++){
    printf("Process id: %i, Start Time: %.9f, End Time: %.9f\n", initial[i].proc_id, initial[i].start_time, initial[i].end_time);
  }
  */
    

  //data_store *chopped = (data_store*) malloc(10 * sizeof(data_store));
  
  
  array chopped;
  array_init(&chopped, LIST_SIZE);
  
  data_store dummy;
  int count = 0;

  //chop up array accordingly
  for (i=0; i < (LIST_SIZE - 1); i++){
    if(initial[i].end_time > initial[i+1].end_time){      //if task is interrupted, task i+1 has higher priority than task i
      dummy.proc_id = initial[i].proc_id;
      dummy.start_time = initial[i+1].end_time;         //new chunk starts when higher priority finishes
      dummy.end_time = initial[i].end_time;             //end times are the same
      initial[i].end_time = initial[i+1].start_time;    //first chunk of task i ends when task i+1 interrupts it
      

      array_push(&chopped, initial[i]);
      array_push(&chopped, initial[i+1]);
      array_push(&chopped, dummy);

      /*
      chopped.a[count] = initial[i];
      chopped.a[count+1] = initial[i+1];
      chopped.a[count+2] = dummy;
   
      count=+3;
    

      chopped = (data_store*) realloc(chopped, 3 * count * sizeof(data_store));
      */

    }

    else{              //if task isn't interrupted
      array_push(&chopped, initial[i]);

      /*
      chopped.a[count] = initial[i];
      count++;
      
      chopped = (data_store*) realloc(chopped, 3 * count * sizeof(data_store));
      */
    }
    
  }


  printf("Size of chopped arrray: %i\n", chopped.s);
  
  for(i=0; i < (chopped.s - 1); i++){
    printf("Process id: %i, Start Time: %.9f, End Time: %.9f\n", chopped.a[i].proc_id, chopped.a[i].start_time, chopped.a[i].end_time);
  }
  

  return 0;

}
