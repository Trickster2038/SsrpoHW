#include <iostream>
#include <string>
#include <cassert>
#include <regex>

#include "collector.h"

#include <sstream>

using namespace std;

const size_t MAX_FIO_LENGTH = 50;
const size_t MAX_FRACTION_LENGTH = 20;
const uint MIN_AGE = 21;
const uint MAX_AGE = 120;
static const auto FIO_REGEX = regex(R"([A-Z][a-z]{2,}_[A-Z].[A-Z].)");

enum Fraction
{
    FRACTION_EDRO,
    FRACTION_YABLOKO,
    FRACTION_LDPR,
    FRACTION_NOVYE_LUDI,
    FRACTION_KPRF,
    FRACTION_UNKNOWN
};

class Converter
{
private:
    static inline map<Fraction, string> fraction_to_string = {{FRACTION_EDRO, "EDRO"},
                                                              {FRACTION_YABLOKO, "YABLOKO"},
                                                              {FRACTION_LDPR, "LDPR"},
                                                              {FRACTION_NOVYE_LUDI, "NOVIE LUDI"},
                                                              {FRACTION_KPRF, "KPRF"}};

    static inline map<string, Fraction> string_to_fraction = {{"EDRO", FRACTION_EDRO},
                                                              {"YABLOKO", FRACTION_YABLOKO},
                                                              {"LDPR", FRACTION_LDPR},
                                                              {"NOVIE LUDI", FRACTION_NOVYE_LUDI},
                                                              {"KPRF", FRACTION_KPRF}};

public:
    static const string &toString(Fraction fraction_code)
    {
        return fraction_to_string.find(fraction_code)->second;
    }
    static const Fraction toFraction(const string &fraction)
    {
        auto it = string_to_fraction.find(fraction);
        return it == string_to_fraction.end() ? FRACTION_UNKNOWN : it->second;
    }
};

class Candidate : public ICollectable
{
    string _fio;
    uint _age;
    uint _income;
    Fraction _fraction;
    uint _voices;

protected:
    bool invariant() const
    {
        smatch m;
        return _fio.size() <= MAX_FIO_LENGTH && _age <= MAX_AGE && _age >= MIN_AGE && !_fio.empty();
    }

public:
    Candidate() = delete;
    Candidate(const Candidate &p) = delete;
    Candidate &operator=(const Candidate &p) = delete;
    Candidate(const string &fio, uint age, uint income, Fraction fraction, uint voices) : _fio(fio), _age(age), _income(income), _fraction(fraction), _voices(voices)
    {
        assert(invariant());
    }

    Candidate(const char *fio, long unsigned int age, long unsigned int income, const char *fraction, long unsigned int voices)
    {
        _fio = string(fio);
        _age = age;
        _income = income;
        _fraction = Converter::toFraction(string(fraction));
        _voices = voices;
        assert(invariant());
    }

    const string &getFio() const { return _fio; }
    const uint getAge() const { return _age; }
    const uint getIncome() const { return _income; }
    const Fraction getFraction() const { return _fraction; }
    const uint getVoices() const { return _voices; }

    virtual bool write(ostream &os) override
    {
        writeString(os, _fio);
        writeNumber(os, _age);
        writeNumber(os, _income);
        writeString(os, Converter::toString(_fraction));
        writeNumber(os, _voices);

        return os.good();
    }
};

class ItemCollector : public ACollector
{
public:
    virtual shared_ptr<ICollectable> read(istream &is) override
    {
        string fio = readString(is, MAX_FIO_LENGTH);
        uint age = readNumber<uint>(is);
        uint income = readNumber<uint>(is);
        Fraction fraction = Converter::toFraction(readString(is, MAX_FRACTION_LENGTH));
        uint voices = readNumber<uint>(is);

        return make_shared<Candidate>(fio, age, income, fraction, voices);
    }
};

bool performCommand(const vector<string> &args, ItemCollector &col)
{
    if (args.empty())
        return false;

    if (args[0] == "l" || args[0] == "load")
    {
        string filename = (args.size() == 1) ? "hw.data" : args[1];

        if (!col.loadCollection(filename))
        {
            cerr << "Ошибка при загрузке файла '" << filename << "'" << endl;
            return false;
        }

        return true;
    }

    if (args[0] == "s" || args[0] == "save")
    {
        string filename = (args.size() == 1) ? "hw.data" : args[1];

        if (!col.saveCollection(filename))
        {
            cerr << "Ошибка при сохранении файла '" << filename << "'" << endl;
            return false;
        }

        return true;
    }

    if (args[0] == "c" || args[0] == "clean")
    {
        if (args.size() != 1)
        {
            cerr << "Некорректное количество аргументов команды clean" << endl;
            return false;
        }

        col.clean();

        return true;
    }

    if (args[0] == "a" || args[0] == "add")
    {
        if (args.size() != 6)
        {
            cerr << "Некорректное количество аргументов команды add" << endl;
            return false;
        }
        col.addItem(make_shared<Candidate>(args[1].c_str(), stoul(args[2]), stoul(args[3]), args[4].c_str(), stoul(args[5])));
        return true;
    }

    if (args[0] == "r" || args[0] == "remove")
    {
        if (args.size() != 2)
        {
            cerr << "Некорректное количество аргументов команды remove" << endl;
            return false;
        }

        col.removeItem(stoul(args[1]));
        return true;
    }

    if (args[0] == "u" || args[0] == "update")
    {
        if (args.size() != 7)
        {
            cerr << "Некорректное количество аргументов команды update" << endl;
            return false;
        }
        col.updateItem(stoul(args[1]), make_shared<Candidate>(args[2].c_str(), stoul(args[3]), stoul(args[4]), args[5].c_str(), stoul(args[6])));
        return true;
    }

    if (args[0] == "v" || args[0] == "view")
    {
        if (args.size() != 1)
        {
            cerr << "Некорректное количество аргументов команды view" << endl;
            return false;
        }

        size_t count = 0;
        for (size_t i = 0; i < col.getSize(); ++i)
        {
            const Candidate &item = static_cast<Candidate &>(*col.getItem(i));

            if (!col.isRemoved(i))
            {
                cout << "[" << i << "] "
                     << item.getFio() << " "
                     << item.getAge() << " "
                     << item.getIncome() << " "
                     << Converter::toString(item.getFraction()) << " "
                     << item.getVoices() << endl;
                count++;
            }
        }

        cout << "Количество элементов в коллекции: " << count << endl;

        return true;
    }

    cerr << "Недопустимая команда '" << args[0] << "'" << endl;
    return false;
}

int main()
{
    ItemCollector col;

    for (string line; getline(cin, line);)
    {
        if (line.empty())
            break;

        istringstream iss(line);
        vector<string> args;

        for (string str; iss.good();)
        {
            iss >> str;
            args.emplace_back(str);
        }

        if (!performCommand(args, col))
            return 1;
    }

    cout << "Выполнение завершено успешно" << endl;
    return 0;
}