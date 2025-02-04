// LightJSON - Simple, Light Weight, Header Only, C++11 compliant JSON Library.
// Github: https://github.com/rft0/lightjson 
// For those who do not want to include json library heavier than small codebase itself.
// Define JSON_DISABLE_DUMPING to not generate JSON::dump() and related methods if you don't need it. 
// This will help to reduce size of the compiled binary.

#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <type_traits>

// #define JSON_DISABLE_DUMPING

template<typename T, typename Enable = void>
struct JSONTypeTraits;

class JSON {
public:
    enum Type {
        Null,
        Boolean,
        Integer,
        Double,
        String,
        Array,
        Object
    };

    static JSON o(std::initializer_list<std::pair<std::string, JSON>> list) {
        std::map<std::string, JSON> obj;
        for (const auto& pair : list) {
            obj[pair.first] = pair.second;
        }
        return JSON(obj);
    }

    JSON() : type(Null) {}
    JSON(bool b) : type(Boolean), boolean(b) {}
    JSON(int i) : type(Integer), integer(i) {}
    JSON(double d) : type(Double), doubleVal(d) {}
    JSON(const char* s) : type(String), string(new std::string(s)) {}
    JSON(const std::string& s) : type(String), string(new std::string(s)) {}
    JSON(const std::vector<JSON>& a) : type(Array), array(new std::vector<JSON>(a)) {}
    JSON(std::initializer_list<JSON> list) : type(Array), array(new std::vector<JSON>(list)) {}
    JSON(const std::map<std::string, JSON>& obj) : type(Object), object(new std::map<std::string, JSON>(obj)) {}

    ~JSON() {
        clear();
    }

    JSON(const JSON& other) : type(other.type) {
        copy(other);
    }

    Type getType() const { return type; }

    template<typename T>
    T get() const {
        return JSONTypeTraits<T>::get(*this);
    }

#ifndef JSON_DISABLE_DUMPING
    friend std::ostream& operator<<(std::ostream &os, const JSON &json) {
        os << json.dump();
        return os;
    }
#endif

    JSON& operator=(const JSON& other) {
        if (this != &other) {
            clear();
            type = other.type;
            copy(other);
        }
        return *this;
    }

    JSON& operator=(bool b) {
        clear();
        type = Boolean;
        boolean = b;
        return *this;
    }

    JSON& operator=(int i) {
        clear();
        type = Integer;
        integer = i;
        return *this;
    }

    JSON& operator=(double d) {
        clear();
        type = Double;
        doubleVal = d;
        return *this;
    }

    JSON& operator=(const char* s) {
        clear();
        type = String;
        string = new std::string(s);
        return *this;
    }

    JSON& operator=(const std::string& s) {
        clear();
        type = String;
        string = new std::string(s);
        return *this;
    }

    JSON& operator=(const std::vector<JSON>& a) {
        clear();
        type = Array;
        array = new std::vector<JSON>(a);
        return *this;
    }

    JSON& operator=(const std::map<std::string, JSON>& obj) {
        clear();
        type = Object;
        object = new std::map<std::string, JSON>(obj);
        return *this;
    }

    JSON& operator=(std::initializer_list<JSON> list) {
        clear();
        type = Array;
        array = new std::vector<JSON>(list);
        return *this;
    }

    JSON& operator[](const std::string& key) {
        if (type != Object) {
            if (type == Null) {
                type = Object;
                object = new std::map<std::string, JSON>();
            } else {
                std::ostringstream oss;
                oss << "Field \"" << key << "\" is not an object.";
                throw std::runtime_error(oss.str());
            }
        }
        
        return (*object)[key];
    }

    const JSON& operator[](const std::string& key) const {
        if (type != Object) {
            std::ostringstream oss;
            oss << "Field \"" << key << "\" is not an object.";
            throw std::runtime_error(oss.str());
        }

        return object->at(key);
    }

    JSON& operator[](size_t index) {
        if (type != Array)
            throw std::runtime_error("Trying to index a non-array JSON");
        if (index >= array->size())
            throw std::runtime_error("Index out of range");
        return (*array)[index];
    }

    const JSON& operator[](size_t index) const {
        if (type != Array)
            throw std::runtime_error("Trying to index a non-array JSON");
        if (index >= array->size())
            throw std::runtime_error("Index out of range");
        return (*array)[index];
    }

#ifndef JSON_DISABLE_DUMPING
    std::string dump(int indent = 4) const {
        std::ostringstream oss;
        dumpValue(*this, oss, 0, indent);
        return oss.str();
    }
#endif

private:
    template<typename T, typename Enable>
    friend struct JSONTypeTraits;

    Type type;
    union {
        bool boolean;
        int integer;
        double doubleVal;
        std::string* string;
        std::vector<JSON>* array;
        std::map<std::string, JSON>* object;
    };

    void clear() {
        switch (type) {
            case String: delete string; break;
            case Array: delete array; break;
            case Object: delete object; break;
            default: break;
        }

        type = Null;
    }

    void copy(const JSON& other) {
        switch (other.type) {
            case Boolean: boolean = other.boolean; break;
            case Integer: integer = other.integer; break;
            case Double: doubleVal = other.doubleVal; break;
            case String: string = new std::string(*other.string); break;
            case Array: array = new std::vector<JSON>(*other.array); break;
            case Object: object = new std::map<std::string, JSON>(*other.object); break;
            default: break;
        }
    }

#ifndef JSON_DISABLE_DUMPING
    void dumpValue(const JSON& value, std::ostringstream& oss, int level, int indent) const {
        switch (value.getType()) {
            case Null: oss << "null"; break;
            case Boolean: oss << (value.boolean ? "true" : "false"); break;
            case Integer: oss << value.integer; break;
            case Double: oss << value.doubleVal; break;
            case String: dumpString(*value.string, oss); break;
            case Array: dumpArray(*value.array, oss, level, indent); break;
            case Object: dumpObject(*value.object, oss, level, indent); break;
        }
    }

    void dumpString(const std::string& str, std::ostringstream& oss) const {
        oss << '"';
        for (char ch : str) {
            switch (ch) {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (ch < 32 || ch == 127) {
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(ch);
                    } else {
                        oss << ch;
                    }
                    break;
            }
        }
        oss << '"';
    }

    void dumpArray(const std::vector<JSON>& arr, std::ostringstream& oss, int level, int indent) const {
        bool hasComplexChildren = false;
        for (const auto& item : arr) {
            if (item.getType() == JSON::Object || item.getType() == JSON::Array) {
                hasComplexChildren = true;
                break;
            }
        }

        const bool prettyPrint = hasComplexChildren && (indent > 0);
        oss << '[';

        if (prettyPrint)
            oss << '\n';

        for (size_t i = 0; i < arr.size(); ++i) {
            if (prettyPrint) {
                oss << std::string(level + indent, ' ');
            }

            dumpValue(arr[i], oss, level + indent, indent);

            if (i < arr.size() - 1) {
                oss << ',';
                if (!prettyPrint) oss << ' ';
            }

            if (prettyPrint)
                oss << '\n';
        }

        if (prettyPrint)
            oss << std::string(level, ' ');

        oss << ']';
    }

    void dumpObject(const std::map<std::string, JSON>& obj, std::ostringstream& oss, int level, int indent) const {
        oss << '{';
        if (indent > 0) oss << '\n';
        size_t i = 0;
        for (const auto& pair : obj) {
            const std::string& key = pair.first;
            const JSON& value = pair.second;

            if (indent > 0) oss << std::string(level + indent, ' ');
            dumpString(key, oss);
            oss << ':';
            if (indent > 0) oss << ' ';
            dumpValue(value, oss, level + indent, indent);
            if (++i < obj.size()) oss << ',';
            if (indent > 0) oss << '\n';
        }
        if (indent > 0) oss << std::string(level, ' ');
        oss << '}';
    }
#endif
};

template<>
struct JSONTypeTraits<bool> {
  static bool get(const JSON& json) {
    if (json.getType() != JSON::Boolean)
      throw std::runtime_error("Not a boolean");
    return json.boolean;
  }
};

// Integer 
template<>
struct JSONTypeTraits<int> {
  static int get(const JSON& json) {
    if (json.getType() != JSON::Integer)
      throw std::runtime_error("Not an integer");
    return json.integer;
  }
};

// Double
template<>
struct JSONTypeTraits<double> {
  static double get(const JSON& json) {
    if (json.getType() != JSON::Double)
      throw std::runtime_error("Not a double");
    return json.doubleVal;
  }
};

// String
template<>
struct JSONTypeTraits<std::string> {
  static std::string get(const JSON& json) {
    if (json.getType() != JSON::String)
      throw std::runtime_error("Not a string");
    return *json.string;
  }
};

// Arrays (homogeneous)
template<typename T>
struct JSONTypeTraits<std::vector<T>> {
  static std::vector<T> get(const JSON& json) {
    if (json.getType() != JSON::Array)
      throw std::runtime_error("Not an array");
      
    std::vector<T> result;
    const auto& arr = *json.array;
    for (const auto& item : arr) {
      result.push_back(item.get<T>());
    }
    return result;
  }
};

// Object (map)
template<>
struct JSONTypeTraits<std::map<std::string, JSON>> {
  static std::map<std::string, JSON> get(const JSON& json) {
    if (json.getType() != JSON::Object)
      throw std::runtime_error("Not an object");
    return *json.object;
  }
};

// SFINAE for arithmetic types
template<typename T>
struct JSONTypeTraits<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
  static T get(const JSON& json) {
    switch (json.getType()) {
      case JSON::Integer: return static_cast<T>(json.integer);
      case JSON::Double: return static_cast<T>(json.doubleVal);
      default: throw std::runtime_error("Not a numeric type");
    }
  }
};

class JSONParser {
public:
    JSONParser(const std::string& data) : data(data), pos(0), line(1), col(1) {}
    JSONParser(std::ifstream& f)
    : data(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>())), pos(0), line(1), col(1) {}

    static JSON parse(const std::string& data) {
        JSONParser parser(data);
        return parser.parse();
    }

    static JSON parse(std::ifstream& f) {
        JSONParser parser(f);
        return parser.parse();
    }


private:
    std::string data;
    int pos;
    int line;
    int col;

    JSON parse() {
        skipWhitespace();
        return parseValue();
    }

    void advance(int count = 1) {
        for (int i = 0; i < count; ++i) {
            if (data[pos] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            pos++;
        }
    }

    void skipWhitespace() {
        while (data[pos] && isspace(data[pos])) {
            advance();
        }
    }

    [[noreturn]] void throwError(const std::string& message) const {
        std::ostringstream oss;

        oss << message << " at line " << line << ", column " << col;
        throw std::runtime_error(oss.str());
    }

    JSON parseValue() {
        skipWhitespace();
        char ch = data[pos];
        if (ch == '{')
            return parseObject();
        else if (ch == '[')
            return parseArray();
        else if (ch == '"')
            return parseString();
        else if (ch == 't' || ch == 'f')
            return parseBoolean();
        else if (ch == 'n')
            return parseNull();
        else if (ch == '-' || (ch >= '0' && ch <= '9'))
            return parseNumber();
        else
            throwError("Unexpected character in JSON");
    }

    JSON parseObject() {
        advance();
        std::map<std::string, JSON> obj;
        skipWhitespace();
        while (data[pos] != '}') {
            std::string key = parseString().get<std::string>();
            skipWhitespace();
            if (data[pos] != ':') {
                throwError("Expected ':' in JSON object");
            }
            advance();
            JSON value = parseValue();
            obj[key] = value;
            skipWhitespace();
            if (data[pos] == ',') {
                advance();
                skipWhitespace();
            }
        }
        advance();
        return JSON(obj);
    }

    JSON parseArray() {
        advance();
        std::vector<JSON> arr;
        skipWhitespace();
        while (data[pos] != ']') {
            arr.push_back(parseValue());
            skipWhitespace();
            if (data[pos] == ',') {
                advance();
                skipWhitespace();
            }
        }
        advance();
        return JSON(arr);
    }

    JSON parseString() {
        advance();
        std::string str;
        while (data[pos] != '"') {
            if (data[pos] == '\\') {
                advance();
                switch (data[pos]) {
                    case '\\': str += '\\'; break;
                    case '"': str += '"'; break;
                    case '/': str += '/'; break;
                    case 'b': str += '\b'; break;
                    case 'f': str += '\f'; break;
                    case 'n': str += '\n'; break;
                    case 'r': str += '\r'; break;
                    case 't': str += '\t'; break;
                    default: throwError("Invalid escape character in string");
                }
                advance();
            } else {
                str += data[pos];
                advance();
            }
        }
        advance();
        return JSON(str);
    }

    JSON parseBoolean() {
        if (strncmp(&data[pos], "true", 4) == 0) {
            advance(4);
            return JSON(true);
        } else if (strncmp(&data[pos], "false", 5) == 0) {
            advance(5);
            return JSON(false);
        } else {
            throwError("Unexpected boolean value in JSON");
        }
    }

    JSON parseNull() {
        if (strncmp(&data[pos], "null", 4) == 0) {
            advance(4);
            return JSON();
        } else {
            throwError("Unexpected null value in JSON");
        }
    }

    JSON parseNumber() {
        size_t start = pos;
        if (data[pos] == '-') {
            advance();
        }
        while (data[pos] >= '0' && data[pos] <= '9') {
            advance();
        }
        if (data[pos] == '.') {
            advance();
            while (data[pos] >= '0' && data[pos] <= '9') {
                advance();
            }
            return JSON(std::stod(std::string(&data[start], pos - start)));
        } else {
            return JSON(std::stoi(std::string(&data[start], pos - start)));
        }
    }
};

#endif
