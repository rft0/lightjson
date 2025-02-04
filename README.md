# LightJSON
LightJSON is a Simple, Light Weight, Header Only, C++11 compliant JSON Library. It is designed for projects which need a minimalistic JSON solution without the overhead of larger libraries.

## Usage
### Including Header
Just include header file to use it.
```c
#include "jspn.hpp"
```
If you only want to use reading contents of json without dumping / manipulaitng it you can disable those features. (which may help to reduce binary size)
```c
#define JSON_DISABLE_DUMPING
#include "jspn.hpp"
```
### Parsing JSON
For starters you can pass either a raw pointer, std::string or a std::ifstream to parser.
```cpp
const char* data = R"("name": "John", "lastName": "Doe", "age": 30, "something": [1, 2.5, {"three": "four"}])";
JSON json = JSONParser::parse(data)

// or

std::ifstream f("test.json");
JSON json = JSONParser::parse(data)
```
### Reading Values
Reading simple fields.
```cpp
const char* data = R"("name": "John", "lastName": "Doe", "age": 30, "something": [1, 2.5, {"dummy": "dummyvalue"}])";
JSON json = JSONParser::parse(data)

std::string name = json["name"].get<std::string>();       // John
int age = json["age"].get<int>();                         // 30
int someFloat = json["something"][1]                      // 2.5
std::string dummy = json["something"][2]["dummy"]         // dummyvalue
```

Reading arrays.
```cpp
const char* data = R"("intArray": [1, 2, 3, 4, 5], "complexArray": ["something", 1, { "dummy": 2 }] )";
JSON json = JSONParser::parse(data);

// Reading an array when all members are same type
std::vector<int> intArray = json["intArray"].get<std::vector<int>>(); // [1, 2, 3, 4, 5]

// Reading an array when it contains members with different types.
std::vector<JSON> complexArray = json["complexArray"].get<std::vector<JSON>>();
complexArray[0].get<std::string>();     // something
complexArray[1].get<int>();             // 1
complexArray[2]["dummy"].get<int>();    // 2

// or we can read value of field "dummy" like this.
JSON anotherRoot = complexArray[2];
anotherRoot["dummy"].get<int>()         // 2
```

### Creating / Changing / Dumping Values
Everytihng discussed in this section is disabled if `JSON_DISABLE_DUMPING` is defined.
```c
JSON root;
root["something"] = "another thing";
root["exampleArray"] = {1, 2, 3};
root["anotherObject]["smt"] = "value"; 

// Dumping json
printf("%s", root.dump(4)); // (first argument means indentation space count, default: 4)

// or we can dump json root directly with using overloaded << operator.
std::cout << root << std::endl;

// another way to create json from root with initializer lists
JSON root = JSON::o({
    {"something", "another thing"},
    {"exampleArray", {1, 2, 3}},
    {"anotherObject", { "smt", "value" }}
});
```



