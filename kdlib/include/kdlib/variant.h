#pragma once 

#include <exception>
#include <string>
#include <sstream>

#include <boost/operators.hpp>

namespace kdlib {


class NumVariantError : public std::exception
{
};


template<typename T>
struct equal_op {

    bool operator()(const T &t1, const T &t2) {
        return t1 == t2;
    }
};

template<typename T>
struct gt_op
{
    bool operator()(const T &t1, const T &t2) {
        return t1 < t2;
    }
};

template<typename T>
struct add_op {

    T operator() (const T &t1, const T &t2) {
        return t1 + t2;
    }
};

template<typename T>
struct sub_op {

    T operator() (const T &t1, const T &t2) {
        return t1 - t2;
    }
};


template<typename T>
struct mul_op {

    T operator() (const T &t1, const T &t2) {
        return t1 * t2;
    }
};


template<typename T>
struct div_op {

    T operator() (const T &t1, const T &t2) {
        return t1 / t2;
    }
};



template<typename T>
struct mod_op {

    T operator() (const T &t1, const T &t2) {
        return t1 % t2;
    }

};

template<typename T>
struct or_op {

    T operator() (const T &t1, const T &t2) {
        return t1 | t2;
    }
};


template<typename T>
struct and_op {

    T operator() (const T &t1, const T &t2) {
        return t1 & t2;
    }
};


template<typename T>
struct xor_op {

    T operator() (const T &t1, const T &t2) {
        return t1 ^ t2;
    }
};

template<typename T>
struct lshift_op {
    T operator() (const T &t1, const T &t2) {
        return t1 << t2;
    }
};

template<typename T>
struct rshift_op {
    T operator() (const T &t1, const T &t2) {
        return t1 >> t2;
    }
};



class NumVariant
{

public:

    NumVariant()
    {
        m_numType = ucharT;
        m_ucharVal = 0;
    }

    NumVariant(const NumVariant  &var)
    {
        copy(var);
    }

    NumVariant operator=(const NumVariant  &var)
    {
        copy(var);
        return *this;
    }

    //NumVariant(bool val)
    //{
    //    m_numType = ucharT;
    //    m_ucharVal = val;
    //}

    NumVariant(char val)
    {
        m_numType = charT;
        m_charVal = val;
    }

    NumVariant(unsigned char val)
    {
        m_numType = ucharT;
        m_ucharVal = val;
    }

    NumVariant(short val)
    {
        m_numType = shortT;
        m_shortVal = val;
    }

    NumVariant(unsigned short val)
    {
        m_numType = ushortT;
        m_ushortVal = val;
    }

    NumVariant(long val)
    {
        m_numType = longT;
        m_longVal = val;
    }

    NumVariant(unsigned long val)
    {
        m_numType = ulongT;
        m_ulongVal = val;
    }

    NumVariant(long long val)
    {
        m_numType = longlongT;
        m_longlongVal = val;
    }


    NumVariant(unsigned long long val)
    {
        m_numType = ulonglongT;
        m_ulonglongVal = val;
    }

    NumVariant(int val)
    {
        m_numType = intT;
        m_intVal = val;
    }

    NumVariant(unsigned int val)
    {
        m_numType = uintT;
        m_intVal = val;
    }

    NumVariant(float val)
    {
        m_numType = floatT;
        m_floatVal = val;
    }

    NumVariant(double val)
    {
        m_numType = doubleT;
        m_doubleVal = val;
    }

    template<typename T>
    NumVariant(const T* val)
    {
        m_numType = ulonglongT;
        m_ulonglongVal = (unsigned long long)val;
    }

    char asChar() const {
        return cast(charT).m_charVal;
    }

    unsigned char asUChar() const {
        return cast(ucharT).m_ucharVal;
    }

    short asShort() const {
        return cast(shortT).m_shortVal;
    }

    unsigned short asUShort() const {
        return cast(ushortT).m_ushortVal;
    }

    long asLong() const {
        return cast(longT).m_longVal;
    }

    unsigned long asULong() const {
        return cast(ulongT).m_ulongVal;
    }

    long long asLongLong() const {
        return cast(longlongT).m_longlongVal;
    }

    unsigned long long asULongLong() const {
        return cast(ulonglongT).m_ulonglongVal;
    }

    int asInt() const {
        return cast(intT).m_intVal;
    }

    unsigned int asUInt() const {
        return cast(uintT).m_uintVal;
    }

    float asFloat() const {
        return cast(floatT).m_floatVal;
    }

    double asDouble() const {
        return cast(doubleT).m_doubleVal;
    }

    bool asBool() const {
        return cast(ucharT).m_ucharVal != 0;
    }

    std::wstring asStr() const {

        std::wstringstream sstr;

        sstr << std::dec;

        switch (m_numType)
        {
        case charT:
        case shortT:
        case longT:
        case intT:
            sstr << asInt();
            return sstr.str();

        case ucharT:
        case ushortT:
        case ulongT:
        case uintT:
            sstr << asUInt();
            return sstr.str();

        case longlongT:
            sstr << m_longlongVal;
            return sstr.str();

        case ulonglongT:
            sstr << m_ulonglongVal;
            return sstr.str();

        case floatT:
            sstr.precision(8);
            sstr << std::defaultfloat << m_floatVal;
            return sstr.str();

        case doubleT:
            sstr.precision(16);
            sstr << std::defaultfloat << m_doubleVal;
            return sstr.str();
        }

        throw NumVariantError();
    }

    std::wstring asHex() const {

        std::wstringstream sstr;

        sstr << std::hex << std::showbase;

        switch (m_numType)
        {
        case charT:
        case shortT:
        case longT:
        case intT:
            sstr << asInt();
            return sstr.str();

        case ucharT:
        case ushortT:
        case ulongT:
        case uintT:
            sstr << asUInt();
            return sstr.str();

        case longlongT:
            sstr << m_longlongVal;
            return sstr.str();

        case ulonglongT:
            sstr << m_ulonglongVal;
            return sstr.str();

        case floatT:
            sstr.precision(8);
            sstr << std::hexfloat << m_floatVal;
            return sstr.str();

        case doubleT:
            sstr.precision(8);
            sstr << std::hexfloat << m_doubleVal;
            return sstr.str();
        }

        throw NumVariantError();
    }

    size_t getSize() const
    {
        switch (m_numType)
        {
        case charT: return sizeof(char);
        case shortT: return sizeof(short);
        case longT: return sizeof(long);
        case intT: return sizeof(int);
        case ucharT: return sizeof(unsigned char);
        case ushortT: return sizeof(unsigned short);
        case ulongT: return sizeof(unsigned long);
        case uintT: return sizeof(unsigned int);
        case longlongT: return sizeof(long long);
        case ulonglongT: return sizeof(unsigned long long);
        case floatT: return sizeof(float);
        case doubleT: return sizeof(double);
        }

        throw NumVariantError();
    }

    bool isChar() const {
        return m_numType == charT;
    }

    bool isUChar() const {
        return m_numType == ucharT;
    }

    bool isShort() const {
        return m_numType == shortT;
    }

    bool isUShort() const {
        return m_numType == ushortT;
    }

    bool isLong() const {
        return m_numType == longT;
    }

    bool isULong() const {
        return m_numType == ulongT;
    }

    bool isLongLong() const {
        return m_numType == longlongT;
    }

    bool isULongLong() const {
        return m_numType == ulonglongT;
    }

    bool isInt() const {
        return m_numType == intT;
    }

    bool isUInt() const {
        return m_numType == uintT;
    }

    bool isFloat() const {
        return m_numType == floatT;
    }

    bool isDouble() const {
        return m_numType == doubleT;
    }

    bool isSigned() const {
        return NumVariant(-1).cast(m_numType) < NumVariant(0);
    }

    bool isInteger() const {
        return !isFloat() && !isDouble();
    }

    void setChar(char val) {
        m_numType = charT;
        m_charVal = val;
    }

    void setBool(bool val) {
        setChar(val);
    }

    void setUChar(unsigned char val) {
        m_numType = ucharT;
        m_ucharVal = val;
    }

    void setShort(short val) {
        m_numType = shortT;
        m_shortVal = val;
    }

    void setUShort(unsigned short val) {
        m_numType = ushortT;
        m_ushortVal = val;
    }

    void setLong(long val) {
        m_numType = longT;
        m_longVal = val;
    }

    void setULong(unsigned long val) {
        m_numType = ulongT;
        m_ulongVal = val;
    }

    void setLongLong(long long val) {
        m_numType = longlongT;
        m_longlongVal = val;
    }

    void setULongLong(unsigned long long val) {
        m_numType = ulonglongT;
        m_ulonglongVal = val;
    }

    void setInt(int val) {
        m_numType = intT;
        m_intVal = val;
    }

    void setUInt(unsigned int val) {
        m_numType = uintT;
        m_uintVal = val;
    }

    void setFloat(float val) {
        m_numType = floatT;
        m_floatVal = val;
    }

    void setDouble(double val) {
        m_numType = doubleT;
        m_doubleVal = val;
    }

    friend bool operator==(const NumVariant& v1, const NumVariant& v2);
    friend bool operator!=(const NumVariant& v1, const NumVariant& v2);
    friend bool operator<(const NumVariant& v1, const NumVariant& v2);
    friend bool operator<=(const NumVariant& v1, const NumVariant& v2);
    friend bool operator>(const NumVariant& v1, const NumVariant& v2);
    friend bool operator>=(const NumVariant& v1, const NumVariant& v2);
    friend bool operator&&(const NumVariant& v1, const NumVariant& v2);
    friend bool operator||(const NumVariant& v1, const NumVariant& v2);


    friend NumVariant operator+(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator-(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator*(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator/(const NumVariant& v1, const NumVariant& v2);

    friend NumVariant operator^(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator&(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator|(const NumVariant& v1, const NumVariant& v2);

    friend NumVariant operator%(const NumVariant& v1, const NumVariant& v2);

    friend NumVariant operator<<(const NumVariant& v1, const NumVariant& v2);
    friend NumVariant operator>>(const NumVariant& v1, const NumVariant& v2);

    friend NumVariant operator~(const NumVariant& v1);
    friend bool operator!(const NumVariant& v1);

    friend NumVariant operator++(NumVariant& v1);
    friend NumVariant operator--(NumVariant& v1);
    friend NumVariant operator++(NumVariant& v1, int);
    friend NumVariant operator--(NumVariant& v1, int);
    friend NumVariant operator-(const NumVariant& v1);

    template< typename RetT, template<class> class FuncT>
    static
        RetT all_op(const NumVariant& v1, const NumVariant& v2)
    {
        NumVariant::NumType  t = maxType(v1, v2);
        NumVariant v3 = v1.cast(t);
        NumVariant v4 = v2.cast(t);

        switch (t)
        {
        case floatT:
            return FuncT<float>()(v3.m_floatVal, v4.m_floatVal);

        case doubleT:
            return FuncT<double>()(v3.m_doubleVal, v4.m_doubleVal);
        }

        return int_op<RetT, FuncT>(v3, v4);
    }

    template< typename RetT, template<class> class FuncT>
    static
        RetT int_op(const NumVariant& v1, const NumVariant& v2)
    {
        NumVariant::NumType  t = maxType(v1, v2);
        NumVariant v3 = v1.cast(t);
        NumVariant v4 = v2.cast(t);

        switch (t)
        {
        case charT:
            return FuncT<char>()(v3.m_charVal, v4.m_charVal);

        case ucharT:
            return FuncT<unsigned char>()(v3.m_ucharVal, v4.m_ucharVal);

        case shortT:
            return FuncT<short>()(v3.m_shortVal, v4.m_shortVal);

        case ushortT:
            return FuncT<unsigned short>()(v3.m_ushortVal, v4.m_ushortVal);

        case longT:
            return FuncT<long>()(v3.m_longVal, v4.m_longVal);

        case ulongT:
            return FuncT<unsigned long>()(v3.m_ulongVal, v4.m_ulongVal);

        case longlongT:
            return FuncT<long long>()(v3.m_longlongVal, v4.m_longlongVal);

        case ulonglongT:
            return FuncT<unsigned long long>()(v3.m_ulonglongVal, v4.m_ulonglongVal);

        case intT:
            return FuncT<int>()(v3.m_intVal, v4.m_intVal);

        case uintT:
            return FuncT<unsigned int>()(v3.m_uintVal, v4.m_uintVal);

        case floatT:
        case doubleT:
            throw NumVariantError();
        }

        throw NumVariantError();
    }


    explicit operator char() const {
        return asChar();
    }

    explicit operator unsigned char() const {
        return asUChar();
    }

    explicit operator short() const {
        return asShort();
    }

    explicit operator unsigned short() const {
        return asUShort();
    }

    explicit operator unsigned long() const {
        return asULong();
    }

    explicit operator long() const {
        return asLong();
    }

    explicit operator unsigned long long() const {
        return asULongLong();
    }

    explicit operator long long()  const {
        return asLongLong();
    }

    explicit operator float() const {
        return asFloat();
    }

    explicit operator double() const {
        return asDouble();
    }

    explicit operator int() const {
        return asInt();
    }

    explicit operator unsigned int() const {
        return asUInt();
    }

    explicit operator bool() const {
        return asChar() != 0;
    }


private:

    enum NumType { charT, ucharT, shortT, ushortT, longT, ulongT, longlongT, ulonglongT, intT, uintT, floatT, doubleT, maxT };

    NumType m_numType;

    union {
        char m_charVal;
        unsigned char m_ucharVal;
        short m_shortVal;
        unsigned short m_ushortVal;
        long m_longVal;
        unsigned long m_ulongVal;
        long long m_longlongVal;
        unsigned long long m_ulonglongVal;
        int m_intVal;
        unsigned int m_uintVal;
        float m_floatVal;
        double m_doubleVal;
    };

    static NumType maxType(const NumVariant& v1, const NumVariant& v2)
    {
        static const NumType castMatrix[maxT][maxT] =
        {
            //  charT         ucharT      shortT      ushortT     longT       ulongT,         longlongT       ulonglongT,     intT        uintT       floatT      doubleT
                { intT,         intT,       intT,       intT,       longT,      ulongT,         longlongT,      ulonglongT,     intT,       uintT,      floatT,     doubleT },     //charT
                { maxT,         intT,       intT,       intT,       longT,      ulongT,         longlongT,      ulonglongT,     intT,       uintT,      floatT,     doubleT },     //ucharT
                { maxT,         maxT,       intT,       intT,       longT,      ulongT,         longlongT,      ulonglongT,     intT,       uintT,      floatT,     doubleT },     //shortT
                { maxT,         maxT,       maxT,       intT,       longT,      ulongT,         longlongT,      ulonglongT,     intT,       uintT,      floatT,     doubleT },     //ushortT
                { maxT,         maxT,       maxT,       maxT,       longT,      ulongT,         longlongT,      ulonglongT,     longT,      ulongT,     floatT,     doubleT },     //longT
                { maxT,         maxT,       maxT,       maxT,       maxT,       ulongT,         longlongT,      ulonglongT,     ulongT,     ulongT,     floatT,     doubleT },     //ulongT
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           longlongT,      ulonglongT,     longlongT,  longlongT,  floatT,     doubleT },     //longlongT 
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           maxT,           ulonglongT,     ulonglongT, ulonglongT, floatT,     doubleT },     //ulonglongT
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           maxT,           maxT,           intT,       uintT,      floatT,     doubleT },     //intT
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           maxT,           maxT,           maxT,       uintT,      floatT,     doubleT },     //uintT
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           maxT,           maxT,           maxT,       maxT,       floatT,     doubleT },     //floatT
                { maxT,         maxT,       maxT,       maxT,       maxT,       maxT,           maxT,           maxT,           maxT,       maxT,       maxT,       doubleT }      //doubleT
        };

        if (v1.m_numType >= maxT || v2.m_numType >= maxT || v1.m_numType < 0 || v2.m_numType < 0)
            throw NumVariantError();

        return v1.m_numType <= v2.m_numType ? castMatrix[v1.m_numType][v2.m_numType] : castMatrix[v2.m_numType][v1.m_numType];
    }

    NumVariant cast(NumType t) {
        return const_cast<const NumVariant&>(*this).cast(t);
    }


    NumVariant cast(NumType t) const
    {
        switch (t)
        {
        case charT:
            return cast<char>();
        case ucharT:
            return cast<unsigned char>();
        case shortT:
            return cast<short>();
        case ushortT:
            return cast<unsigned short>();
        case longT:
            return cast<long>();
        case ulongT:
            return cast<unsigned long>();
        case longlongT:
            return cast<long long>();
        case ulonglongT:
            return cast<unsigned long long>();
        case intT:
            return cast<int>();
        case uintT:
            return cast<unsigned int>();
        case floatT:
            return cast<float>();
        case doubleT:
            return cast<double>();
        }

        throw NumVariantError();
    }

    template<typename T>
    NumVariant cast() const
    {
        switch (m_numType)
        {
        case charT:
            return static_cast<T>(m_charVal);
        case ucharT:
            return static_cast<T>(m_ucharVal);
        case shortT:
            return static_cast<T>(m_shortVal);
        case ushortT:
            return static_cast<T>(m_ushortVal);
        case longT:
            return static_cast<T>(m_longVal);
        case ulongT:
            return static_cast<T>(m_ulongVal);
        case longlongT:
            return static_cast<T>(m_longlongVal);
        case ulonglongT:
            return static_cast<T>(m_ulonglongVal);
        case intT:
            return static_cast<T>(m_intVal);
        case uintT:
            return static_cast<T>(m_uintVal);
        case floatT:
            return static_cast<T>(m_floatVal);
        case doubleT:
            return static_cast<T>(m_doubleVal);
        }

        throw NumVariantError();
    }

    void copy(const NumVariant &var)
    {
        m_numType = var.m_numType;
        switch (m_numType)
        {
        case charT:
            m_charVal = var.m_charVal;
            break;
        case ucharT:
            m_charVal = var.m_ucharVal;
            break;
        case shortT:
            m_shortVal = var.m_shortVal;
            break;
        case ushortT:
            m_ushortVal = var.m_ushortVal;
            break;
        case longT:
            m_longVal = var.m_longVal;
            break;
        case ulongT:
            m_ulongVal = var.m_ulongVal;
            break;
        case longlongT:
            m_longlongVal = var.m_longlongVal;
            break;
        case ulonglongT:
            m_ulonglongVal = var.m_ulonglongVal;
            break;
        case intT:
            m_intVal = var.m_intVal;
            break;
        case uintT:
            m_uintVal = var.m_uintVal;
            break;
        case floatT:
            m_floatVal = var.m_floatVal;
            break;
        case doubleT:
            m_doubleVal = var.m_doubleVal;
            break;
        }
    }
};


inline bool operator==(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<bool, equal_op>(v1, v2);
}

inline bool operator!=(const NumVariant& v1, const NumVariant& v2)
{
    return !(v1 == v2);
}

inline bool operator<(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<bool, gt_op>(v1, v2);
}

inline bool operator<=(const NumVariant& v1, const NumVariant& v2)
{
    return !(v1 > v2);
}

inline bool operator>(const NumVariant& v1, const NumVariant& v2)
{
    return v2 < v1;
}

inline bool operator>=(const NumVariant& v1, const NumVariant& v2)
{
    return !(v1 < v2);
}

inline bool operator&&(const NumVariant& v1, const NumVariant& v2)
{
    return !(NumVariant::all_op<bool, equal_op >(v1, NumVariant(0)) || NumVariant::all_op<bool, equal_op >(v2, NumVariant(0)));
}

inline bool operator||(const NumVariant& v1, const NumVariant& v2)
{
    return !(NumVariant::all_op<bool, equal_op >(v1, NumVariant(0)) && NumVariant::all_op<bool, equal_op >(v2, NumVariant(0)));
}

inline NumVariant operator+(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<NumVariant, add_op>(v1, v2);
}

inline NumVariant operator-(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<NumVariant, sub_op>(v1, v2);
}

inline NumVariant operator*(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<NumVariant, mul_op>(v1, v2);
}

inline NumVariant operator/(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::all_op<NumVariant, div_op>(v1, v2);
}

inline NumVariant operator%(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::int_op<NumVariant, mod_op>(v1, v2);
}

inline NumVariant operator^(const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::int_op<NumVariant, xor_op >(v1, v2);
}

inline NumVariant operator& (const NumVariant& v1, const NumVariant& v2)
{
    return NumVariant::int_op<NumVariant, and_op >(v1, v2);
}

inline NumVariant operator| (const NumVariant& v1, const NumVariant& v2)
{
    return  NumVariant::int_op<NumVariant, or_op >(v1, v2);
}

inline NumVariant operator<<(const NumVariant& v1, const NumVariant& v2)
{
    NumVariant::NumType  safeArgType = v1.m_numType;
    return  NumVariant::int_op<NumVariant, lshift_op >(v1, v2).cast(safeArgType);
}

inline NumVariant operator>>(const NumVariant& v1, const NumVariant& v2)
{
    NumVariant::NumType  safeArgType = v1.m_numType;
    return  NumVariant::int_op<NumVariant, rshift_op >(v1, v2).cast(safeArgType);
}

inline NumVariant operator~(const NumVariant& v1)
{
    NumVariant  v2(~0ULL);
    v2 = v2.cast(v1.m_numType);
    return NumVariant::int_op<NumVariant, xor_op >(v1, v2);
}

inline bool operator!(const NumVariant& v1)
{
    return NumVariant::all_op<bool, equal_op >(v1, NumVariant(0));
}

inline NumVariant operator++(NumVariant& v1)
{
    v1 = v1 + NumVariant(1).cast(v1.m_numType);
    return v1;
}

inline NumVariant operator--(NumVariant& v1)
{
    v1 = v1 - NumVariant(1).cast(v1.m_numType);
    return v1;
}

inline NumVariant operator++(NumVariant& v1, int)
{
    auto t = v1;
    v1 = v1 + NumVariant(1).cast(v1.m_numType);
    return t;
}

inline NumVariant operator--(NumVariant& v1, int)
{
    auto t = v1;
    v1 = v1 - NumVariant(1).cast(v1.m_numType);
    return t;
}

inline NumVariant operator-(const NumVariant& v1)
{
    return NumVariant(0).cast(v1.m_numType) - v1;
}





class NumConvertable
{
public:

    operator NumVariant() {
        return getValue();
    }

    operator NumVariant() const {
        return getValue();
    }

    virtual NumVariant getValue() const = 0;

};

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace

