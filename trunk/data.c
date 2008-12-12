/**
 * data.c
 *      Data structures and common utilities.
 * Copyright (C) Muthu Subramanian K 2008 Dec
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "sch.h"
#include "sch_io.h"


char *room_names[]={
    "GD",
    "Interview",
    "Test",
    "PPT",
    "Any"
};


sch_names *get_sch_names_from_reg(uint32 regno, sch_data *data)
{
    int i;
    sch_names *names = data->names;

    if(names != NULL)
    {
        for(i=0;i<data->n_names;i++)
        {
            if(names[i].regno == regno)
            {
                return &names[i];
            }
        }
    }

    return NULL;
}

sch_data *allocate_sch_data(void)
{
    sch_data *data;
    data = (sch_data *)malloc(sizeof(sch_data));
    if(data == NULL)
    {
        LOG_ERROR("BUFFER ALLOCATION FAILED!!!\n");
    }
    data->names = NULL;
    data->n_names = 0;
    data->n_companies = 0;
    data->companies = NULL;
    data->rooms.n = 0;
    return data;
}

sch_comp *allocate_sch_comp(int n)
{
    sch_comp *comp;
    int i;

    comp = (sch_comp *) malloc(sizeof(sch_comp)*n);
    if(comp == NULL)
    {
        LOG_ERROR("NAME ALLOCATION ERROR...\n");
        return NULL;
    }
    for(i=0;i<n;i++)
    {
        comp[i].name[0] = 0;
        comp[i].n_panels = 0;
        comp[i].students_per_panel = 0;
        comp[i].start_time = 0;
        comp[i].end_time = 0;
        comp[i].ppt_time = 0; /* in min */
        comp[i].is_ppt_complete = FALSE;
        comp[i].gd_list = NULL;
        comp[i].is_gd_complete = FALSE;
        comp[i].interview_list = NULL;
        comp[i].is_interview_complete = FALSE;
        comp[i].written_test = NULL;
        comp[i].is_wr_test_complete = FALSE;
    }
    return comp;
}

sch_names *allocate_sch_names(int n)
{
    int i;
    sch_names *name;
    name = (sch_names *) malloc(sizeof(sch_names)*n);
    if(name == NULL)
    {
        LOG_ERROR("NAME ALLOCATION ERROR...\n");
        return NULL;
    }
    for(i=0;i<n;i++)
    {
        name[i].regno = 0;
        name[i].pri.n = 0;
        name[i].end_time = 0;
        name[i].n_interviews=0;
        //name[i].next = NULL;
    }
    return name;
}

companies *allocate_comp(void)
{
    companies *comps;
    comps = (companies *)malloc(sizeof(companies));
    comps->next = NULL;
    comps->company = NULL;
    return comps;
}

students *allocate_student(void)
{
    students *student;
    student = (students *)malloc(sizeof(students));
    student->next = NULL;
    student->name = NULL;
    return student;
}

void add_to_output(output_list **head, output_list *output)
{
    output_list *data = *head;

    if(*head == NULL)
    {
        *head = output;
    }
    else
    {
        while(data->next != NULL)
        {
            data=data->next;
        }
        data->next = output;
    }
}

sch_names **make_sch_names(students *list, uint32 n)
{
    int i=0;
    sch_names **names;
    if(!n)
        return NULL;
    names=(sch_names **)malloc(sizeof(sch_names *)*n);
    while(list)
    {
        names[i] = list->name;
        list=list->next;
        //LOG_INFO("%s \n",names[i]->name);
        i++;
    }
    return names;
}

void free_student_list(students *list)
{
    students *cur;
    while(list)
    {
        cur=list;
        list=list->next;
        free(cur);
    }
}

uint32 count_student_list(students *list)
{
    uint32 c=0;
    while(list)
    {
        list=list->next;
        c++;
    }
    return c;
}

uint32 get_the_company_priority(sch_names *name, uint32 comp_no)
{
    int i;
    for(i=0;i<name->pri.n;i++)
    {
        if(name->pri.pris[i] == comp_no)
        {
            return i;
        }
    }
    //LOG_ERROR("Error: Unable to get the company priority for %s:%d\n",name->name,comp_no);
    return 0xFF;
}

students *move_single_ahead(students *list)
{
    students *head=list, *prev=NULL;
    while(list != NULL)
    {
        if(prev && list->name->pri.n == 1)
        {
            prev->next=list->next;
            list->next=head;
            head=list;
            list=prev->next;
        }
        else
        {
            list=list->next;
        }
    }
    return head;
}

students *make_interview_list_for_companies(sch_data *data, uint32 comp_no)
{
    students *ret=NULL, *cur=NULL;
    int i,j;

//    printf("DEBUG: Making list for %s\n",data->companies[comp_no].name);
    for(j=0;j<MAX_PRIORITIES;j++)
    {
        for(i=0;i<data->comp_pri->n[comp_no];i++)
        {
            sch_names *name;
            name=get_sch_names_from_reg(data->comp_pri->rollno[comp_no][i],data);
            if(name)
            {
                /* Check the priority of company for this guy */
                if(get_the_company_priority(name, comp_no) == j)
                {
//                    printf("\t CP: %d:%s: P:%d\n",i,name->name,j);
                    /* Add to list */
                    if(ret==NULL)
                    {
                        ret=allocate_student();
                        ret->name = name;
                        cur=ret;
                    }
                    else
                    {
                        cur->next = allocate_student();
                        cur->next->name = name;
                        cur=cur->next;
                    }
                }
            }
            else
            {
                LOG_ERROR("Error: interview_list: unable to get name: %d\n",data->comp_pri->rollno[comp_no][i]);
            }
        }
    }

    ret=move_single_ahead(ret);
    return ret;
}

void make_student_list_for_companies(sch_data *data)
{
    uint32 i,j,k;
    sch_comp *comp;
    students *list=NULL, *prev=NULL, *cur=NULL;

    
    for(i=0;i<data->n_companies;i++)
    {
        comp=&data->companies[i];
        if(comp->is_wr_test_complete &&
           comp->is_gd_complete &&
           comp->is_ppt_complete &&
           !comp->is_interview_complete)
        {
            list=make_interview_list_for_companies(data, i);
        }
        else {
        //LOG_INFO("Making list for company: %s\n",comp->name);
        for(k=0;k<MAX_PRIORITIES;k++)
        {
            for(j=0;j<data->n_names;j++)
            {
                uint32 priority = MAX_PRIORITIES-k;
                //uint32 priority = k+1;
                if(data->names[j].pri.n >= (priority))
                {
                    if(data->names[j].pri.pris[(priority-1)] == i)
                    {
                        /* Add to student list */
                        cur = allocate_student();
                        cur->name = &(data->names[j]);
                        if(prev==NULL)
                            list = cur;
                        else
                            prev->next = cur;
                        prev = cur;
                        //LOG_INFO("STUDENT: %d:%s\n",j,cur->name->name);
                    }
                }
            }
        }
        } /* Else -> if !interview */
        if(!comp->is_ppt_complete || !comp->is_gd_complete)
        {
            comp->n_gd = count_student_list(list);
            comp->gd_list = make_sch_names(list, comp->n_gd);
        }
        if(!comp->is_wr_test_complete)
        {
            comp->n_written_test = count_student_list(list);
            comp->written_test = make_sch_names(list, comp->n_written_test);
        }
        if(!comp->is_interview_complete)
        {
            comp->n_interview = count_student_list(list);
            comp->interview_list = make_sch_names(list, comp->n_interview);
        }
        free_student_list(list);
        prev=NULL;
        list=NULL;
        cur=NULL;
    }
}

students *make_student_list(sch_names **names, uint32 n)
{
    int i;
    students *list = NULL, *prev=NULL, *cur=NULL;
    for(i=0;i<n;i++)
    {
        cur=allocate_student();
        cur->name = names[i];
        if(prev == NULL)
            list=cur;
        else
            prev->next = cur;
        prev=cur;
    }
    return list;
}

students *copy_student_list(students *list)
{
    students *head=NULL, *cur=NULL;
    while(list != NULL)
    {
        if(!head)
        {
            head=cur=allocate_student();
        }
        else
        {
            cur->next=allocate_student();
            cur=cur->next;
        }
        cur->name = list->name;
        list=list->next;
    }
    return head;
}

rooms *allocate_rooms(void)
{
    rooms *room;
    room=(rooms *)malloc(sizeof(rooms));
    room->company = NULL;
    room->student_list=NULL;
    room->end_time=0;
    room->room_no=0;
    room->next=NULL;
    return room;
}
output_list *allocate_output(void)
{
    output_list *output;
    output=(output_list *)malloc(sizeof(output_list));
    output->next=NULL;
    output->room_list = NULL;
    output->time=(uint32)-1;
    return output;
}

rooms *copy_room(sch_rooms *room, uint32 room_no)
{
    rooms *ret;
    ret=allocate_rooms();
    ret->room_no=room_no;
    ret->student_list = copy_student_list(room->student_list[room_no]);
    ret->company=room->company[room_no];
    ret->end_time=room->end_time[room_no];
    return ret;
}

rooms *get_room_from_id(rooms *list, uint32 room_no)
{
    while(list)
    {
        if(list->room_no == room_no)
            return list;
    }
    return NULL;
}

uchar is_student_busy(students *list, sch_names *name)
{
    while(list != NULL)
    {
        if(list->name == name)
            return TRUE;
        list = list->next;
    }

    return FALSE;
}

uchar is_company_complete(sch_comp *comp)
{
    if(comp->is_ppt_complete &&
       comp->is_wr_test_complete &&
       comp->is_gd_complete &&
       comp->is_interview_complete
      )
    {
        return TRUE;
    }
    return FALSE;
}

uchar is_company_in_interview(sch_comp *comp)
{
    if(comp->is_ppt_complete &&
       comp->is_wr_test_complete &&
       comp->is_gd_complete &&
       !comp->is_interview_complete)
    {
        return TRUE;
    }
    return FALSE;
}
uint32 get_company_pref(sch_data *data, sch_names *name, sch_comp *comp)
{
    int i;
    for(i=0;i<name->pri.n;i++)
    {
        if(&data->companies[name->pri.pris[i]] == comp)
            return (i+1);
    }
    return 0xFF;
}


static uchar g_swap_flag=FALSE;
void enable_swapping_for_interview(uchar flag)
{
    g_swap_flag=!(!flag); /* To convert it into bin */
}
uchar is_interview_swapping_enabled(void)
{
    return g_swap_flag;
}

uchar is_company_pending(sch_data *data, students *student, uint32 comp_no)
{
    uint32 i;
    for(i=0;i<student->name->pri.n;i++)
    {
        if(student->name->pri.pris[i] == comp_no)
            return TRUE;
    }
    return FALSE;
}

