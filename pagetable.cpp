/*
 *  Page table functions
 *  CS480-1001
 *  4-4-2023
 *  @author  Noah Hoenig - Gabe Nelson
 *  @redID   824412349 - 823746450
 */
#include "pagetable.h"
#include <iostream>
#include "bit_operations.h"
#define ADDRESS_SPACE 32
PageTable *getPageTable(unsigned int numLevels, unsigned int *levelSizes)
{

    PageTable *pageTable = new PageTable();
    pageTable->levelCount = numLevels;

    pageTable->bitmask = new unsigned int[numLevels];
    pageTable->bitShift = new unsigned int[numLevels];
    pageTable->entryCount = new unsigned int[numLevels];
    int totalBits = ADDRESS_SPACE;
    for (unsigned int i = 0; i < numLevels; i++)
	{
        pageTable->bitmask[i] = calculateBitmask(levelSizes[i],totalBits);
        pageTable->bitShift[i] = calculateBitShift(levelSizes[i],totalBits);
        pageTable->entryCount[i] = calculateEntryCount(levelSizes[i]);
        totalBits -=levelSizes[i];
    }
    pageTable->rootNodePtr = getLevel(pageTable, 0);

	return pageTable;
}


Level *getLevel(PageTable *pageTable, unsigned int depth)
{
	Level *currentLevel = new Level();

	currentLevel->PageTablePtr = pageTable;

	currentLevel->depth = depth;
    
	if (depth == pageTable->levelCount - 1)
	{   
		currentLevel->MapPtr = getMap(pageTable, depth); 
	}
	else
	{
		currentLevel->NextLevelPtr = new Level*[pageTable->entryCount[depth]](); 
		for (unsigned int i = 0; i < pageTable->entryCount[depth]; i++)
		{
			currentLevel->NextLevelPtr[i] = nullptr; 
		}
	}
	return currentLevel;
}
Map **getMap(PageTable *pageTable, unsigned int depth)
{
    Map** mapArr = new Map*[pageTable->entryCount[depth]];
	
    for (unsigned int i = 0; i < pageTable->entryCount[depth]; i++) {
        Map* map = new Map(); 
        mapArr[i] = map;
        mapArr[i]->valid = false; 
    }

    return mapArr;
}

unsigned int virtualAddressToVPN(unsigned int virtualAddress, unsigned int mask, unsigned int shift){
    return (mask & virtualAddress) >> shift;
}

Map * lookup_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress){
    Level *levelPtr = pageTable->rootNodePtr;
    for(unsigned int i = 0; i < pageTable->levelCount; i++){
        unsigned int vpn = virtualAddressToVPN(virtualAddress,pageTable->bitmask[i], pageTable->bitShift[i]);
        if(levelPtr->depth == pageTable->levelCount - 1){
            if(levelPtr->MapPtr[vpn]->valid){
                return levelPtr->MapPtr[vpn];
            }
        }
        if(levelPtr->NextLevelPtr && levelPtr->NextLevelPtr[vpn] != nullptr){
            levelPtr = levelPtr->NextLevelPtr[vpn];
        }
        else{
            return nullptr;
        }

    }
    return nullptr;
}
void insert_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress, unsigned int frame){
    Level *level = pageTable->rootNodePtr;
    for(unsigned int i = 0; i < pageTable->levelCount; i++){
        unsigned int vpn = virtualAddressToVPN(virtualAddress, pageTable->bitmask[i], pageTable->bitShift[i]);
        
        if(i == pageTable->levelCount - 1){
            if(level->MapPtr[vpn]->valid == false){
                level->MapPtr[vpn]->valid = true;
                level->MapPtr[vpn]->frameNumber = frame;
                return;
            }
        }if(level->NextLevelPtr[vpn] == nullptr){
            level->NextLevelPtr[vpn] = getLevel(pageTable, i+1);
        }
        level = level->NextLevelPtr[vpn];

    }
}
bool search_tlb(unsigned int vpn,TLB *tlbCache){
    auto iter = tlbCache->cache.find(vpn);

    if (iter != tlbCache->cache.end()) {
        return true;
    } else {
        return false;
    }
}
void insert_tlb(unsigned int vpn,unsigned int pfn, TLB *tlbCache){
    tlbCache->cache.insert({vpn,pfn});
}

