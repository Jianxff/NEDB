#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <stdarg.h>
#include <stdio.h>
#include <regex>
#include <iomanip>
#include <unistd.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <Utils/BasicUtils.hpp>
#include <Utils/BalanceTree.h>
#include <Interface/NEDB.h>

namespace NEDBnamespace{
    using namespace NEDBSTD;
    using namespace UTILSTD;
    using namespace BTREESTD;

    /* Basic Sign */
    #define TEXT_LENGTH         32
    #define LONGTEXT_LENGTH     255
    #define PAGE_HEAD_SIZE      40 
    #define DATA_OFFSET         2

/* Lock Status */
    #define SIG_BLOCK           0
    #define SIG_RUN             1
    #define SIG_FREE            2

    extern int SIG_DEBUG;
    extern int SIG_WAIT_MSECS;
    extern int SIG_CHECK_TIMES;
    extern int MAX_TABLES;
    extern int DEFAULT_PAGE_SIZE;

    extern int TIME_FLAG;

    /* File Format */
    #define FRAM_SUFFIX      ".nef"
    #define DATA_SUFFIX      ".ned"
    #define RELATIVE_PATH    0
    #define FULL_PATH        1
    #define FILE_DROP        0
    #define FILE_UNMOUNT     1

    extern std::string DEFAULT_DIR;

    /**
     * @brief NEDB data type
     * 
     */
    enum DATA_TYPE: __uint16_t{
        __INT,
        __INT64,
        __REAL,
        __TEXT,
        __LONGTEXT
    };
    static std::string const kTypeName[] = {"int","int64","real","text","longtext"};
    static int const kTypeSize[] = {4,8,8,32,255};

    /**
     * @brief NEDB Index Union
     * 
     */
    union INDEX{
        int i_index;
        long long l_index;
        double d_index;
        char t_index[32];
    };

    /**
     * @brief NEDB command sign
     * 
     */
    enum COMMAND: __uint16_t{
        __UNKNOWN,
        __EXIT,
        __HELP,
        __OPERATION,
        __SETDIR,
        __SHOWDIR,
        __DIRINIT,
        __MOUNT,
        __UNMOUNT,
        __MOUNTALL,
        __OPEN,
        __SETPAGESIZE,
        __SHOWPAGESIZE,
        __DEBUGSET
    };

    /**
     * @brief NEDB operation sign
     * 
     */
    enum OPERATION: __uint16_t{
        CREATE_TABLE,
        INSERT_VALUES,
        DELETE_VALUES,
        UPDATE_VALUES,
        SELECT_VALUES,
        SELECT_TABLES,
        DESCRIBE_TABLE,
        DROP_TABLE,
        UNDEFINED
    };

    /* <---------------------------- Basic Class -----------------------------> */
    /*
        [C]Table - ?????????
        [C]Page - ?????????
        [C]Row - ?????????
        [C]Index - ????????????
        [C]Executor - ???????????????
        [C]Praser - ???????????????
        [C]InputStream - ???????????????
        [C]File - ?????????????????????
        [C]NEexception - ?????????????????????
        [C]BalanceTree - B+???
        [C]Implement - ?????????

        InputStream -> Praser(input) -> Executor(command)
        -> Table(command) -> Page(command) ->Row(command)
                            |
                            V
                    NEexception(error)
    */
    /*
        DATA_TYPE  @  ??????????????????????????????
        COMMAND  @  ???????????????
        OPERATION  @  ??????????????????
        Split(string str,char c,int& length)  @  ?????????????????????
        Table  @  ???????????? 
        Page @  ???????????????
        Row  @  ????????????(????????????)  

    */
    class InputStream;
    class Parser;
    class Executor;
    class Table;
    class Page;
    class Row;
    class Analyzer;
    class File;
    class Implement;
    class Memorizer;
    class Index;

    /* Console Function */
    void __START__();
    void __MESSAGE__();
    void __HELP__();

    /* Other Funtion */
    int __DEBUG_SET__(std::string);
    void parm_check(std::string str,NEDBnamespace::DATA_TYPE type);
    int StatusCheck(int& state,int level,int times);


}

using namespace NEDBnamespace;
class NEDBSTD::DataBase{
    private:
        std::string     __SrcDir__;
        Table**         __Tables__;
        int             __Cursor__;
        int             __PageSize__;
        void            database_init();
    public:
        DataBase(std::string dir);              //????????????

        /* DataBase Lock */
        int     __ProcStatus__;         //DataBase Lock
        int&    Status();
        void    SetStatus(int state = SIG_BLOCK);

        /* Dir Operation */
        std::string getDir();                   //??????????????????
        int     setDir(std::string dir);        //???????????? 
        int     dirInit();                      //???????????????

        /* PageSize Operation */
        int     getDefaultPageSize();
        int     setDefaultPageSize(int);        //????????????????????????????????????

        /* File Section */
        int     openall(int& num);                      //?????????????????????
        int     open(std::string name, int mode = FULL_PATH);     //???????????????
        void     dropTable(std::string name, int mode = FILE_UNMOUNT);

        /* Execute Section */
        int     exec(std::string sql, int& num, std::string& res);          //??????sql??????

        /* Query Section */
        int     create_table(std::string table, std::string value);
        int     insert_into(std::string table, std::string field, std::string value);
        int     delete_from(std::string table, std::string condition, int& num);
        int     update_set(std::string table, std::string set_val, std::string conditions, int& num);
        int     select_from(std::string table, std::string field, std::string conditions, int& num, std::string& res);
        int     describe_table(std::string table, std::string& res);
        int     select_tables(int& num, std::string& res);

        /* Basic Section */
        int     close();                        //???????????????
        Table*  getTable(int);
        Table*  getTable(std::string name);
        void    addTable(Table*);
        int     getCursor();

    };

#endif