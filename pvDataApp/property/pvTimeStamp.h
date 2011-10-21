/* pvTimeStamp.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvDataCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
#include <string>
#include <stdexcept>
#include <pv/pvType.h>
#include <pv/timeStamp.h>
#include <pv/pvData.h>
#ifndef PVTIMESTAMP_H
#define PVTIMESTAMP_H
namespace epics { namespace pvData { 

class PVTimeStamp {
public:
    PVTimeStamp() : pvSecs(0),pvUserTag(0), pvNano(0) {}
    //default constructors and destructor are OK
    //This class should not be extended
    
    //returns (false,true) if pvField(isNot, is valid timeStamp structure
    bool attach(PVField *pvField);
    void detach();
    bool isAttached();
    // following throw logic_error is not attached to PVField
    // a set returns false if field is immutable
    void get(TimeStamp &) const;
    bool set(TimeStamp const & timeStamp);
private:
    PVLong* pvSecs;
    PVInt* pvUserTag;
    PVInt* pvNano;
};
    
}}
#endif  /* PVTIMESTAMP_H */
