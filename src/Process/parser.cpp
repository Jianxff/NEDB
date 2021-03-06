#include<Basic/process.h>
#include<main.h>
using namespace std;
using namespace NEDBnamespace;

Parser::Parser(){
    statement_ = "";
    object_ = "";
    condition_ = "";
    value_ = "";
    operation_ = UNDEFINED;
    command_ = __UNKNOWN;
}

void Parser::flush(){
    statement_ = "";
    object_ = "";
    condition_ = "";
    value_ = "";
    operation_ = UNDEFINED;
    command_ = __UNKNOWN;
}

void Parser::i_analyse(string input){
    try{
        command_ = __OPERATION;
        operation_ = operate_type(input);
        statement_ = Trim(input);
        deconstruct();
    }
    catch(NEexception& e){
        throw e;
    }

}

void Parser::analyse(string input){
    try{
        if(input[0] != '.'){
            command_ = __OPERATION;
            operation_ = operate_type(input);
            statement_ = Trim(input);
            deconstruct();
        }
        else if(input == ".exit"){
            command_ = __EXIT;
        }
        else if(input == ".help"){
            command_ = __HELP;
        }
        else if(input == ".dir"){
            command_ = __SHOWDIR;
        }
        else if(input == ".dirinit"){
            command_ = __DIRINIT;
        }
        else if(input == ".mountall"){
            command_ = __MOUNTALL;
        }
        else if(input.substr(0, input.find(" ")).compare(".debug") == 0){
            command_ = __DEBUGSET;
            int index = input.find(" ");
            if(Trim(input)==".debug"){
                statement_ ="";
            }else{
                statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
            }
        }
        else if(input.substr(0, input.find(" ")).compare(".setdir") == 0){
            command_ = __SETDIR;
            int index = input.find(" ");
            statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
        }
        else if(input.substr(0, input.find(" ")).compare(".mount") == 0){
            command_ = __MOUNT;
            int index = input.find(" ");
            statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
        }
        else if(input.substr(0, input.find(" ")).compare(".unmount") == 0){
            command_ = __UNMOUNT;
            int index = input.find(" ");
            statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
        }
        else if(input.substr(0, input.find(" ")).compare(".open") == 0){
            command_ = __OPEN;
            int index = input.find(" ");
            statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
        }
        else if(input.substr(0, input.find(" ")).compare(".setsize") == 0){
            command_ = __SETPAGESIZE;
            int index = input.find(" ");
            statement_ = Trim(input.substr(index + 1, input.length() - index - 1));
        }
        else if(input == ".size"){
            command_ = __SHOWPAGESIZE;
        }
        else{
            command_ = __UNKNOWN;
            statement_ = input;
        }
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw SYSTEM_ERROR;
    }

}

void Parser::deconstruct(){
    try{
        switch(operation_){
            case CREATE_TABLE:
                parser_create_table();
                break;
            case INSERT_VALUES:
                parser_insert_values();
                break;
            case DELETE_VALUES:
                parser_delete_values();
                break;
            case SELECT_VALUES:
                parser_select_values();
                break;
            case UPDATE_VALUES:
                parser_update_values();
                break;
            case DESCRIBE_TABLE:
                parser_describe_table();
                break;
            case DROP_TABLE:
                parser_drop_table();
                break;
            case SELECT_TABLES:
                break;
            default:
                throw SQL_FORM_ERROR;
        }
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw SYSTEM_ERROR;
    }

}

void Parser::parser_create_table(){
    regex layout("create table (.+)\\((.+)\\)");
    smatch result;
    if(regex_match(statement_, result, layout)){
        auto it = result.begin();
        object_ = Trim(*++it);
        value_ = Trim(*++it);
    }
    else throw SQL_FORM_ERROR;
}

void Parser::parser_insert_values(){
    regex layout("insert into (.+) \\((.+)\\) values \\((.+)\\)");
    regex layout2("insert into (.+) values \\((.+)\\)");
    smatch result;
    if(regex_match(statement_, result, layout)){
        auto it = result.begin();
        object_ = Trim(*++it);
        condition_ = Trim(*++it);
        value_ = Trim(*++it);
    }
    else if(regex_match(statement_, result, layout2)){
        auto it = result.begin();
        object_ = Trim(*++it);
        value_ = Trim(*++it);
    }
    else throw SQL_FORM_ERROR;
}

void Parser::parser_delete_values(){
    regex layout("delete from (.+) where (.+)");
    //regex layout2("DELETE FROM (.+)");
    smatch result;
    if(regex_match(statement_, result, layout)){
        auto it = result.begin();
        object_ = Trim(*++it);
        condition_ = Trim(*++it);
    }
    else{
        condition_ = "";
        object_ = Trim(statement_.substr(12, statement_.length() - 12));
    }
}

void Parser::parser_select_values(){
    regex layout("select (.+) from (.+) where (.+)");
    regex layout2("select (.+) from (.+)");
    smatch result;
    if(regex_match(statement_, result, layout)){
        auto it = result.begin();
        value_ = Trim(*++it);
        object_ = Trim(*++it);
        condition_ = Trim(*++it);
    }
    else if(regex_match(statement_, result, layout2)){
        auto it = result.begin();
        value_ = Trim(*++it);
        object_ = Trim(*++it);
    }
    else throw SQL_FORM_ERROR;

}

void Parser::parser_update_values(){
    regex layout("update (.+) set (.+) where (.+)");
    regex layout2("update (.+) set (.+)");
    smatch result;
    if(regex_match(statement_, result, layout)){
        auto it = result.begin();
        object_ = Trim(*++it);
        value_ = Trim(*++it);
        condition_ = Trim(*++it);
    }
    else if(regex_match(statement_, result, layout2)){
        auto it = result.begin();
        object_ = Trim(*++it);
        value_ = Trim(*++it);
    }
    else throw SQL_FORM_ERROR;
}

void Parser::parser_describe_table(){
    object_ = Trim(statement_.substr(15, statement_.length() - 15));
    if(object_.length() == 0) throw SQL_FORM_ERROR;
}

void Parser::parser_drop_table(){
    object_ = Trim(statement_.substr(11, statement_.length() - 11));
    if(object_.length() == 0) throw SQL_FORM_ERROR;
}


OPERATION  Parser::operate_type(string input){
    if(input.compare(0, 13, "create table ") == 0)  return CREATE_TABLE;
    if(input.compare(0, 12, "insert into ") == 0)   return INSERT_VALUES;
    if(input.compare(0, 12, "delete from ") == 0)   return DELETE_VALUES;
    if(input.compare("select tables") == 0)         return SELECT_TABLES;
    if(input.compare(0, 7, "select ") == 0)         return SELECT_VALUES;
    if(input.compare(0, 7, "update ") == 0)         return UPDATE_VALUES;
    if(input.compare(0, 11, "drop table ") == 0)    return DROP_TABLE;
    if(input.compare(0, 15, "describe table ") == 0)return DESCRIBE_TABLE;
    return UNDEFINED;
}

string Parser::getStatment(){
    return statement_;
}

COMMAND Parser::getCommand(){
    return command_;
}

OPERATION Parser::getOperate(){
    return operation_;
}

void Parser::setCondition(string input){
    condition_ = Trim(input);
}

void Parser::setValue(string input){
    value_ = Trim(input);
}

void Parser::setObject(string input){
    object_ = Trim(input);
}

void Parser::setStatement(string input){
    statement_ = Trim(input);
}

void Parser::setOperation(OPERATION input){
    command_ = __OPERATION;
    operation_ = input;
}