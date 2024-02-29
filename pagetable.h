/*
 *  Page table functions
 *  CS480-1001
 *  4-4-2023
 *  @author  Noah Hoenig - Gabe Nelson
 *  @redID   824412349 - 823746450
 */
#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H
#include <unordered_map>


class PageTable;

class Map{
    public:
        unsigned int frameNumber;
        bool valid;

};
class Level{
    public:
        unsigned int depth;      // current depth of level 
        PageTable *PageTablePtr; // Pointer to pagetable
        Level** NextLevelPtr;  // Pointer to level array
        Map** MapPtr;          // Pointer to map array

};
class PageTable { 
   
    public:
        Level* rootNodePtr; 

        unsigned int levelCount; // Number of levels

        unsigned int *bitmask; // bit mask for level i

        unsigned int *bitShift; // # of bits to shift level i page bits

        unsigned int *entryCount; // # of possible pages for level i

};

class TLB {
    public:
        std::unordered_map<int, int> cache;
        
};

// Initialize page table
PageTable *getPageTable(unsigned int numLevels, unsigned int *levelSizes);

// Initialize levels
Level *getLevel(PageTable *pagetable, unsigned int currentDepth);

// Initialize map
Map **getMap(PageTable *pg, unsigned int depth);

unsigned int virtualAddressToVPN(unsigned int virtualAddress, unsigned int mask, unsigned int shift);

Map * lookup_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress);

void insert_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress, unsigned int frame);

bool search_tlb(unsigned int vpn,TLB *tlbCache);

void insert_tlb(unsigned int vpn, unsigned int pfn, TLB *tlbCache);

#endif