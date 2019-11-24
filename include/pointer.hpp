#ifndef POINTER
#define POINTER

#define DEBUG

using namespace std;

#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>

namespace grace {
    class OutOfRangeException { };

    template<class T>
    class Iter {
        T *pointer;             // Valor atual do ponteiro
        T *end;             // Aponta para um elemento após o fim
        T *begin;           // Aponta para o começo da array
        unsigned length;    // Tamanho da sequência

        public:
        // Construtor padrão
        Iter() : pointer{nullptr}, end{nullptr}, begin{nullptr}, length{0} { }

        // Construtor com definições
        Iter(T *ptr, T *first, T *last) : pointer{ptr}, end{last}, begin{first}, length{last - first} { }

        unsigned size() { return length; }

        T &operator* () {
            if (pointer >= end || pointer < begin)
                throw OutOfRangeException();

            return *pointer;
        }

        T *operator-> () {
            if (pointer >= end || pointer < begin)
                throw OutOfRangeException();

            return pointer;
        }

        Iter operator++ () {
            pointer++;
            return *this;
        }

        Iter operator-- () {
            pointer--;
            return *this;
        }

        Iter operator++ (int notUsed) {
            T *tmp = pointer;
            pointer++;
            return Iter<T>(tmp, begin, end);
        }

        Iter operator-- (int notUsed) {
            T *tmp = pointer;
            pointer--;
            return Iter<T>(tmp, begin, end);
        }

        T &operator[] (int i) {
            if (i < 0 || i >= end - begin)
                throw OutOfRangeException();

            return pointer[i];
        }

        bool operator== (Iter rhs) {
            return pointer == rhs.pointer;
        }

        bool operator!= (Iter rhs) {
            return pointer != rhs.pointer;
        }

        bool operator< (Iter rhs) {
            return pointer < rhs.pointer;
        }

        bool operator<= (Iter rhs) {
            return pointer <= rhs.pointer;
        }

        bool operator> (Iter rhs) {
            return pointer > rhs.pointer;
        }

        bool operator>= (Iter rhs) {
            return pointer >= rhs.pointer;
        }

        Iter operator- (int n) {
            pointer -= n;
            return *this;
        }

        Iter operator+ (int n) {
            pointer += n;
            return *this;
        }

        int operator- (Iter<T> &rhs) {
            return pointer - rhs.pointer;
        }
    };

    template<class T>
    class GraceInfo {
        public:
        unsigned refCount;      // Quantidade de referências atualmente
        T *memPointer;          // Ponteiro para a memória alocada
        bool isArray;           // Está apontando para uma array?
        unsigned arraySize;     // Se sim, o tamanho da array

        // Construtor básico, recebe um ponteiro e um tamanho
        GraceInfo (T *mPointer, unsigned size = 0) : refCount{1}, memPointer{mPointer}, isArray{size != 0}, arraySize{size} { }

    };

    // Sobrecarregando o operador == para permitir a comparação de GraceInfo.
    template<class T>
    bool operator==(const GraceInfo<T> &lhs, const GraceInfo<T> &rhs) {
        return lhs.memPointer == rhs.memPointer;
    }

    template<class T, int SIZE=0>
    class GracePointer {
        static list<GraceInfo<T>> garbageList;      // Mantém a lista de lixos
        T *address;                                 // Memória alocada apontada atualmente pelo GracePointer;
        bool isArray;                               // Está apontando para uma array?
        unsigned arraySize;                         // Se sim, o tamanho da array
        static bool first;                          // Primeiro GracePointer foi criado?

        // Encontra um ponteiro em garbageList
        typename list<GraceInfo<T>>::iterator findPointerInfo (T *ptr) {
            typename list<GraceInfo<T>>::iterator p;            

            for (p = garbageList.begin(); p != garbageList.end(); p++) {
                if (p->memPointer == ptr)
                    return p;
            }

            return p;
        }

        public:
        typedef Iter<T> GraceIterator;      // Define o tipo do iterador para GracePointer<T>

        GracePointer (T *t = nullptr) {
            if (first) atexit(shutdown);
            first = false;

            typename list<GraceInfo<T>>::iterator p = findPointerInfo(t);

            // Se p já está na lista, incremente um referência no contador. Se não, adicione-o na lista.
            if (p != garbageList.end())
                p->refCount++;
            else {
                GraceInfo<T> garbageObject(t, SIZE);
                garbageList.push_front(garbageObject);
            }

            address = t;
            arraySize = SIZE;
            isArray = SIZE > 0;

            #ifdef DEBUG
                cout << "Construindo GracePointer.";
                if (isArray)
                    cout << " Tamanho é " << arraySize << endl;
                else
                    cout << endl;
            #endif
        }

        // Construtor de cópia
        GracePointer (const GracePointer &other) {
            typename list<GraceInfo<T>>::iterator p;
            p = findPointerInfo(other.address);
            p->refCount++;

            address = other.address;
            arraySize = other.arraySize;
            isArray = arraySize > 0;

            #ifndef DEBUG
                cout << "Construindo cópia.";
                if (isArray)
                    cout << " Tamanho é " << arraySize << endl;
                else
                    cout << endl;
            #endif
        }

        ~GracePointer () {
            typename list<GraceInfo<T>>::iterator p;
            p = findPointerInfo(address);

            if (p->refCount) p->refCount--;

            #ifdef DEBUG
                cout << "GracePointer fora de escopo." << endl;
            #endif

            collect();
        }

        static bool collect () {
            bool memFreed = false;

            #ifdef DEBUG
                cout << "Antes da coleta de lixo para ";
                showList();
            #endif

            typename list<GraceInfo<T>>::iterator p;

            do {
                for (p = garbageList.begin(); p != garbageList.end(); p++) {
                    if (p->refCount > 0) continue;

                    memFreed = true;
                    garbageList.remove(*p);

                    if (p->memPointer) {
                        if (p->isArray) {
                            #ifdef DEBUG
                                cout << "Apagando array de tamanho " << p->arraySize << endl;
                            #endif

                            delete [] p->memPointer;
                        } else {
                            #ifdef DEBUG
                                cout << "Apagando: " << *(T *) p->memPointer << endl;
                            #endif

                            delete p->memPointer;
                        }

                        break;
                    }
                } 
            } while (p != garbageList.end());

            #ifdef DEBUG
                cout << "Depois da coleta de lixo para ";
                showList();
            #endif

            return memFreed;
        }

        T *operator= (T *t) {
            typename list<GraceInfo<T>>::iterator p;

            p = findPointerInfo(address);
            p->refCount--;
            p = findPointerInfo(t);

            if (p != garbageList.end())
                p->refCount++;
            else {
                GraceInfo<T> garbageObject(t, SIZE);
                garbageList.push_front(garbageObject);
            }

            address = t;

            return t;
        }

        GracePointer &operator= (GracePointer &rv) {
            typename list<GraceInfo<T>>::iterator p;

            p = findPointerInfo(address);
            p->refCount--;

            p = findPointerInfo(rv.address);
            p->refCount++;
            address = rv.address;

            return rv;
        }

        T &operator* () {
            return *address;
        }

        T *operator->() {
            return address;
        }

        T &operator[] (int i) {
            return address[i];
        }

        operator T *() { return address; }

        Iter<T> begin () {
            int size;

            if (isArray)
                size = arraySize;
            else
                size = 1;

            return Iter<T>(address, address, address + size);
        }

        Iter<T> end () {
            int size;

            if (isArray)
                size = arraySize;
            else
                size = 1;

            return Iter<T>(address + size, address, address + size);
        }

        static int garbageListSize () { return garbageList.size(); }

        static void showList () {
            typename list<GraceInfo<T>>::iterator p;

            cout << "garbageList<" << typeid(T).name() << ", " << SIZE << ">:" << endl;
            cout << "memPointer     refCount        value" << endl;

            if (garbageList.begin() == garbageList.end()) {
                cout << "           -- Vazio --" << endl << endl;
                return;
            }

            for (p = garbageList.begin(); p != garbageList.end(); p++) {
                cout << "[" << (void *)p->memPointer << "[      " << p->refCount << "       ";

                if (p->memPointer)
                    cout << "   " << *p->memPointer;
                else
                    cout << "endl";
            }

            cout << endl;
        }

        static void shutdown () {
            if (garbageListSize() == 0) return;

            typename list<GraceInfo<T>>::iterator p;

            for (p = garbageList.begin(); p != garbageList.end(); p++) {
                p->refCount = 0;
            }

            #ifdef DEBUG
                cout << "Antes de coletar para shutdown() para " << typeid(T).name() << endl;
            #endif

            collect();

            #ifdef DEBUG
                cout << "Depois de coletar para shutdown() para " << typeid(T).name() << endl;
            #endif
        }
    };

    template<class T, int SIZE>
    list<GraceInfo<T>> GracePointer<T, SIZE>::garbageList;

    template<class T, int SIZE>
    bool GracePointer<T, SIZE>::first = true;
}


#endif