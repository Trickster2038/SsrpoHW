#include <string>
#include <vector>
#include <cassert>
#include <memory>

#include <fstream>

template<typename T>
T readNumber(std::istream& is)
{
    T result;
    is.read(reinterpret_cast<char *>(&result), sizeof(result));
    return result;
}

std::string readString(std::istream& is, size_t max_string_length)
{
    uint16_t len = readNumber<uint16_t>(is);

    assert(len <= max_string_length);

    char b[max_string_length+1];

    if (len > 0)
        is.read(b, len);

    b[len] = 0;

    return std::string(b);
}

template<typename T>
void writeNumber(std::ostream& os, T i)
{
    os.write(reinterpret_cast<char *>(&i),sizeof(i));
}

void writeString(std::ostream& os, const std::string& s)
{
    uint16_t len = s.length();

    writeNumber(os, len);

    os.write(s.c_str(), len);
}

class ICollectable
{
public:
    virtual ~ICollectable() = default;

    virtual bool write(std::ostream& os) = 0;
};

class ACollector
{
    std::vector<std::shared_ptr<ICollectable>> _items;
    std::vector<bool>                          _removed_signs;
    size_t                                     _removed_count = 0;

    bool    invariant() const
    {
        return _items.size() == _removed_signs.size() && _removed_count <= _items.size();
    }

public:
    virtual ~ACollector() = default;

    virtual std::shared_ptr<ICollectable> read(std::istream& is) = 0;

    size_t getSize() const { return _items.size(); }

    std::shared_ptr<ICollectable> getItem(size_t index) const
    {
        assert(index < _items.size());

        return _items[index];
    }

    bool isRemoved(size_t index) const
    {
        assert(index < _removed_signs.size());

        return _removed_signs[index];
    }

    void addItem(std::shared_ptr<ICollectable> item)
    {
        _items.emplace_back(item);
        _removed_signs.emplace_back(false);
    }

    void removeItem(size_t index)
    {
        assert(index < _items.size());
        assert(index < _removed_signs.size());

        if (!_removed_signs[index])
        {
            _removed_signs[index] = true;
            _removed_count ++;
        }
    }

    void updateItem(size_t index, const std::shared_ptr<ICollectable> & item)
    {
        assert(index < _items.size());

        _items[index] = item;
    }

    void clean()
    {
        _items.clear();
        _removed_signs.clear();
        _removed_count = 0;
    }

    bool loadCollection(const std::string file_name)
    {
        std::ifstream ifs (file_name, std::ios_base::binary);

        if (!ifs)
            return false;

        size_t count = readNumber<size_t>(ifs);

        _items.reserve(count);

        for(size_t i=0; i < count; ++i)
            addItem(read(ifs));

        assert(invariant());

        return ifs.good();
    }

    bool saveCollection(const std::string file_name) const
    {
        assert(invariant());

        std::ofstream ofs (file_name, std::ios_base::binary);

        if (!ofs)
            return false;

        size_t count = _items.size() - _removed_count;

        writeNumber(ofs, count);

        for(size_t i=0; i < _items.size(); ++i)
            if (!_removed_signs[i])
                _items[i]->write(ofs);

        return ofs.good();
    }

};
