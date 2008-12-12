/**
 * sch.c Core scheduler
 *
 * Copyright (C) Muthu Subramanian K Nov 2008
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "sch.h"
#include "sch_io.h"
#include "algo1.h"
#include "algo2.h"
#include "sch_output.h"

/**
 * Experimental main
 */
int main(int dummy, char *argc[])
{
    sch_data *data;
    output_list *output;
    uint32 *comp_pri;
    data = read_from_file("data.txt");
    uint32 interview=0, swap=0;

    if(dummy > 1)
        interview=atoi(argc[1]);
    if(dummy > 2)
        swap=atoi(argc[2]);
    enable_swapping_for_interview(swap);
    /* Algo 1*/
    comp_pri=(uint32 *)malloc(sizeof(uint32)*data->n_companies);
    prioritize_companies(data, comp_pri,interview);
   
    /* Update Company list */
    make_student_list_for_companies(data);

    /* Output no.of students in each company */
    debug_companies(data);

    /* Algo 2*/
    if(1)
    {
        output=allocate_students_any(data,comp_pri);
    }

    /* Print Output */
    process_output(data, output);
    return 0;
}
