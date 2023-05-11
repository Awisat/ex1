#include "HackEnrollment.h"
#include "IsraeliQueue.h"
#include "stdio.h"
#include "malloc.h"
int IdDiffFunction(void* id1, void* id2);
int AbsoluteValue(int number);
int AsciiDiff(void* str1, void* str2);
int HackersFile(void*, void*);


typedef struct Student_t * Student;
struct Student_t {
    int id;
    unsigned int totalCredits;
    double gpa;
    char *name;
    char *surname;
    char *city;
    char *department;
    bool isHacker;
    int* friends;
    int* rivals;
    int* courses;
    int coursesCount;
};

typedef struct Course_t * Course;
struct Course_t{
  int courseNumber;
  int size;
  IsraeliQueue courseQueue;
};

/*
typedef struct Hacker_t * Hacker;
struct Hacker_t{
    Student hacker;
    int* friendsId;
    int* rivalsId;
};
*/

 struct EnrollmentSystem_t {
    Student* students;
    Course* courses;
    int studentsCount;
    int coursesCount;
};

int IdDiffFunction(void* student1, void* student2){
    int firstId = ((Student) student1)->id;
    int secondId = ((Student) student2)->id;
    int result = firstId - secondId;
    if(result < 0) return (-result);
    return result;
}



// TODO is malloc/re-alloc fail return NULL
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers){
    EnrollmentSystem newEnrollmentSystem = malloc(sizeof (struct EnrollmentSystem_t));
    if(newEnrollmentSystem == NULL){ // to check if malloc failed
        return NULL;
    }
    Student student = malloc(sizeof (struct Student_t));
    int studentsCount = 0;
    while (fscanf(students, "%d %d %lf %s %s %s", &student->id, &student->totalCredits,
                  &student->gpa, student->name, student->surname, student->department) == 6){
        newEnrollmentSystem->students = realloc(newEnrollmentSystem->students,
                                                (studentsCount + 1) * sizeof (struct Student_t));
        student->isHacker = false;
        student->friends = NULL;
        student->rivals = NULL;
        newEnrollmentSystem->students[studentsCount] = student;
        studentsCount++;
    }

    int id, hackerIndex = 0;
    while(fscanf(hackers, "%d", &id) != EOF){
        // Find the index of the hacker in students array
        for(hackerIndex = 0; hackerIndex < studentsCount; hackerIndex++){
            if(newEnrollmentSystem->students[hackerIndex]->id == id){
                // Hacker found
                break;
            }
        }
        Student hacker = newEnrollmentSystem->students[hackerIndex];
        hacker->isHacker = true;
        // Read courses
        int desiredCoursesCount = 0, friendsCount = 0, rivalsCount = 0;
        int desiredCourse, friend, rival;
        while(fscanf(hackers, "%d", &desiredCourse) != '\n'){
            hacker->courses = realloc(hacker->courses, (desiredCoursesCount + 1) * sizeof (int));
            hacker->courses[desiredCoursesCount] = desiredCourse;
            desiredCoursesCount++;
        }
        hacker->coursesCount = desiredCoursesCount;
        // Read friends
        while(fscanf(hackers, "%d", &friend) != '\n'){
            hacker->friends = realloc(hacker->friends, (friendsCount + 1) * sizeof (int));
            hacker->friends[friendsCount] = friend;
            friendsCount++;
        }
        // Read rivals
        while(fscanf(hackers, "%d", &rival) != '\n'){
            hacker->rivals = realloc(hacker->rivals, (rivalsCount + 1) * sizeof (int));
            hacker->rivals[rivalsCount] = rival;
            rivalsCount++;
        }
    }

    free(student);

    Course course = malloc(sizeof (struct Course_t));
    int coursesCount = 0;
    while(fscanf(courses, "%d %d", &course->courseNumber, &course->size) == 2){
        newEnrollmentSystem->courses = realloc(newEnrollmentSystem->courses,
                                               (coursesCount + 1) * sizeof (struct Course_t));
        newEnrollmentSystem->courses[coursesCount] = course;
        coursesCount++;
    }
    newEnrollmentSystem->studentsCount = studentsCount;
    newEnrollmentSystem->coursesCount = coursesCount;
    return newEnrollmentSystem;
}

int CompareFunction(void *obj1, void *obj2) {
    return 0; // TODO implement
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues){
    if(sys == NULL || queues == NULL){
        return NULL;
    }
    int courseNumber, courseIndex, studentIndex;
    while(fscanf(queues, "%d", &courseNumber) != EOF){
        // Find course in courses array
        for(courseIndex = 0; courseIndex < sys->coursesCount; courseIndex++){
            if(courseNumber == sys->courses[courseIndex]->courseNumber){
                break;
            }
        }
        Course course = sys->courses[courseIndex];
        FriendshipFunction function[] = {NULL};
        course->courseQueue = IsraeliQueueCreate(function, CompareFunction, 20, 0);

        int studentId;
        while(fscanf(queues, "%d", &studentId) != '\n'){
            // Find student in students array
            for(studentIndex = 0; studentIndex < sys->studentsCount; studentIndex++){
                if(studentId == sys->students[studentIndex]->id){
                    break;
                }
            }
            Student student = sys->students[studentIndex];
            if(IsraeliQueueEnqueue(course->courseQueue, student) == ISRAELIQUEUE_ALLOC_FAILED){
                IsraeliQueueDestroy(course->courseQueue);
                return NULL;
            }
        }
    }
    return sys;
}

void hackEnrollment(EnrollmentSystem sys, FILE* out){
    // Create friendship functions array
    FriendshipFunction functions[]={IdDiffFunction, AsciiDiff, HackersFile};

    // Add friendship functions to all courses queues
    int courseIndex=0;
    for(courseIndex = 0; courseIndex < sys->coursesCount; courseIndex++){
            IsraeliQueueAddFriendshipMeasure(sys->courses[courseIndex]->courseQueue, IdDiffFunction);
            IsraeliQueueAddFriendshipMeasure(sys->courses[courseIndex]->courseQueue, AsciiDiff);
            IsraeliQueueAddFriendshipMeasure(sys->courses[courseIndex]->courseQueue, HackersFile);
    }
    // Create temp queues for printing
    Course *coursesTemp = malloc(sys->coursesCount * sizeof(struct Course_t));
    for(int i = 0; i < sys->coursesCount; i++){
        FriendshipFunction function[] = {NULL};
        coursesTemp[i]->courseQueue = IsraeliQueueCreate(function, CompareFunction, 0, 0);
    }

    // Go over hackers and enqueue them in the courses they want
    bool printError = false;
    courseIndex = 0;
    int hackerIndex = 0;
    for(hackerIndex = 0; hackerIndex < sys->studentsCount; hackerIndex++){
        int goodPlaces = 0;
        if(sys->students[hackerIndex]->isHacker){
            Student hacker = sys->students[hackerIndex];
            for(courseIndex = 0; courseIndex< hacker->coursesCount; courseIndex++){
                int courseNumber = hacker->courses[courseIndex];
                for(int i=0; i < sys->coursesCount; i++){
                    if(sys->courses[courseIndex]->courseNumber == courseNumber){
                        IsraeliQueueEnqueue(sys->courses[courseIndex]->courseQueue, hacker);
                        // Check how many courses hacker is in good place
                        int counter = 0;
                        Student curr = (Student)(IsraeliQueueDequeue(sys->courses[courseIndex]->courseQueue));
                        while(curr != NULL){
                            if(curr->id == hacker->id && counter < sys->courses[courseIndex]->size) {
                                // Hacker is in place < size
                                goodPlaces++;
                            }
                            IsraeliQueueEnqueue(coursesTemp[courseIndex]->courseQueue, curr);
                        }
                    }
                }
            }
            if(hacker->coursesCount == 1 && goodPlaces < 1){
                printError = true;
                break;
            } else if(hacker->coursesCount > 1 && goodPlaces < 2){
                printError = true;
                break;
            }
        }
    }

    if(printError){
        fprintf(out, "Cannot satisfy constraints for %d\n", sys->students[hackerIndex]->id);
        return;
    }

    for(int i = 0; i < sys->coursesCount; i++) {
        fprintf(out, "%d ");
        Student curr = IsraeliQueueDequeue(coursesTemp[i]->courseQueue);
        while (curr != NULL){
            fprintf(out, "%d ", curr->id);
            curr = IsraeliQueueDequeue(coursesTemp[i]->courseQueue);
        }
        fprintf(out, "\n");
    }

}
