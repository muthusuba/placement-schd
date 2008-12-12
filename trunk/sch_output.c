/**
 * sch_output.c
 *     Output generation.
 * Copyright(C) K.Muthu Subramanian 2008 Dec
 */

#include <stdio.h>
#include <stdlib.h>
#include "sch.h"
#include "data.h"
#include "sch_output.h"

#define output_print printf

void output_headers_student(output_list *output)
{
    output_print("\n");
    output_print("\"Student Name\"");
    while(output)
    {
        uint32 cur_time = output->time;
        uint32 t;
        t = cur_time/60;
        cur_time = t*100;
        cur_time += (output->time-t*60);
        cur_time += 700;
        if(cur_time >= 2400)
            cur_time -= 2400;
        
        output_print(",%04d",cur_time);
        
        output=output->next;
    }
    output_print("\n");
}

void output_headers(char *header, output_list *output)
{
    output_print("\n");
    output_print("\"%s Name\"",header);
    while(output)
    {
        uint32 cur_time = output->time;
        uint32 t;
        t = cur_time/60;
        cur_time = t*100;
        cur_time += (output->time-t*60);
        cur_time += 700;
        if(cur_time >= 2400)
            cur_time -= 2400;
        
        output_print(",%04d",cur_time);
        
        output=output->next;
    }
    output_print("\n");
}
rooms *find_room_from_name(sch_names *name, rooms *room)
{
    while(room)
    {
        if(is_student_busy(room->student_list, name))
        {
            if(room->room_type == ROOM_PPT)
            {
                rooms *t;
                t=find_room_from_name(name, room->next);
                if(t)
                    return t;
            }
            return room;
        }
        room=room->next;
    }
    return NULL;
}
void process_student_wise(sch_data *data, output_list *output)
{
    int i;
    rooms *room;
    output_list *head=output;

    output_headers_student(output);
    for(i=0;i<data->n_names;i++)
    {
        output_print("\"%s\"",data->names[i].name);
        output=head;
        while(output)
        {
            room=find_room_from_name(&data->names[i], output->room_list);
            if(room)
            {
                output_print(",\"%s_%s_%s_%d\"", 
                             room->company->name,
                             data->rooms.room[room->room_no],
                             room_names[room->room_type],
                             get_company_pref(data, &data->names[i], room->company));

            }
            else
            {
                output_print(",");
            }
            output=output->next;
        }
        output_print("\n");
    }
}
void process_room_wise(sch_data *data, output_list *output)
{
    int i;
    output_list *head=output;

    output_headers("Room",output);
    for(i=0;i<data->rooms.n;i++)
    {
        output_print("\"%s\",",data->rooms.room[i]);
        output=head;
        while(output)
        {
            rooms *room=output->room_list;
            while(room)
            {
                if(room->room_no == i)
                {
                    output_print("%s:%s ",room->company->name,
                                         room_names[room->room_type]);
                    break;
                }
                room=room->next;
            }
            output_print(","); 
            output=output->next;
        }
        output_print("\n");
    }
}

void process_company_wise(sch_data *data, output_list *output)
{
    int i;
    output_list *head=output;

    output_headers("Company",output);
    for(i=0;i<data->n_companies;i++)
    {
        output_print("\"%s\",",data->companies[i].name);
        output=head;
        while(output)
        {
            rooms *room=output->room_list;
            while(room)
            {
                if(room->company == &data->companies[i])
                {
                    output_print("%s:%s ",data->rooms.room[room->room_no],
                                         room_names[room->room_type]);
                }
                room=room->next;
            }
            output_print(","); 
            output=output->next;
        }
        output_print("\n");
    }
}

void process_output(sch_data *data, output_list *output)
{
    process_student_wise(data, output);
    process_company_wise(data, output);
    process_room_wise(data, output);
}
