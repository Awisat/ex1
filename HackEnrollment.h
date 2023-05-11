#ifndef HACKENROLLMENT_H
#define HACKENROLLMENT_H

#include "stdio.h"

typedef struct EnrollmentSystem_t * EnrollmentSystem;

/**@param FILE* students:
 * @param FILE* courses:
 * @param FILE* hackers:
 *
 * HERE .*/
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

void hackEnrollment(EnrollmentSystem sys, FILE* out);


























#endif //EX1_HACKENROLLMENT_H
