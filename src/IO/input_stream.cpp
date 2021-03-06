#include<Basic/process.h>
using namespace std;
using namespace NEDBnamespace;

InputStream::InputStream(){
    __Input = "";
}

string InputStream::read(){
    string input = "";
    getline(cin, input);
    int pos = input.find_first_not_of(' ');
    if(input[pos] == '.'){
        __Input = input;
        return __Input;
    }
    while(true){
        int index = input.find(";");
        if(index == -1){
            __Input = __Input + input;
            printf("    -> ");
            getline(cin, input);
        }
        else{
            __Input = __Input + input.substr(0, index);
            fflush(stdin);
            break;
        }
    }
    __Input = Trim(__Input);
    return __Input;
}

string InputStream::getInput(){
    return __Input;
}

void InputStream::clear_input(){
    __Input = "";
}

