#include "HackEnrollment.h"







int main(int argc, char *argv[]){
    char *students, *courses, *hackers, *queues, *target;
    if(argc == 5){ // Not case-sensitive
        students = argv[1];
        courses = argv[2];
        hackers = argv[3];
        queues = argv[4];
        target = argv[5];
    } else if (argc == 6) { // Case sensitive
        students = argv[2];
        courses = argv[3];
        hackers = argv[4];
        queues = argv[5];
        target = argv[6];
    }
    // Open files
    FILE *studentsFp = fopen(students, "r");
    FILE *coursesFp = fopen(courses, "r");
    FILE *hackersFp = fopen(hackers, "r");
    FILE *queuesFp = fopen(queues, "r");
    FILE *targetFp = fopen(target, "a");

    EnrollmentSystem sys = createEnrollment(studentsFp, coursesFp, hackersFp);
    readEnrollment(sys, queuesFp);
    hackEnrollment(sys, targetFp);

    return 0;
}