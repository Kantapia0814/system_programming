#include <stdio.h>
#include <stdlib.h>

struct Rectangle {
    int width;
    int height;
};

struct Student {
    char name[20];
    int score;
};

int area(struct Rectangle *r) {
    return r->width * r->height;
}

int main() {
    struct Rectangle r1 = {5, 8};
    printf("%d\n", area(&r1));

    struct Student sd[3] = {{"Amy", 90}, {"Bob", 80}, {"Chris", 70}};
    double avg = (sd[0].score + sd[1].score + sd[2].score) / 3;
    printf("Average score: %.2f\n", avg);
}