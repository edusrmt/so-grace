// Allocate and discard objects.

#include <new>
#include "../include/grace.hpp"
using namespace std;
using namespace grace;

int main() {
    try {
        GracePointer<int> p = new int(1);
        p = new int(2);
        p = new int(3);
        p = new int(4);

        GracePointer<int>::collect();
        cout << "*p: " << *p << endl;
    } catch(bad_alloc exc) {
        cout << "Allocation failure!\n";
        return 1;
    }

    return 0;
}