#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COURSES 100
#define MAX_SEMESTERS 12
#define COURSE_CODE_LEN 4  // 3位字母数字串 + '\0'

typedef struct Course {
    char code[COURSE_CODE_LEN];
    int credits;
    int prereqCount;
    char prereqs[MAX_COURSES][COURSE_CODE_LEN];
} Course;

typedef struct SemesterPlan {
    char courses[MAX_COURSES][COURSE_CODE_LEN];
    int courseCount;
} SemesterPlan;

SemesterPlan plans[MAX_SEMESTERS]; // 定义一个存储学期计划的数组

void readCourses(Course courses[], int *courseCount) {
    int n;
    printf("输入课程总数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        courses[i].prereqCount = 0;
        printf("输入课程 %d 的课程号(3位字母数字串): ", i + 1);
        scanf("%s", courses[i].code);
        printf("输入课程 %d 的学分: ", i + 1);
        scanf("%d", &courses[i].credits);

        int prereqNum;
        printf("输入课程 %d 的先修课程数: ", i + 1);
        scanf("%d", &prereqNum);

        for (int j = 0; j < prereqNum; j++) {
            printf("输入课程 %d 的先修课程 %d 的课程号: ", i + 1, j + 1);
            scanf("%s", courses[i].prereqs[j]);
        }
        courses[i].prereqCount = prereqNum;
    }
    *courseCount = n;
}

void readSemesterDetails(int *totalSemesters, int *creditsPerSemester) {
    printf("输入学期总数: ");
    scanf("%d", totalSemesters);
    printf("输入每学期的学分上限: ");
    scanf("%d", creditsPerSemester);
}

int isValidPrereq(Course courses[], int courseCount, const char *prereq) {
    for (int i = 0; i < courseCount; i++) {
        if (strcmp(courses[i].code, prereq) == 0) {
            return 1;
        }
    }
    return 0;
}

void validatePrerequisites(Course courses[], int courseCount) {
    for (int i = 0; i < courseCount; i++) {
        for (int j = 0; j < courses[i].prereqCount; j++) {
            if (!isValidPrereq(courses, courseCount, courses[i].prereqs[j])) {
                printf("错误: 课程 %s 的先修课程 %s 不在课程列表中\n", courses[i].code, courses[i].prereqs[j]);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void generateBalancedPlan(Course courses[], int courseCount, int totalSemesters, int creditsPerSemester, SemesterPlan plans[]) {
    int taken[MAX_COURSES] = {0};  // 记录课程是否已安排
    int semestersUsed = 0;

    while (semestersUsed < totalSemesters && !allCoursesScheduled(taken, courseCount)) {
        int creditsThisSemester = 0;
        int courseIndex = -1;

        // 找到可以安排且学分最少的课程
        for (int i = 0; i < courseCount; i++) {
            if (!taken[i] && canTakeCourse(courses, taken, i, totalSemesters, creditsPerSemester)) {
                if (courseIndex == -1 || courses[i].credits < courses[courseIndex].credits) {
                    courseIndex = i;
                }
            }
        }

        if (courseIndex != -1) {
            strcpy(plans[semestersUsed].courses[plans[semestersUsed].courseCount], courses[courseIndex].code);
            plans[semestersUsed].courseCount++;
            taken[courseIndex] = 1;
            creditsThisSemester += courses[courseIndex].credits;
            printf("学期 %d: 课程 %s (%d 学分)\n", semestersUsed + 1, courses[courseIndex].code, creditsThisSemester);
        }

        if (creditsThisSemester == 0) {  // 没有找到可安排的课程
            break;
        }

        semestersUsed++;
    }

    if (!allCoursesScheduled(taken, courseCount)) {
        printf("无法在所有学期内均匀安排所有课程\n");
    }
}

void generateConcentratedPlan(Course courses[], int courseCount, int totalSemesters, int creditsPerSemester, SemesterPlan plans[]) {
    int taken[MAX_COURSES] = {0};  // 记录课程是否已安排
    int creditsThisSemester = 0;

    for (int sem = 0; sem < totalSemesters && !allCoursesScheduled(taken, courseCount); sem++) {
        for (int i = 0; i < courseCount; i++) {
            if (!taken[i] && canTakeCourse(courses, taken, i, totalSemesters, creditsPerSemester)) {
                strcpy(plans[sem].courses[plans[sem].courseCount], courses[i].code);
                plans[sem].courseCount++;
                taken[i] = 1;
                creditsThisSemester += courses[i].credits;

                if (creditsThisSemester > creditsPerSemester) {
                    // 超过学分上限，移到下一个学期
                    plans[sem].courseCount--;
                    taken[i] = 0;
                    creditsThisSemester -= courses[i].credits;
                    break;
                }
            }
        }
    }

    if (!allCoursesScheduled(taken, courseCount)) {
        printf("无法在前几个学期内安排所有课程\n");
    }
}

int canTakeCourse(Course courses[], int taken[], int courseIndex, int totalSemesters, int creditsPerSemester) {
    for (int i = 0; i < courses[courseIndex].prereqCount; i++) {
        int prereqFound = 0;
        for (int j = 0; j < totalSemesters; j++) {
            for (int k = 0; k < MAX_COURSES && plans[j].courseCount > k; k++) {
                if (strcmp(plans[j].courses[k], courses[courseIndex].prereqs[i]) == 0) {
                    prereqFound = 1;
                    break;
                }
            }
            if (prereqFound) {
                break;
            }
        }
        if (!prereqFound) {
            return 0;
        }
    }
    return 1;
}

int allCoursesScheduled(int taken[], int courseCount) {
    for (int i = 0; i < courseCount; i++) {
        if (!taken[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    Course courses[MAX_COURSES];
    int courseCount;
    int totalSemesters, creditsPerSemester;
    SemesterPlan plans[MAX_SEMESTERS];

    readSemesterDetails(&totalSemesters, &creditsPerSemester);
    readCourses(courses, &courseCount);
    validatePrerequisites(courses, courseCount);

    int strategy;
    printf("选择策略: 1. 学习负担均匀 2. 课程集中在前几个学期\n ");
    scanf("%d", &strategy);
    if (strategy == 1) {
    // 学习负担均匀的策略
    distributeCoursesEvenly(courses, courseCount, plans, totalSemesters, creditsPerSemester);
}
    else if (strategy == 2) {
    // 课程集中在前几个学期的策略
    concentrateCoursesEarly(courses, courseCount, plans, totalSemesters, creditsPerSemester);
}
    else {
    printf("无效的策略选择\n");
    return 1;
}

    // 打印学期计划
    for (int i = 0; i < totalSemesters; i++) {
    printf("学期 %d:\n", i + 1);
}

    return 0;
}
