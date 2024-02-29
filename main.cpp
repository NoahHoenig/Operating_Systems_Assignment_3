/*
 *  Program # 3
 *  This files runs the application and main thread 
 *  Compile using the Makefile command 'make' in command prompt
 *  Run program using './mmuwithtlb trace.tr' with optional arg
 *  -n, -c, -p in command prompt
 *  CS480-1001
 *  4-4-2023
 *  @author  Noah Hoenig - Gabe Nelson
 *  @redID   824412349 - 823746450
 */
#include <iostream>
#include <unistd.h>
#include <string>
#include <cmath>
#include "vaddr_tracereader.h"
#include "pagetable.h"
#include "bit_operations.h"
#include "print_helpers.h"

#define ADDRESS_SPACE 32
#define LRU_SIZE 8
#define BADFLAG 1
#define LIMIT_BITS 28

using namespace std; 

int main(int argc, char **argv)
{
    int option;   /* command line switch */
    int idx;  // general purpose index variable
    int num_mem_accesses = -1;
    int cache_capacity = 0;
    string mode = "summary";

    
    while ( (option = getopt(argc, argv, "n:c:p:")) != -1) {
        switch (option) {
            case 'n': // Process only the first N memory accesses / references. Processes all addresses if not present
                num_mem_accesses = atoi(optarg);  
                if(num_mem_accesses < 0) {
                    cerr << "Number of memory accesses must be a number, greater than or equal to 0" << endl;
                    exit(BADFLAG);
                }
                break;
            case 'c': // Cache capacity of the TLB
                cache_capacity = atoi(optarg);
                if(cache_capacity < 0 ){
                    cerr << "Cache capacity must be a number, greater than or equal to 0" << endl;
                    exit(BADFLAG);
                }
                break;
            case 'p':  // Mode is a string that specifies what to be printed to the standard output:
                mode = optarg;
                break;
            default:
                cerr << "Usage: " << argv[0] << " <<ARG1>> <<ARG2>> -p N -h N -n N" << endl;
                exit(BADFLAG); 
        }
    }
    idx = optind;
    
    FILE *ifp;	        /* trace file */
    const char* file_name = argv[idx];
    unsigned long vAddr = 0;  /* instructions processed */
    p2AddrTr trace;	/* traced address */
    idx ++;
    /* ERROR CHECKING */
    if(idx == argc){
        cerr << "USAGE. Specify bits per level" << endl;
        exit(BADFLAG);
    }

    // Total #of bits for all levels
    unsigned int sum = 0;
    // Calculate number of levels
    unsigned int numLevels = argc - idx; /*Amount of levels from command line stored in PageTable struct*/
    // Number of bits for each level
    unsigned int levelSizes[numLevels];
    
    int i = 0;
    // Read in remaing mandator arguments rep bits per level 
    while(idx < argc){
        if(atoi(argv[idx]) < 1){
            cerr << "Level " << i << " page table must be at least 1 bit" <<endl;
            exit(BADFLAG);
        }
        sum += atoi(argv[idx]);
        levelSizes[i] = atoi(argv[idx]);
        i++;
        idx++;
    }
    // Total # of bits for all levels should be less than or equal to 28
    if(sum > LIMIT_BITS){
        cerr << "Too many bits used in page tables" << endl;
        exit(BADFLAG);
    }
    
    // Declare and istantiate page table
    PageTable *pageTable = getPageTable(numLevels, levelSizes);
    

    // No address processing needed for this mode
    if(mode == "levelbitmasks"){
        report_levelbitmasks(numLevels,pageTable->bitmask);
    }

    // Open trace file
    if ((ifp = fopen(file_name,"rb")) == NULL) {
        cerr << "Unable to open <<" << file_name << ">> " << endl;
        exit(BADFLAG);
    }
    // count addresses read in
    int counter = 0;
    // frame for PFN
    unsigned int currentFrame = 0;
    // Track hits in tlb cache and page table
    unsigned int tlbHitCounter = 0;
    unsigned int pageHitCounter = 0;
    bool tlbHit = false;
    bool pageHit = false;
    // Declare TLB cache and LRU data structures
    TLB *tlbCache = new TLB();
    std::unordered_map<int, int> LRU;
    bool check = false;
    
    // Read in virtual addresses one by one
    while(NextAddress(ifp, & trace)) //ifp - file handle from fopen
    {
        // Only proccess n # of addresses
        if(num_mem_accesses == counter){
            break;
        }

        vAddr = trace.addr;
        unsigned int offset = (calculateBitmask(ADDRESS_SPACE-sum,ADDRESS_SPACE-sum) & vAddr) >> calculateBitShift(ADDRESS_SPACE-sum,ADDRESS_SPACE-sum);
        unsigned int vpn = virtualAddressToVPN(vAddr,calculateBitmask(sum,ADDRESS_SPACE), calculateBitShift(sum,ADDRESS_SPACE));
        unsigned int pfn;
        
        // Add vpn to LRU
        auto iter = LRU.find(vpn);
        if (iter != LRU.end()) {
            // Key exists, increment value
            iter->second += 1;
        } else {
            // Key does not exist, insert new key-value pair
            LRU[vpn] = counter;
        }

        
        int minimum = LRU.begin()->second; 
        // Iterate through the LRU and find the smallest value
        for (auto it = LRU.begin(); it != LRU.end(); ++it) {
            if (it->second < minimum) {
                minimum = it->second;
            }
        }
        
        // Remove the least recently used cache entry from TLB
        if(static_cast<int>(tlbCache->cache.size()) > cache_capacity){
            for (const auto& kv : tlbCache->cache) {
                for (auto it = LRU.begin(); it != LRU.end(); ++it) {
                    if (it->second == minimum) {
                        check = true;
                        tlbCache->cache.erase(it->first);
                        break; 
                    }
                }
                if(check == true){
                    break;
                }
            }
        }
        // Remove least use mapping once size exceeds 8
        if (LRU.size() > LRU_SIZE) {
            for (auto it = LRU.begin(); it != LRU.end(); ++it) {
                if (it->second == minimum) {
                    LRU.erase(it);
                    break; 
                }
            }
        }
        
        // search tlb
        if(search_tlb(vpn,tlbCache)&&cache_capacity>0){
            tlbHitCounter++;
            tlbHit = true;
            pfn = tlbCache->cache[vpn];
            // return vpn mapping
        }else{
            Map *mappingFound = lookup_vpn2pfn(pageTable, vAddr);
            if(mappingFound == nullptr){
                insert_vpn2pfn(pageTable, vAddr, currentFrame);
                ++currentFrame;
            }else{
                pageHitCounter++;
                pageHit = true;
            }
            pfn = lookup_vpn2pfn(pageTable, vAddr)->frameNumber;
            // return vpn mapping
            // insert mapping to TLB cache
            insert_tlb(vpn,pfn,tlbCache);
        }
        unsigned int pAddr =  (pfn << (ADDRESS_SPACE - sum)) + offset;
        
        // PRINT MODES
        unsigned int pages[numLevels];
        for(unsigned int i = 0; i < numLevels; i++){
            pages[i] = virtualAddressToVPN(vAddr,pageTable->bitmask[i], pageTable->bitShift[i]);
        }
        
        if(mode == "offset"){
            hexnum(offset);
        }
        if(mode == "vpn2pfn"){
            report_pagetable_map(numLevels,pages,pfn);
        }
        if(mode=="va2pa"){
            report_virtualAddr2physicalAddr(vAddr,pAddr);
        }
        if(mode=="va2pa_tlb_ptwalk"){
            report_va2pa_TLB_PTwalk(vAddr,pAddr,tlbHit,pageHit);
        }
        counter++;
        check = false;
        tlbHit = false;
        pageHit = false;
    }

    // Size of whole page table data structure
    unsigned int size = 0;
    for(unsigned int i = 0; i < numLevels; i++)
    {
        size += pageTable->entryCount[i];
    }
    size += sizeof(pageTable);
    
    // DEFAULT PRINT MODE
    if(mode=="summary"){
        report_summary(pow(2,ADDRESS_SPACE-sum),tlbHitCounter,pageHitCounter,counter,currentFrame,size);
    }

    fclose(ifp);

    return 0;
}
