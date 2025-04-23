#include <stdio.h>
#include "common.h"

class ticket { 
public:
    ticket() {
        ID = counter++;
    }

    int id() {
        return ID;
    }

private:
    int ID;
    static int counter;
};


int ticket::counter = 0;

int main() {
    ticket a = ticket();
    ticket b = ticket();

    printf("%d %d\n",a.id(), b.id());

    return 0;
}
