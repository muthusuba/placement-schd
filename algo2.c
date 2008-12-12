/**
 * Algo2.c
 *       Actual scheduling functions.
 *
 * Copyright (C) Muthu Subramanian K 2008 Dec 
 */

#include <stdio.h>
#include <stdlib.h>
#include "sch.h"
#include "data.h"
#include "algo2.h"
#include "sch_output.h"

#define START_TIME 0
#define STEP_TIME  5 /* in min */
#define USE_WAVE_CONCEPT 0
//#define PPT_PERCENT 60


/* Define Debug for debug prints */
#define DEBUG

static uint32 sim_cur_time = 0;

void debug_comps(companies *comps)
{
    while(comps != NULL)
    {
        LOG_INFO("CPri: %s\n",comps->company->name);
        comps = comps->next;
    }
}

void debug_student_list_with_company(sch_data *data, sch_comp *comp, students *list)
{
    while(list)
    {
        //LOG_INFO("\t %s\n",list->name->name);
        LOG_INFO(",%s,%d\n",list->name->name, 
                 get_company_pref(data, list->name, comp));
        list = list->next;
    }
}


void debug_student_list(students *list)
{
    while(list)
    {
        //LOG_INFO("\t %s\n",list->name->name);
        LOG_INFO(",%s\n",list->name->name);
        list = list->next;
    }
}

#if 0
students *make_student_list(sch_names **names, uint32 n)
{
    students *list=NULL, *cur=NULL, *prev=NULL;
    uint i;
    for(i=0;i<n;i++)
    {
        cur=allocate_student();
        if(cur)
        {
            cur->name = names[i];
            cur->next = NULL;
            if(list==NULL)
                list = cur;
            else
                prev->next = cur;
            prev=cur;
        }
    }
    return list;
}
#endif
/* Allocate cur_processing student list */
void allocate_student_list(sch_comp *comp)
{
    if(!comp->is_ppt_complete)
    {
        comp->student_list = make_student_list(comp->gd_list, comp->n_gd);
    }
    else if(!comp->is_wr_test_complete)
    {
        LOG_INFO("Allocating for WR Test: %s\n",comp->name);
        comp->student_list = make_student_list(comp->written_test, comp->n_written_test);
    }
    else if(!comp->is_gd_complete)
    {
        comp->student_list = make_student_list(comp->gd_list, comp->n_gd);
    }
    else if(!comp->is_interview_complete)
    {
        comp->student_list = make_student_list(comp->interview_list, comp->n_interview);
        comp->student_list2=NULL;
        comp->n_complete=0;
    }
}

companies *make_comps_list(sch_data *data, uint32 *comp_pri)
{
    int i, dummy=0;
    companies *comps=NULL, *c=NULL, *next = NULL;
    sch_comp *company;

    //LOG_INFO("Creating List: No. of Companies: %d\n",data->n_companies);
    for(i=0;i<data->n_companies;i++)
    {
        company = &data->companies[comp_pri[i]];
        //LOG_INFO("Processing: %s\n",company->name);
        if(is_company_complete(company))
        {
            //LOG_INFO("Dummy company1: %s\n",company->name);
            dummy++;
            continue;
        }

        /* Allocate the first list */
        allocate_student_list(company);
        if(company->student_list == NULL)
        {
            //LOG_INFO("Dummy company2: %s\n",company->name);
            dummy++;
            continue;
        }
        
        c=allocate_comp();
        c->company = company;
        if(comps == NULL)
        {
            comps=c;
        }
        else
        {
            next->next = c;
        }
        next = c;
    }
    LOG_INFO("No. of dummy companies: %d\n",dummy);
    //debug_comps(comps);

    return comps;
}

uchar is_student_busy_other_rooms(sch_data *data, sch_names *name, uint32 roomno)
{
    int i;
    for(i=0;i<data->rooms.n;i++)
    {
        if(data->rooms.in_use[i] &&
           //data->rooms.company[i]->is_ppt_complete &&
           i != roomno &&
           is_student_busy(data->rooms.student_list[i], name))
        {
            return TRUE;
        }
    }
    return FALSE;
}



/* Use for only currently PPT rooms/companies */
uchar is_student_busy_percent(sch_data *data, uint32 roomno, uint32 percent)
{
#if 0
    students *list=data->rooms.student_list[roomno];
    uint32 total=count_student_list(list);
#else
    uint32 total=data->rooms.company[roomno]->n_gd;
    students *list=make_student_list(data->rooms.company[roomno]->gd_list, total);
#endif
    uint32 count=0;

    while(list != NULL)
    {
        if(is_student_busy_other_rooms(data, list->name, roomno))
            count++;
        list=list->next;
    }
    if((count*100/total) > percent)
    {
        //LOG_INFO("Percent busy is higher: %d\n",(count*100/total));
        return TRUE;
    }
#if 1
    free_student_list(list);
#endif

    return FALSE;
}

uchar is_student_busy_any(sch_data *data, sch_names *name)
{
    int i;
    for(i=0;i<data->rooms.n;i++)
    {
        if(data->rooms.in_use[i] &&
           data->rooms.company[i]->is_ppt_complete &&
           is_student_busy(data->rooms.student_list[i], name))
        {
            return TRUE;
        }
#if 0
        else if(data->rooms.in_use[i] &&
                !data->rooms.company[i]->is_ppt_complete &&
                is_student_busy(data->rooms.student_list[i], name))
        {
            if(is_student_busy_percent(data, i, 100-PPT_PERCENT))
            {
                return TRUE;
            }
        }
#endif
    }
    return FALSE;
}

uint32 panels_available(sch_data *data, companies *comps)
{
    sch_rooms *rooms = &data->rooms;
    int i, c=0;
    for(i=0;i<rooms->n;i++)
    {
        if(rooms->in_use[i])
        {
            if(comps->company == rooms->company[i])
            {
                c++;
            }
        }
    }
    if(c)
    {
        if(!comps->company->is_ppt_complete)
        {
            return FALSE;
        }
        if(!comps->company->is_gd_complete || !comps->company->is_interview_complete)
        {
            if(c >= comps->company->n_panels)
            {
                return FALSE;
            }
            return (comps->company->n_panels-c);
        }
        if(!comps->company->is_wr_test_complete)
        {
            return FALSE;
        }
        return FALSE;
    }
    return comps->company->n_panels;
}


/* 
  Are students available for company 'comps' are not? 
  Also checks if the room_type is valid for that company.
 */
uchar students_available(sch_data *data, companies *comps, uchar room_type)
{
    int i, j;
    int count = 0;
    uchar checked = 0;
    uchar malloced = 0;
    uint32 n=comps->company->n_gd;
    sch_names **names = comps->company->gd_list;
   
    if(comps->company->student_list)
    {
        n = count_student_list(comps->company->student_list);
        names=make_sch_names(comps->company->student_list, n);
        malloced=1;
    }
    else
    {
#ifdef DEBUG
        //LOG_INFO("Warning: Student List is NULL, using gd_list\n");
#endif
        return FALSE;
    }
    for(j=0;j<n;j++)
    {
        checked=0;
        for(i=0;i<data->rooms.n;i++)
        {
            if(data->rooms.in_use[i] &&
               data->rooms.company[i]->is_ppt_complete &&
               is_student_busy(data->rooms.student_list[i], names[j]))
            {
                checked=1;
                break;
            }        
            else if(data->rooms.in_use[i] &&
                    !data->rooms.company[i]->is_ppt_complete &&
                    is_student_busy(data->rooms.student_list[i], names[j]))
            {
                if(is_student_busy_percent(data, i, 100-data->ppt_percent))
                {
                    checked=1;
                    break;
                }
            }
 
        }
        if(checked == 0)
            count++;
    }
    if(malloced)
        free(names);
    
    if(!comps->company->is_ppt_complete)
    {
        if(room_type == ROOM_PPT || room_type == ROOM_ANY || room_type == ROOM_TEST_PPT )
        {
            if((count*100)/n > data->ppt_percent)
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    if(!comps->company->is_wr_test_complete)
    {
        if(room_type == ROOM_TEST || room_type == ROOM_ANY || room_type == ROOM_TEST_PPT)
        {
            if(count == comps->company->n_written_test)
            {
                LOG_INFO("Students available: WR_TEST: %d\n",count);
                return TRUE;
            }
            else if(count > comps->company->n_written_test)
            {
                LOG_ERROR("count > n_wr_test\n");
                return TRUE;
            }
        }
        return FALSE;
    }
    if(!comps->company->is_gd_complete)
    {
        if((room_type == ROOM_GD || room_type == ROOM_ANY))
        {
            if(count >= (comps->company->students_per_panel*panels_available(data,comps)) || 
              (count && (count == count_student_list(comps->company->student_list))))
            {
                LOG_INFO("Students available: GD: %d %s:%d\n",count, comps->company->name,panels_available(data,comps));
                return TRUE;
            }
        }
        return FALSE;
    }
    if(!comps->company->is_interview_complete)
    {
        if(room_type == ROOM_INTERVIEW || room_type == ROOM_ANY)
        {
            if(count == comps->company->n_interview)
            {
                return TRUE;
            }
            else if(count > comps->company->n_interview)
            {
                LOG_ERROR("count > n_interview\n");
                return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

void copy_to_cur_list(sch_comp *comp, uchar room_type)
{

    if(room_type == ROOM_ANY || room_type == ROOM_TEST_PPT)
    {
        if(!comp->is_ppt_complete)
            room_type = ROOM_PPT;
        else if(!comp->is_wr_test_complete)
            room_type = ROOM_TEST;
        else if(!comp->is_gd_complete)
            room_type = ROOM_GD;
        else if(!comp->is_interview_complete)
            room_type = ROOM_INTERVIEW;
        else
            return;
    }
    switch(room_type)
    {
        case ROOM_PPT:
        case ROOM_GD:
            comp->student_list = make_student_list(comp->gd_list, comp->n_gd);
            break;
        case ROOM_TEST:
            comp->student_list = make_student_list(comp->written_test, comp->n_written_test);
            break;
        case ROOM_INTERVIEW:
            comp->student_list=make_student_list(comp->interview_list, comp->n_interview);
            break;
    }
}

uint32 count_pending_companies(sch_data *data, students *student)
{
    int i,c=0;
    sch_comp *comp;
    sch_pri *pri = &student->name->pri;
    
    for(i=0;i<pri->n;i++)
    {
        comp=&data->companies[pri->pris[i]];
        c+=!comp->is_ppt_complete + !comp->is_gd_complete + 
           !comp->is_interview_complete + !comp->is_wr_test_complete;
    }
    //LOG_INFO("DEBUG: Returning count: %d\n",c);
    return c;
}
students *insert_student(sch_data *data, students *head, students *ins)
{
    students *cur=head, *prev=NULL;
    uint32 ins_c = count_pending_companies(data, ins);
    uint32 comp_no=10;
    //uint32 comp_no=0xFF;
    while(cur)
    {
        if(count_pending_companies(data, cur) < ins_c || 
           is_company_pending(data,cur,comp_no))
        {
            ins->next=cur;
            break;
        }
        prev=cur;
        cur = cur->next;
    }
    /* Insert */
    if(prev)
        prev->next=ins;
    else
        head=ins;
    //ins->next=NULL;
    return head;
}

void reallocate_student_list(sch_data *data, companies *comps)
{
    students *student = comps->company->student_list;
    //students *new=student, *cur=NULL;
    students *new=NULL, *cur=NULL;
    uint32 n1, n2;
    if(!comps->company->student_list)
        return;
    n1=count_student_list(comps->company->student_list);
    //student = student->next;
    //new->next = NULL;
    while(student)
    {
        cur=student;
        student=student->next;
        cur->next = NULL;
        /* Insert into new list */
        new=insert_student(data, new, cur);
    }
    n2=count_student_list(new);
    if(n1 != n2)
    {
        LOG_ERROR("Error: New list!=Initial student list->Debug required :%d:%d\n",n1,n2);
    }

    comps->company->student_list=new;
    return;
}
/* 
   Data: Global Data
   comps: Companies priority list 
 */
sch_comp *get_next_forced_company(sch_data *data, companies **comps_parm, companies **allocated_list_parm, uchar room_type)
{
    companies *comps=*comps_parm;
    companies *prev=NULL;
    companies *allocated_list = *allocated_list_parm;
    
    while(comps != NULL)
    {
        uchar checked = 0;
        /* Check start time of the company */
        if(comps->company->start_time <= sim_cur_time && comps->company->force >= 1000)
        {
            if(!comps->company->is_ppt_complete)
            {
                checked = 1;
                if(room_type != ROOM_PPT && room_type != ROOM_ANY && room_type != ROOM_TEST_PPT)
                    goto NEXT_ITR;
            }
            else
            {
                if(!comps->company->is_wr_test_complete)
                {
                    checked = 1;
                    if(room_type != ROOM_TEST && room_type != ROOM_ANY && room_type != ROOM_TEST_PPT)
                        goto NEXT_ITR;
                }
                else if(!comps->company->is_gd_complete)
                {
                    checked = 1;
                    if(room_type != ROOM_GD && room_type != ROOM_ANY)
                        goto NEXT_ITR;
                }
            }
            if(checked && panels_available(data, comps) && 
               students_available(data, comps, room_type))
            {
                break;
            }
        }
NEXT_ITR:
        prev=comps;
        comps = comps->next;
    }
    if(!comps)
        return NULL;

    if(prev==NULL)
    {
        *comps_parm=comps->next;
    }
    else
    {
        prev->next = comps->next;
    }
    comps->next = NULL;

    if(allocated_list == NULL)
    {
        allocated_list = comps;
        *allocated_list_parm = comps;
    }
    else
    {
        while(allocated_list->next != NULL)
        {
            allocated_list=allocated_list->next;
        }
        allocated_list->next = comps;
    }
    /* Reallocate student list */
    reallocate_student_list(data, comps);
    return comps->company;
}


/* 
   Data: Global Data
   comps: Companies priority list 
 */
sch_comp *sch_next_company(sch_data *data, companies **comps_parm, companies **allocated_list_parm, uchar room_type)
{
    companies *comps=*comps_parm;
    companies *prev=NULL;
    companies *allocated_list = *allocated_list_parm;
    
    while(comps != NULL)
    {
        uchar checked = 0;
        /* Check start time of the company */
        if(comps->company->start_time <= sim_cur_time)
        {
            if(!comps->company->is_ppt_complete)
            {
                checked = 1;
                if(room_type != ROOM_PPT && room_type != ROOM_ANY && room_type != ROOM_TEST_PPT)
                    goto NEXT_ITR;
            }
            else
            {
                if(!comps->company->is_wr_test_complete)
                {
                    checked = 1;
                    if(room_type != ROOM_TEST && room_type != ROOM_ANY && room_type != ROOM_TEST_PPT)
                        goto NEXT_ITR;
                }
                else if(!comps->company->is_gd_complete)
                {
                    checked = 1;
                    if(room_type != ROOM_GD && room_type != ROOM_ANY)
                        goto NEXT_ITR;
                }
            }
            if(checked && panels_available(data, comps) && 
               students_available(data, comps, room_type))
            {
                break;
            }
        }
NEXT_ITR:
        prev=comps;
        comps = comps->next;
    }
    if(!comps)
        return NULL;

    if(prev==NULL)
    {
        *comps_parm=comps->next;
    }
    else
    {
        prev->next = comps->next;
    }
    comps->next = NULL;

    if(allocated_list == NULL)
    {
        allocated_list = comps;
        *allocated_list_parm = comps;
    }
    else
    {
        while(allocated_list->next != NULL)
        {
            allocated_list=allocated_list->next;
        }
        allocated_list->next = comps;
    }
    /* Reallocate student list */
    reallocate_student_list(data, comps);
    return comps->company;
}

/* Gets the next company from the allocated list */
sch_comp *get_next_allocated_company(sch_data *data, companies **comps_parm, companies **allocated_list_parm, uchar room_type)
{
    companies *allocated_list = *allocated_list_parm;
    companies *prev = NULL;
    while(allocated_list != NULL)
    {
        /* Check if company has completed all the work */
        if(is_company_complete(allocated_list->company))
        {
            if(prev == NULL)
            {
                *allocated_list_parm = allocated_list->next;
                free(allocated_list);
                allocated_list = *allocated_list_parm;
            }
            else
            {
                prev->next = allocated_list->next;
                free(allocated_list);
                allocated_list = prev->next;
            }
            continue;
        }

        /* Check if panels and students are available */
        if(panels_available(data, allocated_list) && 
           students_available(data, allocated_list, room_type))
        {
            return allocated_list->company;
        }

        prev = allocated_list;
        allocated_list = allocated_list->next;
    }
    return NULL;
}

sch_comp *get_previous_company(sch_data *data, uint32 room_no)
{
    sch_comp *comp = data->rooms.company[room_no];
    companies comps;
    comps.next=NULL;
    comps.company = comp;

    if(!comp || is_company_complete(comp))
        return NULL;

    /* Check if panels and students are available */
    if(panels_available(data, &comps) && 
       students_available(data, &comps, data->rooms.type[room_no]))
    {
        return comp;
    }
    return NULL;
}
companies *remove_company(companies *head, companies *comp)
{
    companies *prev=NULL, *cur=head;
    while(cur && cur != comp)
    {
        prev=cur;
        cur=cur->next; 
    }
    if(!cur)
    {
        LOG_ERROR("Error: Unable to find comp!:%s\n",comp->company->name);
    }
    else
    {
        if(!prev)
            head=cur->next;
        else
            prev->next=cur->next;
        free(cur);
    }
    return head;
}

/* Change company status */
void change_status(sch_comp *comp, uchar room_type)
{
    uchar changed=0;
    if(comp->student_list == NULL)
    {
        if(!comp->is_ppt_complete)
        {
            LOG_INFO("%d: PPT_COMPLETE: %s\n",sim_cur_time, comp->name);
            comp->is_ppt_complete = TRUE;
            changed=1;
        }
        else if(!comp->is_wr_test_complete)
        {
            LOG_INFO("%d: WR_TEST_COMPLETE: %s\n",sim_cur_time, comp->name);
            comp->is_wr_test_complete = TRUE;
            changed=1;
        }
        else if(!comp->is_gd_complete)
        {
            LOG_INFO("%d: GD_COMPLETE: %s\n",sim_cur_time, comp->name);
            comp->is_gd_complete = TRUE;
            changed=1;
        }
//        else if(!comp->is_interview_complete && !comp->student_list2)
        else if(!comp->is_interview_complete)
        {
            //if(comp->n_complete >= comp->n_interview)
            if(1)
            {
                LOG_INFO("%d: INTERVIEW_COMPLETE: %s\n",sim_cur_time, comp->name);
                comp->is_interview_complete = TRUE;
                changed=1;
            }
            if(comp->n_complete > comp->n_interview)
            {
                LOG_INFO("N_COMPLETE is > N_Interview\n");
            }
        }
    }
    if(changed)
        copy_to_cur_list(comp, ROOM_ANY);

    return;
}


sch_comp *get_next_company_interview(sch_data *data, companies **comps_parm, uchar room_type)
{
    companies *comp=*comps_parm;
    if(room_type != ROOM_INTERVIEW)
        return NULL;
    while(comp)
    {
        if(comp->company->student_list==NULL)
            change_status(comp->company, ROOM_INTERVIEW);
        if(is_company_complete(comp->company))
        {
            *comps_parm=remove_company(*comps_parm, comp);
        }
        else
        {
            if(comp->company->student_list && panels_available(data, comp))
                return comp->company;
        }
        comp=comp->next;
    }

    return NULL;
}

/* Gets the next company - tries from the allocated_list first,
   if not from the comps list
 */
sch_comp *get_next_company(sch_data *data, companies **comps_parm, companies **allocated_list_parm, uchar room_type)
{
    sch_comp *comp;
    /* 
       Step1.1: Check for allocated companies - 
                if they can be re-scheduled this slot 
     */
    comp = get_next_allocated_company(data, comps_parm, allocated_list_parm, room_type);
    if(comp)
    {
        return comp;
    }
    comp = sch_next_company(data, comps_parm, allocated_list_parm, room_type);
    
    return comp;
}


/* Free students and company from a room */
void free_students_and_company(sch_rooms *rooms, int n)
{
    students *list, *next;
    list = rooms->student_list[n];
    while(list!=NULL)
    {
        next = list->next;
        free(list);
        list = next;
    }
    rooms->student_list[n] = NULL;
    change_status(rooms->company[n], rooms->type[n]);
    if(is_company_in_interview(rooms->company[n]))
    {
        rooms->company[n] = NULL;
    }
    return;
}

uchar has_completed_interview(sch_data *data, students *student, sch_comp *comp)
{
    output_list *output=data->output;
    rooms *room;
    while(output)
    {
        room=output->room_list;
        while(room)
        {
            if(room->company == comp)
            {
                if(is_student_busy(room->student_list, student->name))
                    return TRUE;
            }
            room=room->next;
        }
        output=output->next;
    }
    return FALSE;
}

uchar has_completed_previous_interviews(sch_data *data, students *student, sch_comp *comp)
{
    uint32 i;
    sch_names *name=student->name;
    for(i=0;i<name->pri.n;i++)
    {
        if(&data->companies[name->pri.pris[i]] == comp)
            break;
        if(!has_completed_interview(data, student, &data->companies[name->pri.pris[i]]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

students *get_next_interview_student_from_list(sch_data *data, students **student_list_parm, uchar first, sch_comp *comp, uchar force)
{
    students *student_list=*student_list_parm;
    students *prev=NULL, *head = student_list;
    while(student_list != NULL)
    {
        /* TODO: Check if student is free */
        if(student_list->name->end_time <= sim_cur_time &&
           (student_list->name->n_interviews == 0 || !first))
        {
#if 1
            if((first && 
               (&data->companies[student_list->name->pri.pris[0]] == comp ||
                (is_interview_swapping_enabled() && 
                &data->companies[student_list->name->pri.pris[1]] == comp)
                ))|| 
               has_completed_previous_interviews(data, student_list, comp) ||
               force)
#endif
            {
                if(head == student_list)
                {
                    *student_list_parm=student_list->next;
                    student_list->next = NULL;
                }
                else
                {
                    prev->next = student_list->next;
                    student_list->next = NULL;
                }
                return student_list;
            }
        }
        prev=student_list;
        student_list=student_list->next;
    }
    return NULL;
}


/* Get next student for interview */
students *get_next_interview_student(sch_data *data, sch_comp *comp)
{
    students *ret=NULL;

    ret=get_next_interview_student_from_list(data, &comp->student_list, 1, comp, 0);

    if(!ret)
        ret=get_next_interview_student_from_list(data, &comp->student_list, 0, comp, 0);
    if(!ret)
        ret=get_next_interview_student_from_list(data, &comp->student_list, 0, comp, comp->force);
    if(ret)
    {
        /* Set students End time */
        ret->name->end_time = sim_cur_time+comp->ppt_time;
//printf("NStudent: %s Start: %d End: %d\n",ret->name->name, sim_cur_time, ret->name->end_time);
        ret->name->n_interviews++;
    }
    return ret;
}

void room_snap_shot(sch_data *data, uint32 roomno, output_list **output_parm, uchar room_type)
{
    output_list *output=*output_parm;
    static output_list *cur_output=NULL;
    //uint32 nroom = data->rooms.n;
    rooms *room;

    if(!cur_output)
    {
        cur_output=*output_parm=output=allocate_output();
        cur_output->time=sim_cur_time;
        /* Updata Data->output also */
        data->output = cur_output;
    }
    else if(cur_output->time != sim_cur_time)
    {
        cur_output->next = allocate_output();
        cur_output=cur_output->next;
        cur_output->time=sim_cur_time;
    }
    
    /* Copy room info */
    room=copy_room(&data->rooms, roomno);
    room->room_type = room_type;

    /* Add to list */
    {
        rooms *head=cur_output->room_list;
        if(!head)
        {
            cur_output->room_list = room;
        }
        else
        {
            while(head->next)
            {
                head=head->next;
            }
            head->next=room;
        }
    }
#ifdef DEBUG
{
    sch_rooms *rooms=&data->rooms;
    uint32 cur_time = sim_cur_time;
    uint32 t;
    t = cur_time/60;
    cur_time = t*100;
    cur_time += (sim_cur_time-t*60);
    cur_time += 700;
    if(cur_time >= 2400)
        cur_time -= 2400;

//    LOG_INFO("%d: Allocating:%s: %s for %s\n",sim_cur_time, room_names[rooms->type[roomno]], rooms->room[roomno], comp->name);
    LOG_INFO("%s,%d,%04d,%s,%s\n",rooms->company[roomno]->name, 
                                count_student_list(rooms->student_list[roomno]),
                                cur_time, 
                                room_names[room_type], 
                                rooms->room[roomno]);
    debug_student_list_with_company(data, rooms->company[roomno], rooms->student_list[roomno]);
}
#endif
}



uchar check_n_free_room(sch_data *data, sch_rooms *rooms, int n, output_list **output)
{
    uchar freed=0;

    if(rooms->end_time[n] <= sim_cur_time && rooms->in_use[n] && rooms->type[n] != ROOM_INTERVIEW) /* TODO */
    {
        /* Free students and company panels */
        free_students_and_company(rooms, n);

        /* Free the room */
        rooms->in_use[n] = FALSE;
        freed=1;
    }
    else if(rooms->type[n] == ROOM_INTERVIEW && rooms->in_use[n])
    {
        if(rooms->student_list[n] != NULL &&
           rooms->student_list[n]->name->end_time <= sim_cur_time)
        {
            /* TODO Check buffer time */
            rooms->student_list[n]->name->end_time = sim_cur_time + 10;
            free(rooms->student_list[n]);
            rooms->student_list[n] = NULL;
        }
        if(rooms->student_list[n] == NULL && rooms->company[n]->student_list == NULL)
        {
            free_students_and_company(rooms, n);
            rooms->in_use[n]=FALSE;
            freed=1;
        }
    }
    return freed;
}


int32 comp_priority(sch_data *data, sch_comp *comp, sch_names *student)
{
    uint32 i;
    for(i=0;i<student->pri.n;i++)
    {
        if(&data->companies[student->pri.pris[i]] == comp)
        {
            return i;
        }
    }
    LOG_INFO("Error: Unable to find company!!! For priority: %s:%s",comp->name,student->name);
    return -1;
}
#if 0
students *make_next_interview_list(sch_data *data, sch_comp *comp)
{
    uint32 i, n;
    students *head=NULL, *cur=NULL;
    uint32 p_priority= (uint32)-1;

    if(comp->n_complete >= comp->n_interview)
    {
        LOG_INFO("ERROR: N_COMPLETE=N_INTERVIEW: Trying to get more students");
        return NULL;
    }
    for(i=comp->n_complete;i<n;i++)
    {
        if(comp_priority(data, comp,comp->interview_list[i]) > interview_current_priority+1)
        {
            break;
        }
        if(p_priority == (uint32)-1)
        {
            p_priority = comp_priority(data, comp, comp->interview_list[i]);
        }
        else if(comp_priority(data, comp, comp->interview_list[i]) != p_priority)
        {
            break;
        }

        /* Add student to list */
        if(head==NULL)
        {
            head=allocate_student();
            head->name = comp->interview_list[i];
            cur=head;
        }
        else
        {
            cur->next = allocate_student();
            cur->next->name = comp->interview_list[i];
            cur = cur->next;
        }
    }
    return head;
}
//#else
students *make_next_interview_list(sch_data *data, sch_comp *comp)
{
    uint32 i, n;
    students *head=NULL, *cur=NULL;
    uint32 p_priority= (uint32)-1;

    if(comp->n_complete >= comp->n_interview)
    {
        LOG_INFO("ERROR: N_COMPLETE=N_INTERVIEW: Trying to get more students");
        return NULL;
    }
    for(i=comp->n_complete;i<n;i++)
    {
        if(comp_priority(data, comp,comp->interview_list[i]) > interview_current_priority+1)
        {
            break;
        }
        if(p_priority == (uint32)-1)
        {
            p_priority = comp_priority(data, comp, comp->interview_list[i]);
        }
        else if(comp_priority(data, comp, comp->interview_list[i]) != p_priority)
        {
            break;
        }

        /* Add student to list */
        if(head==NULL)
        {
            head=allocate_student();
            head->name = comp->interview_list[i];
            cur=head;
        }
        else
        {
            cur->next = allocate_student();
            cur->next->name = comp->interview_list[i];
            cur = cur->next;
        }
    }
    return head;
}

#endif

/* Allocate room */
void allocate_room(sch_data *data, uint32 roomno, sch_comp *comp, output_list **output)
{
    uint32 i;
    students *prev=NULL, *head=NULL, *cur=NULL, *rcur=NULL;
    sch_rooms *rooms = &data->rooms;
    uchar room_type=0;

    /* Allocate Room */
    rooms->in_use[roomno] = TRUE;
    rooms->company[roomno] = comp;
    rooms->student_list[roomno] = NULL;

    /* Allocate Students to this room */
    if(!comp->is_ppt_complete || !comp->is_wr_test_complete)
    {
        /* Allocate all students */
        rooms->student_list[roomno] = comp->student_list;
        comp->student_list = NULL;

        if(!comp->is_ppt_complete)
        {
            rooms->end_time[roomno] = sim_cur_time + comp->ppt_time;
            room_type = ROOM_PPT;
        }
        else
        {
            rooms->end_time[roomno] = sim_cur_time + comp->wr_test_time;
            room_type = ROOM_TEST;
        }
    }
    else if(!comp->is_gd_complete)
    {
        rooms->end_time[roomno] = sim_cur_time + comp->gd_time;
        prev=NULL;
        head=comp->student_list;
        cur=head;
        for(i=0;i<comp->students_per_panel && cur;i++)
        {
            //if(cur == NULL)
            //    break;
            if(!is_student_busy_any(data,cur->name))
            {
                /* Move student */
                if(cur==head)
                {
                    head = cur->next;
                }
                else
                {
                    prev->next = cur->next;
                }
                if(rooms->student_list[roomno] == NULL)
                {
                    rooms->student_list[roomno]=rcur=cur;
                }
                else
                {
                    rcur->next=cur;
                    rcur=rcur->next;
                }
                cur=cur->next;
                rcur->next=NULL;

            }
            else
            {
                prev=cur;
                cur=cur->next;
                i--;
            }
        }
        /* Set the headers */
        comp->student_list = head;

        room_type = ROOM_GD;
    }
    else if(!comp->is_interview_complete)
    {
        rooms->end_time[roomno] = sim_cur_time + data->wave_time;
        room_type = ROOM_INTERVIEW;
 
        /* TODO */
        /* Allocate a student here */
        rooms->student_list[roomno] = get_next_interview_student(data, comp);
    }

#if 0//def DEBUG
{
    uint32 cur_time = sim_cur_time;
    uint32 t;
    t = cur_time/60;
    cur_time = t*100;
    cur_time += (sim_cur_time-t*60);
    cur_time += 700;
    if(cur_time >= 2400)
        cur_time -= 2400;

//    LOG_INFO("%d: Allocating:%s: %s for %s\n",sim_cur_time, room_names[rooms->type[roomno]], rooms->room[roomno], comp->name);
    LOG_INFO("%s,%d,%04d,%s,%s\n",comp->name, 
                                count_student_list(rooms->student_list[roomno]),
                                cur_time, 
                                room_names[room_type], 
                                rooms->room[roomno]);
    debug_student_list(rooms->student_list[roomno]);
}
#endif

    /* Record Changes */
    room_snap_shot(data, roomno, output, room_type);
}

void print_all_data(sch_data *data)
{
    int i;
    LOG_INFO("Pending companies:\n");
    for(i=0;i<data->n_companies;i++)
    {
        if(!is_company_complete(&data->companies[i]))
        {
            LOG_INFO("Company: %s, Flags: %d %d %d %d\n",
                      data->companies[i].name, 
                      data->companies[i].is_ppt_complete,
                      data->companies[i].is_wr_test_complete,
                      data->companies[i].is_gd_complete,
                      data->companies[i].is_interview_complete);
            debug_student_list(data->companies[i].student_list);
        }
    }
}

/* Allocate using simulation */
/* Note the order of rooms to be entered - gd, test and ppt */
output_list *allocate_students_any(sch_data *data, uint32 *comp_pri)
{
    int i;
    //static int prev_room=-1;
    int j;
    //uint32 time_slot = START_TIME;
    companies *comps=NULL;
    companies *allocated_list=NULL;
    sch_comp *cur_comp;
    sch_rooms *rooms = &data->rooms;
    output_list *output=NULL;

    /* Set the initial start time */
    sim_cur_time = data->time;
    /* Make linked list for companies */
    comps = make_comps_list(data,comp_pri);


    while(comps!=NULL || allocated_list !=NULL)
    {
#ifdef DEBUG
        uchar allocated_room = 0;
#endif
        //LOG_INFO("Simulating time: %d\n",sim_cur_time);
        /* Step0: Check if rooms need to be freed */
        for(j=0;j<rooms->n;j++)
        {
            check_n_free_room(data, rooms, j, &output);
        }

        /* Try allocating same rooms */
        for(i=0;i<rooms->n;i++)
        {
            j=rooms->n-i-1;
            if(!rooms->in_use[j])
            {
                /* Step1.1: Check if previous company can be allocated */
                cur_comp=NULL;
                cur_comp = get_previous_company(data, j);
                if(cur_comp)
                {
                    /* Allocate Room */
                    allocate_room(data, j, cur_comp, &output);
                }
                else
                {
                    rooms->company[j]=NULL;
                    /* Try the forced companies */
                cur_comp = get_next_forced_company(data, &comps, &allocated_list, rooms->type[j]);
                    if(cur_comp)
                        allocate_room(data, j, cur_comp, &output);
                }
            }
            else if(rooms->student_list[j] == NULL)
            {
                sch_comp *comp=rooms->company[j];
                if(is_company_in_interview(comp))
                {
                    /* TODO: check if a student can be allocated now */
                    rooms->student_list[j] = get_next_interview_student(data, rooms->company[j]);
                    if(rooms->student_list[j])
                    {
 #ifdef DEBUG
                        allocated_room = 1;
#endif
                        room_snap_shot(data, j, &output, ROOM_INTERVIEW);
                    }
                }
            }
        }
        //i=prev_room;
        i=-1;
        /* Allocate rooms */
        for(j=0;j<rooms->n;j++)
        {
            i++;
            if(i>=rooms->n)
                i=0;
 
            if(!rooms->in_use[i])
            {
                /* Step1.2: Get the company */
                cur_comp = get_next_company(data, &comps, &allocated_list, rooms->type[i]);
                if(!cur_comp)
                    cur_comp = get_next_company_interview(data, &comps, rooms->type[i]);
                if(cur_comp == NULL)
                {
                    /* Unable to schedule anything right now */
                    //LOG_INFO("Uable to schedule any company for room: %d time: %d\n", i, sim_cur_time);
                    continue;
                }
#ifdef DEBUG
                allocated_room = 1;
#endif
                /* Allocate Room */
                allocate_room(data, i, cur_comp, &output);
                #if 0
                prev_room=i;
                #endif
            }
       }

        /* Updated Simulation Time */
        sim_cur_time += STEP_TIME;
#ifdef DEBUG
        {
            static uint32 old_allocated_time=0;
            static uchar debuged=0;
            if(!allocated_room)
            {
                if(!debuged && (sim_cur_time - old_allocated_time) > 60*5) /* 5 hrs */
                {
                    debuged=1;
                    LOG_INFO("Simulation stuck!!! Debug: Time: %d\n",sim_cur_time);
                    print_all_data(data);
                    process_output(data, output);
                }
            }
            else
            {
                old_allocated_time = sim_cur_time;
                debuged=0;
            }
        }
#endif
    }

    //free(comps);
    return output;
}
