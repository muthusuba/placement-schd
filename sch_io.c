/**
 * sch_io.c Scheduler I/O handlers
 *
 * Copyright (C) Muthu Subramanian K Nov 2008
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sch.h"
#include "data.h"
#include "sch_io.h"

void debug_students(sch_data *data)
{
    int i;
    for(i=0;i<data->n_names;i++)
    {
        LOG_INFO("%s:%d\n",data->names[i].name,data->names[i].regno);
    }
}
void debug_companies(sch_data *data)
{
    int i;
    for(i=0;i<data->n_companies;i++)
    {
        if(data->companies[i].n_gd)
            LOG_INFO("%s, %d\n",data->companies[i].name,data->companies[i].n_gd);
        else
            LOG_INFO("%s, %d\n",data->companies[i].name,data->companies[i].n_interview);
    }
}

void debug_priorities(sch_names *name)
{
    int i;
    for(i=0;i<name->pri.n;i++)
    {
        LOG_INFO("\t%d\n",name->pri.pris[i]);
    }
}

void debug_rooms(sch_data *data)
{
    int i;
    for(i=0;i<data->rooms.n;i++)
    {
        LOG_INFO("%s\n",data->rooms.room[i]);
    }
}

void debug_data(sch_data *data)
{
    int i=0;
    sch_names *name = data->names;
    for(i=0;i<data->n_names;i++)
    {
        LOG_INFO("%s:%d\n",name[i].name,name[i].regno);
        debug_priorities(&name[i]);
    }
}

int read_company_name(FILE *fp, char *company)
{
    int t;
    if(fgets(company,MAX_NAME_SIZE,fp) == NULL)
    {
        LOG_ERROR("EOF!!!\n");
        return 0;
    }
    t=strlen(company);
    company[t-1] = '\0';
  
    return 1;
}

int read_company_info(FILE *fp, sch_comp *comp)
{
    int ch;
    if(!read_company_name(fp, comp->name))
        return 0;

    if(fscanf(fp,"%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
       &comp->n_panels, 
       &comp->students_per_panel, 
       &comp->start_time, 
       &comp->end_time,
       &comp->ppt_time,
       &comp->wr_test_time,
       &comp->gd_time,
       &comp->force,
       &ch)==EOF)
    {
        LOG_ERROR("EOF!!!\n");
        return 0;
    }
    comp->is_ppt_complete = (char)ch;

    return 1;
}

int read_student_list(FILE *fp, sch_names **names, int n, sch_data *data)
{
    int i=0;
    int32 reg_no;

    if(n<=0 || names == NULL)
        return 1;
    
    for(i=0;i<(n-1);i++)
    {
        if(fscanf(fp,"%d ",&reg_no)==EOF)
            break;

        names[i]=get_sch_names_from_reg(reg_no, data);
        if(names[i] == NULL)
        {
            LOG_ERROR("student_list: Invalid Regno: %d\n",reg_no);
        }
    }
    if(fscanf(fp,"%d\n",&reg_no)==EOF)
    {
        LOG_ERROR("EOF!!!\n");
        return 0;
    }
    names[i] = get_sch_names_from_reg(reg_no, data);
    if(names[i] == NULL)
    {
        LOG_ERROR("student_list: Invalid Regno: %d\n",reg_no);
    }

    return 1;
}

int read_company_any_list(FILE *fp, uchar *is, uint32 *n, sch_names ***list, sch_data *data)
{
    int ch;
    
    if(fscanf(fp,"%d\n",&ch) == EOF)
    {
        LOG_ERROR("EOF!!!\n");
        return 0;
    }
    *is = (char)ch;
    //if(!(char)ch)
    if(0)
    {
        fscanf(fp,"%d\n",n);
        *list=(sch_names **)malloc((*n)*sizeof(sch_names **));
        if(!read_student_list(fp,*list,*n, data))
        {
            LOG_ERROR("EOF!!!\n");
            return 0;
        }
    }

    return 1;
}

/* Optimize/Cleanup */
int read_company_all_list(FILE *fp, sch_comp *comp, sch_data *data)
{
    /* Written test */
    if(!read_company_any_list(fp, 
                              &comp->is_wr_test_complete, 
                              &comp->n_written_test, 
                              &comp->written_test,
                              data))
    {
        return 0;
    }

   /* GD List */
    if(!read_company_any_list(fp, 
                              &comp->is_gd_complete, 
                              &comp->n_gd, 
                              &comp->gd_list,
                              data))
    {
        return 0;
    }

    /* Interview list */
    if(!read_company_any_list(fp, 
                              &comp->is_interview_complete, 
                              &comp->n_interview, 
                              &comp->interview_list,
                              data))
    {
        return 0;
    }

    return 1;
}

void read_companies(FILE *fp, sch_data *data)
{
    int n,i;
    fscanf(fp,"%d\n",&n);
    data->n_companies = n;
    if(n>MAX_COMPANIES)
    {
        LOG_ERROR("MAX COMPANIES MAXD - recompilation required\n");
    }
    if(data->companies == NULL)
    {
        data->companies = allocate_sch_comp(n);
    }
    for(i=0;i<n;i++)
    {
        if(!read_company_info(fp, &data->companies[i]))
            break;
        if(!read_company_all_list(fp, &data->companies[i], data))
            break;
    }
}
void read_priorities(FILE *fp, sch_names *name)
{
    int i,n;
    fscanf(fp,"%d\n",&n);
    name->pri.n = n;
    for(i=0;i<n;i++)
    {
        if(fscanf(fp,"%d\n",&name->pri.pris[i])==EOF)
        {
            LOG_ERROR("EOF!!!\n");
            break;
        }
        if(name->pri.pris[i])
            name->pri.pris[i]--;
    }
}

void read_rooms(FILE *fp, sch_data *data)
{
    int i,n,t,ch;
    fscanf(fp,"%d\n",&n);
    data->rooms.n = n;
    if(n>MAX_ROOMS)
    {
        LOG_ERROR("MAX ROOMS MAXD - recompilation required\n");
    }
    for(i=0;i<n;i++)
    {
        if(fgets(data->rooms.room[i], MAX_NAME_SIZE, fp) == NULL)
        {
            LOG_ERROR("EOF!!!\n");
            break;
        }
        t=strlen(data->rooms.room[i]);
        data->rooms.room[i][t-1] = '\0';
        fscanf(fp,"%d %d\n",&data->rooms.capacity[i], &ch);
        data->rooms.type[i]=(uchar)ch;
        data->rooms.company[i] = NULL;
        data->rooms.student_list[i] = NULL;
        data->rooms.in_use[i] = 0;
        data->rooms.end_time[i] = 0;
        data->rooms.interview_end_time[i] = 0;
    }
}

void read_names(FILE *fp, sch_data *data)
{
    int i,n,t;
    sch_names *name=NULL;
    fscanf(fp, "%d\n",&n);
    data->n_names = n;
    
    name = data->names;
    if(name == NULL)
    {
        name = allocate_sch_names(n);
        data->names = name;
    }

    for(i=0;i<n;i++)
    {
        if(fgets(name[i].name, MAX_NAME_SIZE, fp) == NULL)
        {
            break;
        }
        t=strlen(name[i].name);
        name[i].name[t-1] = '\0';
        fscanf(fp, "%d\n",&name[i].regno);
        read_priorities(fp, &name[i]);
    }
}

void read_global(FILE *fp, sch_data *data)
{
    fscanf(fp,"%d\n",&data->ppt_percent);
    if(data->ppt_percent > 99)
        data->ppt_percent = 99;
    fscanf(fp,"%d\n",&data->wave_time);
}

/* Interview company priority list reading */
company_priority *read_company_priority_list(FILE *fp)
{
    company_priority *comp_pri=NULL;
    int i,j,n,r,p;

    comp_pri = (company_priority *)malloc(sizeof(company_priority));
    fscanf(fp,"%d\n",&n);
    for(i=0;i<n;i++)
    {
        fscanf(fp,"%d ",&r);
        comp_pri->n[i]=r;
        printf("Company: %d NStudents: %d\n",i,r);
        comp_pri->rollno[i] = (uint32 *)malloc(sizeof(uint32)*r);
        for(j=0;j<r;j++)
        {
            fscanf(fp,"%d ",&p);
            comp_pri->rollno[i][j] = p;
        }
        fscanf(fp,"\n");
    }
    return comp_pri;
}

sch_data *read_from_file(char *filename)
{
    FILE *fp;
    sch_data *data;

    fp=fopen(filename,"r");
    if(fp==NULL)
    {
        LOG_ERROR("File open error: %s\n",filename);
        return NULL;
    }

    data = allocate_sch_data();
    read_global(fp, data);
    read_companies(fp,data);
    read_rooms(fp,data);
    read_names(fp,data);
    data->comp_pri = read_company_priority_list(fp);

#ifdef DEBUG
    debug_companies(data);
    debug_rooms(data);
    debug_data(data);
    debug_students(data);
#endif

    fclose(fp);
    return data;
}


