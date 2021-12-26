#pragma once

#include "macros.h"

// TODO(Tobi): "Bucket List"; might also allocate additional memory if needed)
// Unfortunately; cannot really free stuff when not needed anymore since i don't know if something is still referenced
// can only free the memory if definitely the whole thing is not needed anymore
// TODO(Tobi): Improve entry; I could save a t least one spot; where I know it is empty/ where I know, until here; there is nothing empty
// This then really means bucket list

template <typename T>
struct bucket_list {
    int Start;
    int Last;
    int Size;
    // int CheckSpot; // TODO(Tobi): For quicker insert
    T* _Data;
};

struct bucket_entry {
    unsigned int Generation;
    int Index;
};

template <typename T>
struct generation_link {
    T* Pointer;
    unsigned int Generation; // TODO(Tobi): Now, I could also use an unsigned int64
};

// TODO(Tobi): Do I want this to work like that?
template <typename T>
bucket_list<T> BucketListInit(int size, T* data) {
    bucket_list<T> ret = {};
    ret.Size = size;
    ret._Data = data;

    return ret;
}

template <typename T>
void BucketListRemove(bucket_list<T>* bucketList, int index) {
    Assert(
        (bucketList->Start <= bucketList->Last && index >= bucketList->Start && index < bucketList->Last)
        // ||
        // (bucketList->Start >= bucketList->Last && index >= bucketList->Start || index < bucketList->Last)
    , "Object to remove is not active, (%d <-> %d) - %d", bucketList->Start, bucketList->Last, index);
    Assert(bucketList->_Data[index].Generation != 0, "Object already is deleted");

    bucketList->_Data[index].LastGeneration = bucketList->_Data[index].Generation;
    bucketList->_Data[index].Generation = 0;

    // Update the start of the buffer, so that when iterating, I can start at a later point
    if (index == bucketList->Start) {
        bucketList->Start = bucketList->Last;
        inc (i,   index + 1,    bucketList->Last) {
            if (bucketList->_Data[i].Generation != 0) {
                bucketList->Start = i;
                break;
            }
        }
    } else if (index == bucketList->Last) {
        bucketList->Last = bucketList->Start;
        dec (i,   index,    bucketList->Start) {
            if (bucketList->_Data[i].Generation != 0) {
                bucketList->Last = i;
                break;
            }
        }
    }
}

template <typename T>
void BucketListRemove(bucket_list<T>* bucketList, T* element) {
    BucketListRemove(bucketList, (int) (element - bucketList->_Data));
}

template <typename T>
T* BucketGetCleared(bucket_list<T>* bucketList) {

    // TODO(Tobi): Clearing the T normaly should work; this is just due to the ridiculously big hero
    // The big part of it gets cleared anyway later

    for (int i = bucketList->Start; i != bucketList->Last; ++i) {
        T* item = &bucketList->_Data[i];
        if (item->Generation == 0) {
            unsigned int nextGeneration = bucketList->_Data[i].LastGeneration + 1;
            memset(item, 0, sizeof(T)); //*item = {};
            item->Generation = nextGeneration;
            return item;
        }
    }
    
    if (bucketList->Start > 0) {
        int index = --bucketList->Start;
        T* item = &bucketList->_Data[index];
        Assert(item->Generation == 0, "The element in front of start somehow actually hasn't been dead");
        unsigned int nextGeneration = bucketList->_Data[index].LastGeneration + 1;
        memset(item, 0, sizeof(T)); //*item = {};
        item->Generation = nextGeneration;
        return item;
    } else {
        int index = bucketList->Last++;
        Assert(index < bucketList->Size, "The bucket list is not big enough"); // TODO(Tobi): Maybe have this auto-expanding
        T* item = &bucketList->_Data[index];
        Assert(item->Generation == 0, "The element after last somehow actually hasn't been dead");
        unsigned int nextGeneration = bucketList->_Data[index].LastGeneration + 1;
        memset(item, 0, sizeof(T)); //*item = {};
        item->Generation = nextGeneration;
        return item;
    }
}

template <typename T>
T* BucketGetItemOrNull(bucket_list<T>* bucketList, bucket_entry entry) {
    Assert(entry.Generation, "The generation of the entry is 0; this cannot happen");
    T* item = bucketList->_Data[entry.Index];
    if (item->Generation == entry.Generation) {
        return item;
    } else {
        return nullptr;
    }
}

template <typename T>
T* GenerationLinkResolve(generation_link<T> gl) {
    if (!gl.Pointer) { return nullptr; }
    T* item = gl.Pointer;
    if (item->Generation == gl.Generation) {
        return item;
    } else {
        return nullptr;
    }
}

template <typename T>
generation_link<T> GenerationLinkCreate(T* obj) {
    Assert(obj->Generation, "The GenerationLink to create would have generation 0");

    generation_link<T> ret;
    ret.Pointer = obj;
    ret.Generation = obj->Generation;
    return ret;
}

#define ARRAY_REMOVE_LOOP(array, index, count) array[(index)--] = array[--(count)]

// TODO(Tobi): Maybe I will actually provide a version, where I can also set the variable, and that will automatically ignore stuff being dead
#define inc_bucket_dead(variable, bucketList) for (int variable = (bucketList)->Start; variable < (bucketList)->Last; ++variable)

//#define inc_bucket(counterVariable, typeVariable, bucketList) for (int counterVariable = (bucketList)->Start; counterVariable < (bucketList)->Last; ++counterVariable) for (auto* typeVariable = &(bucketList)->_Data[counterVariable]; typeVariable && typeVariable->Generation; typeVariable = nullptr)
#define inc_bucket(counterVariable, typeVariable, bucketList) \
    Assert((bucketList)->_Data, "Array is null"); \
    for (int counterVariable = (bucketList)->Start; counterVariable < (bucketList)->Last; ++counterVariable) \
        if (auto* typeVariable = &(bucketList)->_Data[counterVariable]) \
           if (!typeVariable->Generation) { continue; } else


// #define inc_bucket(counterVariable, typeVariable, bucketList) \
//     for (int counterVariable = (bucketList)->Start; counterVariable < (bucketList)->Last; ++counterVariable) { \
//         auto* typeVariable = &(bucketList)->_Data[counterVariable]; \
//         if (!typeVariable->Generation) { continue; }

// #define inc_bucket(counterVariable, typeVariable, bucketList)
//     for (int counterVariable = (bucketList)->Start; counterVariable < (bucketList)->Last; ++counterVariable) { \
//         switch (true) {
//             case true:
//         }


// NOTE(Tobi): I don't think there is any reason to do that, since bucket already means that I don't care about the order and additionally, I don'tcopy stuff around
// #define dec_bucket(variable, bucketList) for (int variable = (bucketList)->Last; --variable >= (bucketList)->Start; )


