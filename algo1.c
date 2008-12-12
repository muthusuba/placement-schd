/**
 * Algo1.c
 *       Prioritize companies before running the algo.
 *
 * Copyright (C) Muthu Subramanian K 2008 Dec
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "sch.h"
#include "algo1.h"

void prioritize_companies(sch_data *data, uint32 *comp_pri, uchar interview)
{
    uchar flag=0;
    int i,j,c=0;
    int32 *weights;
    sch_names *names = data->names;
    weights=(int32 *)malloc(data->n_companies*sizeof(uint32));
    memset(weights, 0, data->n_companies*sizeof(uint32));

    memset(comp_pri, 0, data->n_companies*sizeof(uint32));
    printf("NCompanies: %d\n",data->n_companies);
    
    /* Add Bonus weights */
    for(i=0;i<data->n_companies;i++)
    {
        weights[i] += data->companies[i].force;
        LOG_INFO("Bonus Weight (force): %s:%d\n",data->companies[i].name,data->companies[i].force);
    }
    
    c=0;
    while(!flag)
    {
        for(i=0;i<data->n_names;i++)
        {
            for(j=0;j<names[i].pri.n;j++)
            {
                if(interview)
                {
                    if(j==c && names[i].pri.pris[j] != INVALID_COMPANY)
                        weights[names[i].pri.pris[j]]++;
                    flag=1;
                }
                else
                {
                    /* Actual weightage allocation */
                    weights[names[i].pri.pris[j]] += (MAX_PRIORITIES-j);
                    //weights[names[i].pri.pris[j]] += 1;
                }
            }
        }
        if(!interview)
            flag=1;
        else
            c++;
        if(c>MAX_PRIORITIES)
            flag=1;
    }

    /* Enter data into comp_pri */
    c=0;
    for(i=0;i<data->n_companies;i++)
    {
        for(j=0;j<data->n_companies;j++)
        {
            if(weights[j] > weights[c])
                c=j;
        }
        if(weights[c] != -1)
        {
            LOG_INFO("Priority List: %d: %d W:%d\n",i,c,weights[c]);
            weights[c] = -1;
            comp_pri[i] = c;
        }
    }
    free(weights);
}

