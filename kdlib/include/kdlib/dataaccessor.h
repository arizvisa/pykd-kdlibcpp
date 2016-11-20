#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include <kdlib/dbgtypedef.h>
#include <kdlib/variant.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DataAccessor;
typedef boost::shared_ptr<DataAccessor> DataAccessorPtr;

class DataAccessor
{
public:

    virtual size_t getLength() const = 0;

    virtual unsigned char readByte(size_t pos=0) const = 0;
    virtual void writeByte(unsigned char value, size_t pos=0) = 0;

    virtual char readSignByte(size_t pos=0) const = 0;
    virtual void writeSignByte(char value, size_t pos=0) = 0;

    virtual unsigned short readWord(size_t pos=0) const = 0;
    virtual void writeWord(unsigned short value, size_t pos=0) = 0;

    virtual short readSignWord(size_t pos=0) const = 0;
    virtual void writeSignWord(short value, size_t pos=0) = 0;

    virtual unsigned long readDWord(size_t pos=0) const = 0;
    virtual void writeDWord(unsigned long value, size_t pos=0) = 0;

    virtual long readSignDWord(size_t pos=0) const = 0;
    virtual void writeSignDWord(long value, size_t pos=0) = 0;

    virtual unsigned long long readQWord(size_t pos=0) const = 0;
    virtual void writeQWord(unsigned long long value, size_t pos=0) = 0;

    virtual long long readSignQWord(size_t pos=0) const = 0;
    virtual void writeSignQWord(long long value, size_t pos=0) = 0;

    virtual float readFloat(size_t pos=0) const = 0;
    virtual void writeFloat(float value, size_t pos=0) = 0;

    virtual double readDouble(size_t pos=0) const = 0;
    virtual void writeDouble(double value, size_t pos=0) = 0;

    virtual void readBytes( std::vector<unsigned char>&  dataRange, size_t count, size_t pos=0) const = 0;
    virtual void writeBytes( const std::vector<unsigned char>&  dataRange, size_t pos=0) = 0;

    virtual void readWords( std::vector<unsigned short>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeWords( const std::vector<unsigned short>&  dataRange, size_t pos=0)  = 0;

    virtual void readDWords( std::vector<unsigned long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeDWords( const std::vector<unsigned long>&  dataRange, size_t  pos=0)  = 0;

    virtual void readQWords( std::vector<unsigned long long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeQWords( const std::vector<unsigned long long>&  dataRange, size_t  pos=0) = 0;

    virtual void readSignBytes( std::vector<char>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeSignBytes( const std::vector<char>&  dataRange, size_t  pos=0) = 0;

    virtual void readSignWords( std::vector<short>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeSignWords( const std::vector<short>&  dataRange, size_t  pos=0) = 0;

    virtual void readSignDWords( std::vector<long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeSignDWords( const std::vector<long>&  dataRange, size_t  pos=0) = 0;

    virtual void readSignQWords( std::vector<long long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeSignQWords( const std::vector<long long>&  dataRange, size_t  pos=0) = 0;

    virtual void readFloats( std::vector<float>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeFloats( const std::vector<float>&  dataRange, size_t  pos=0) = 0;

    virtual void readDoubles( std::vector<double>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void writeDoubles( const std::vector<double>&  dataRange, size_t  pos=0) = 0;

    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual VarStorage getStorageType() const = 0;
    virtual std::wstring getRegisterName() const = 0;
};

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr  getEmptyAccessor();
DataAccessorPtr getMemoryAccessor( MEMOFFSET_64  offset, size_t length);
DataAccessorPtr getCacheAccessor(const std::vector<char>& buffer);
DataAccessorPtr getCacheAccessor(size_t bufferSize);
//DataAccessorPtr getVariantAccessor(const NumVariant& var);
DataAccessorPtr  getRegisterAccessor(const std::wstring& registerName);


///////////////////////////////////////////////////////////////////////////////

}
