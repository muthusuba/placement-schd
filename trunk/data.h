/** data.h */

#ifndef __DATA_H__
#define __DATA_H__

#define MAX_COMPANIES       500
#define MAX_NAME_SIZE       100
#define MAX_ROOMS            50
#define MAX_PRIORITIES       15
#define INVALID_COMPANY     0xFFFF

#ifndef FALSE
#define FALSE               0
#define TRUE                1
#endif

#define ROOM_GD         0
#define ROOM_INTERVIEW  1
#define ROOM_TEST       2
#define ROOM_PPT        3
#define ROOM_ANY        4
#define ROOM_TEST_PPT   5

typedef unsigned char uchar;
typedef unsigned int uint32;
typedef int int32;

typedef struct SCH_PRI
{
    uint32 n;
    uint32 pris[MAX_COMPANIES]; /* Companies indexed from zero */
}sch_pri;

typedef struct SCH_NAMES
{
    char name[MAX_NAME_SIZE];
    uint32 regno;
    sch_pri pri;
    uint32 end_time;  /* Interview end time */
    uint32 n_interviews; /* No. of interviews attended */
    //struct SCH_NAMES *next;
}sch_names;

typedef struct STUDENTS
{
    sch_names *name;
    struct STUDENTS *next;
}students;

typedef struct SCH_COMP
{
    char name[MAX_NAME_SIZE];
    uint32 n_panels;
    uint32 students_per_panel;
    uint32 start_time;
    uint32 end_time;
    uint32 force;
    
    /* PPT Info */
    uint32 ppt_time;  /* (in min.) PPT Time or Company's 
                         interview time per student */
    uchar is_ppt_complete;
 
    /* Written test info */
    uchar is_wr_test_complete;
    uint32 wr_test_time;
    uint32 n_written_test;
    sch_names **written_test; /* May not be required */
    
    /* GD Info */
    uchar is_gd_complete; /* Avoiding bit-fields */
    uint32 gd_time;
    uint32 n_gd;
    sch_names **gd_list; /* OR ppt_list -> considered same */
    
    /* Interview info */
    uchar is_interview_complete;
    uint32 n_interview;
    sch_names **interview_list;
    uint32 n_complete;

    /* Current processing list */
    students *student_list;
    students *student_list2;
   //struct SCH_COMP *next;
}sch_comp;

typedef struct COMPS
{
    sch_comp *company;
    struct COMPS *next;
}companies;

typedef struct SCH_ROOMS
{
    uint32 n;
    char room[MAX_ROOMS][MAX_NAME_SIZE];
    uint32 capacity[MAX_ROOMS];
    uchar type[MAX_ROOMS]; /* 0->GD, 1->Interview, 2->Test, 3-> PPT, 4->Any 5->PPT_TEST */
    
    uchar in_use[MAX_ROOMS];
    uint32 end_time[MAX_ROOMS];
    uint32 interview_end_time[MAX_ROOMS]; /* Interview end time */
    students *student_list[MAX_ROOMS];
    sch_comp *company[MAX_ROOMS];
}sch_rooms;

typedef struct ROOMS
{
    uint32 room_no;
    uint32 room_type; /* To differentiate when the room_type is ANY */
    uint32 end_time;
    students *student_list;
    sch_comp *company;
    struct ROOMS *next;
}rooms;

/* Company priority list - Used for interview */
typedef struct COMPANY_PRIORITY
{
    uint32 n[MAX_COMPANIES];
    uint32 *rollno[MAX_COMPANIES];
}company_priority;

/* Scheduler Output */
typedef struct OUTPUT_DATA
{
    uint32 time;
    rooms *room_list;
    struct OUTPUT_DATA *next;
}output_list;


typedef struct SCH_DATA
{
    // start_time
    // cur_time ??
    // breaks - 
    // buffer time_gd, interview
    uint32 ppt_percent;
    uint32 wave_time;
    uint32 time;
    uint32 n_names;
    sch_names *names;
    uint32 n_companies;
    sch_comp *companies;
    sch_rooms rooms;
    company_priority *comp_pri;
    output_list *output;
}sch_data;




/* Extern declarations */
extern char *room_names[];


/* Function Declarations */
sch_names *get_sch_names_from_reg(uint32 regno, sch_data *data);
sch_data *allocate_sch_data(void);
sch_comp *allocate_sch_comp(int n);
sch_names *allocate_sch_names(int n);
students *allocate_student(void);
companies *allocate_comp(void);
void add_to_output(output_list **head, output_list *output);
void free_student_list(students *list);

uint32 count_student_list(students *list);
sch_names **make_sch_names(students *list, uint32 n);

void make_student_list_for_companies(sch_data *data);
students *make_student_list(sch_names **names, uint32 n);
rooms *get_room_from_id(rooms *list, uint32 room_no);
rooms *copy_room(sch_rooms *room, uint32 room_no);
output_list *allocate_output(void);
rooms *allocate_rooms(void);
students *copy_student_list(students *list);
uchar is_student_busy(students *list, sch_names *name);
uchar is_company_complete(sch_comp *comp);
uchar is_company_in_interview(sch_comp *comp);
uint32 get_company_pref(sch_data *data, sch_names *name, sch_comp *comp);
uchar is_interview_swapping_enabled(void);
void enable_swapping_for_interview(uchar flag);
uchar is_company_pending(sch_data *data, students *student, uint32 comp_no);
#endif
