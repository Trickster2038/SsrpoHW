#include <iostream>
#include <string>
#include <cassert>
#include <regex>

#include "collector.h"

#include <sstream>

using namespace std;

const size_t MAX_FIO_LENGTH = 50;
const uint MIN_AGE = 21;
const uint MAX_AGE = 120;
static const auto r = regex(R"([A-Z][a-z]{2,} [A-Z].[A-Z].)");

enum Fraction
{
    FRACTION_EDRO,
    FRACTION_YABLOKO,
    FRACTION_LDPR,
    FRACTION_NOVYE_LUDI,
    FRACTION_KPRF
};

class Candidate //: public ICollectable
{
    string _fio;
    uint _age;
    float _income;
    Fraction _fraction;
    uint _voices;

protected:
    bool invariant() const
    {
        return _fio.size() <= MAX_FIO_LENGTH && _age <= MAX_AGE && _age >= MIN_AGE && !_fio.empty() && _income >= 0.0;
    }

public:
    Candidate() = delete;
    Candidate(const Candidate &p) = delete;
    Candidate &operator=(const Candidate &p) = delete;

    Candidate(const string &fio, uint age, float income, Fraction fraction, uint voices) : _fio(fio), _age(age), _income(income), _fraction(fraction), _voices(voices)
    {
        assert(invariant());
    }
};

int main(){
    Candidate a("Urov S.G.", 38, 150202, FRACTION_EDRO, 8021);
    return 0;
}