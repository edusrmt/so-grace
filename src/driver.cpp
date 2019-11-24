#include "../include/grace.hpp"
#include "../include/pointer.hpp"

#include <new>

using namespace grace;

int main () {
    // MemoryPool<24> * memory = new MemoryPool<24>(1024);

    GracePointer<int> p;
    
    try {
        p = new int;
    } catch (bad_alloc e) {
        std::cout << "Falha na alocação" << std::endl;
        return 1;
    }

    *p = 88;

    cout << "Valor de p é: " << *p << std::endl;
    int k = *p;
    cout << "k é " << k << std::endl;

    return 0;
}