#ifndef GRACE
#define GRACE

#include <cstdlib>
#include <cmath>
#include <new>

namespace grace {
    template <size_t BLOCK_SIZE = 16>
    class MemoryPool {
        public:
        struct Header {
            size_t length;
            Header() : length(0u) { }
        };

        struct MemoryBlock : public Header {
            union {                
                MemoryBlock * next;
                char rawData[BLOCK_SIZE];
            };

            MemoryBlock() : Header(), next (nullptr) { };
        };

        private:
        unsigned int numberOfBlocks;    // Number of blocks in the list.
        MemoryBlock *pool;              // Head of the list.
        MemoryBlock &listHead;          // End of the list.

        public:
        static constexpr size_t R_BLOCK_SIZE = sizeof(grace::MemoryPool<BLOCK_SIZE>::MemoryBlock);
        static constexpr size_t HEADER_SIZE = sizeof(grace::MemoryPool<BLOCK_SIZE>::Header);

        explicit MemoryPool(size_t bytes) : numberOfBlocks{(unsigned int)std::ceil((bytes + HEADER_SIZE) / R_BLOCK_SIZE) + 1u},
                                            pool{new MemoryBlock[numberOfBlocks]},
                                            listHead{pool[numberOfBlocks - 1]}
        {
            this->pool[0].length = numberOfBlocks - 1;
            this->pool[0].next = nullptr;

            this->listHead.length = 0;
            this->listHead.next = this->pool;
        }

        ~MemoryPool () {
            delete[] pool;
        }

        void *allocate(size_t bytes) {
            MemoryBlock * cur = this->listHead.next;
            MemoryBlock * prev = this->listHead;
            size_t blocks = std::ceil((bytes + HEADER_SIZE) / R_BLOCK_SIZE);

            while (cur != nullptr) {
                if(cur->length == blocks) {
                    prev->next = cur->next;
                    cur->length = blocks;

                    return reinterpret_cast<void *>(reinterpret_cast<Header *>(cur) + (1U));
                } else if (cur->length > blocks) {
                    prev->next = cur + blocks;
                    prev->next->next = cur->next;
                    prev->next->length = cur->length - blocks;
                    cur->length = blocks;

                    return reinterpret_cast<void *>(reinterpret_cast<Header *>(cur) + (1U));
                } else {
                    prev = cur;
                    cur = cur->next;
                }
            }

            throw std::bad_alloc();
        }

        void free(void *ptr) {
            ptr = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Header *>(ptr) - (1U));

            MemoryBlock *cur = (MemoryBlock *)ptr;
            MemoryBlock *post = this->listHead.next;
            MemoryBlock *prev = this->listHead;

            if (post == nullptr) {
                prev->next = cur;
                post = cur;
            }

            while (post != nullptr) {
                if ((cur - post) > (cur - prev) || (cur - post) < 0) {
                    break;
                } else {
                    prev = post;
                    post = post->next;
                }
            }

            MemoryBlock *pre = post;
            MemoryBlock *pos = post->next;

            // | PRE | CUR | POS | -> | BLK             |
            if ((cur - pre) == (long int)pre->length && (pos - cur) == (long int)cur->length) {
                pre->next = pos->next;
                pre->length = pre->length + cur->length + pos->length;
            }
            // | PRE | CUR | ... | POS | -> | BLK       | ... | POS | 
            else if ((cur - pre) == (long int)pre->length) {
                pre->next = pos;
                pre->length = pre->length + cur->length;
            }
            // | PRE | ... | CUR | POS | -> | PRE | ... | BLK       |
            else if ((pos - cur) == (long int)cur->length) {
                pre->next = cur;
                cur->next = pos->next;
                cur->length = cur->length + pos->length;
            } 
            // | PRE | ... | CUR | ... | POS |
            else {
                cur->next = pos;
                pre->next = cur;
            }
        }
    };

}

#endif