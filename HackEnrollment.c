#include <string.h>
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
    int gpa;
    char *name;
    char *surname;
    char *city;
    char *department;
    bool isHacker;
    int* friends;
    int* rivals;
    int* courses;
    int coursesCount;
    int numFriends;
    int numRivals;
};

typedef struct Course_t * Course;
struct Course_t{
  int courseNumber;
  int size;
  IsraeliQueue courseQueue;
};


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

int HackersFile(void* student1, void* student2){
    if(((Student)student1)->isHacker) {
        // Check if friends
        for (int i = 0; i < ((Student) student1)->numFriends; i++) {
            if (((Student) student1)->friends[i] == ((Student) student2)->id) {
                return 20;
            }
        }
        // Check if rivals
        for (int i = 0; i < ((Student) student1)->numRivals; i++) {
            if (((Student) student1)->rivals[i] == ((Student) student2)->id) {
                return -20;
            }
        }
    }
    if(((Student)student2)->isHacker) {
        // Check if friends
        for (int i = 0; i < ((Student) student2)->numFriends; i++) {
            if (((Student) student2)->friends[i] == ((Student) student1)->id) {
                return 20;
            }
        }
        // Check if rivals
        for (int i = 0; i < ((Student) student2)->numRivals; i++) {
            if (((Student) student2)->rivals[i] == ((Student) student1)->id) {
                return -20;
            }
        }
    }
    return 0;
}

int AsciiDiff(void* student1, void* student2){
    int distance = 0;
    char* s1 = ((Student) (student1))->name;
    char* s2 = ((Student) (student2))->name;
    int length1 = strlen(s1);
    int length2 = strlen(s2);
    int minLength = length1 < length2 ? length1: length2;
    for (int i = 0; i < minLength; i++) {
        distance += abs(s1[i] - s2[i]);
    }
    if (length1 > length2) {
        for (int i = minLength; i < length1; i++) {
            distance += s1[i];
        }
    } else {
        for (int i = minLength; i < length2; i++) {
            distance += s2[i];
        }
    }
    return distance;
}

int CompareFunction(void *obj1, void *obj2) {
    return 0; // TODO implement
}

// TODO is malloc/re-alloc fail return NULL
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers){
    // Find the longest line in students
    int longestLine = 0, currentLine = 0;
    char c;
    while((c = fgetc(students)) != EOF){
        currentLine++;
        if(c == '\n'){
            if(currentLine > longestLine){
                longestLine = currentLine;
            }
            currentLine =0;
        }
    }
    rewind(students);

    // Buffers for reading students
    char* buffName = malloc(longestLine);
    char* buffSurname = malloc(longestLine);
    char* buffCity = malloc(longestLine);
    char* buffDepartment = malloc(longestLine);

    EnrollmentSystem newEnrollmentSystem = malloc(sizeof (struct EnrollmentSystem_t));
    if(newEnrollmentSystem == NULL || buffName == NULL || buffSurname == NULL ||
        buffCity == NULL || buffDepartment == NULL){ // to check if malloc failed
        return NULL;
    }

    // Read students
    int studentsCount = 0;
    newEnrollmentSystem->students = malloc(sizeof (struct Student_t));

    int id, totalCredits;
    int gpa;
    while (true){
        int res = fscanf(students, "%d %d %d %s %s %s %s\n", &id, &totalCredits, &gpa, buffName, buffSurname, buffCity, buffDepartment);
        if(res != 7) {
            break;
        }

        newEnrollmentSystem->students = realloc(newEnrollmentSystem->students,
                                                (studentsCount + 1) * sizeof (struct Student_t));
        Student  student = malloc(sizeof (struct Student_t));
        newEnrollmentSystem->students[studentsCount] = student;

        student->id = id;
        student->totalCredits = totalCredits;
        student->gpa = gpa;

        student->isHacker = false;
        student->friends = NULL;
        student->rivals = NULL;
        student->numFriends = 0;
        student->numRivals = 0;

        student->name = malloc(strlen(buffName) + 1);
        student->surname = malloc(strlen(buffSurname) + 1);
        student->city = malloc(strlen(buffCity) + 1);
        student->department = malloc(strlen(buffDepartment) + 1);

        if(student->name == NULL || student->surname == NULL || student->city == NULL || student->department == NULL){
            return NULL;
        }

        strcpy(student->name, buffName);
        strcpy(student->surname, buffSurname);
        strcpy(student->city,buffCity);
        strcpy(student->department,buffDepartment);

        studentsCount++;
    }

    free(buffName);
    free(buffSurname);
    free(buffCity);
    free(buffDepartment);

    int hackerIndex = 0;
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
        while(fscanf(hackers, "%d", &desiredCourse) == 1){
            hacker->courses = realloc(hacker->courses, (desiredCoursesCount + 1) * sizeof (int));
            hacker->courses[desiredCoursesCount] = desiredCourse;
            desiredCoursesCount++;
            if(fgetc(hackers) == '\n'){
                break;
            }
        }
        hacker->coursesCount = desiredCoursesCount;
        // Read friends
        while(fscanf(hackers, "%d", &friend) == 1){
            hacker->friends = realloc(hacker->friends, (friendsCount + 1) * sizeof (int));
            hacker->friends[friendsCount] = friend;
            friendsCount++;
            if(fgetc(hackers) == '\n'){
                break;
            }
        }
        hacker->numFriends = friendsCount;
        // Read rivals
        while(fscanf(hackers, "%d", &rival) == 1){
            hacker->rivals = realloc(hacker->rivals, (rivalsCount + 1) * sizeof (int));
            hacker->rivals[rivalsCount] = rival;
            rivalsCount++;
            if(fgetc(hackers) == '\n'){
                break;
            }
        }
        hacker->numRivals = rivalsCount;
    }

    newEnrollmentSystem->courses = malloc(sizeof (struct Course_t));
    int coursesCount = 0, courseNumber, courseSize;
    while(fscanf(courses, "%d %d", &courseNumber, &courseSize) == 2){
        newEnrollmentSystem->courses = realloc(newEnrollmentSystem->courses,
                                               (coursesCount + 1) * sizeof (struct Course_t));
        Course course = malloc(sizeof (struct Course_t));
        course->courseNumber = courseNumber;
        course->size = courseSize;
        FriendshipFunction function[] = {NULL};
        course->courseQueue = IsraeliQueueCreate(function, CompareFunction, 20, 0);
        newEnrollmentSystem->courses[coursesCount] = course;
        coursesCount++;
    }
    newEnrollmentSystem->studentsCount = studentsCount;
    newEnrollmentSystem->coursesCount = coursesCount;
    return newEnrollmentSystem;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues){
    if(sys == NULL || queues == NULL){
        return NULL;
    }
    int courseNumber, courseIndex, studentIndex;
    while(fscanf(queues, "%d", &courseNumber) == 1){
        // Find course in courses array
        for(courseIndex = 0; courseIndex < sys->coursesCount; courseIndex++){
            if(courseNumber == sys->courses[courseIndex]->courseNumber){
                break;
            }
        }
        Course course = sys->courses[courseIndex];

        int studentId;
        while(fscanf(queues, "%d", &studentId) == 1){
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
            if(fgetc(queues) == '\n'){
                break;
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
    Course *coursesTemp = malloc(sys->coursesCount * sizeof(Course));

    for(int i = 0; i < sys->coursesCount; i++){
        coursesTemp[i] = malloc(sizeof (struct Course_t));
        printf("course count: %d\n", sys->coursesCount);
        printf("temp queue created\n");
        FriendshipFunction function[] = {NULL};
        printf("before creating queue: %d\n",i);
        IsraeliQueue queue = IsraeliQueueCreate(function, CompareFunction, 0, 0);
        coursesTemp[i]->courseQueue = queue;
        printf("after creating queue:%d\n",i);
    }

    // Go over hackers and enqueue them in the courses they want
    bool printError = false;
    courseIndex = 0;
    int hackerIndex = 0;
    for(hackerIndex = 0; hackerIndex < sys->studentsCount; hackerIndex++){
        int goodPlaces = 0;
        if(sys->students[hackerIndex]->isHacker){
            printf("FOUND HACKER %d\n", sys->students[hackerIndex]->id);
            Student hacker = sys->students[hackerIndex];
            for(courseIndex = 0; courseIndex < hacker->coursesCount; courseIndex++){
                int courseNumber = hacker->courses[courseIndex];
                for(int i=0; i < sys->coursesCount; i++){
                    if(sys->courses[i]->courseNumber == courseNumber){
                        IsraeliQueueEnqueue(sys->courses[i]->courseQueue, hacker);
                        printf("HACKER ENQUEUED %d in course %d\n", hacker->id, sys->courses[i]->courseNumber);
                        // Check how many courses hacker is in good place
                        int counter = 0;
                        Student curr;
                        while((curr = (Student)(IsraeliQueueDequeue(sys->courses[i]->courseQueue))) && curr != NULL){
                            counter++;
                            if(curr->id == hacker->id && counter < sys->courses[i]->size) {
                                // Hacker is in place < size
                                goodPlaces++;
                            }
                            IsraeliQueueEnqueue(coursesTemp[i]->courseQueue, curr);
                        }
                    }
                }
            }
            printf("good %d", goodPlaces);
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
        if(IsraeliQueueSize(coursesTemp[i]->courseQueue) != 0) {
            fprintf(out, "%d ", sys->courses[i]->courseNumber);
            Student curr = IsraeliQueueDequeue(coursesTemp[i]->courseQueue);
            while (curr != NULL) {
                fprintf(out, "%d ", curr->id);
                curr = IsraeliQueueDequeue(coursesTemp[i]->courseQueue);
            }
            fprintf(out, "\n");
        }
    }

}
