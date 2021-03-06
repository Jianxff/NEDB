#include<Basic/storage.h>
#include<main.h>
using namespace std;
using namespace NEDBnamespace;

Memorizer::Memorizer(Table* table){
    table_ = table;
}

/**
 * .nef //表架构文件
 * .ned //数据文件
 */

void Memorizer::TableStore(){
    try{
        if(table_ == NULL){
            throw TABLE_NOT_FOUND;
        }
        //检测空文件/创建文件
        string filePath = table_->filePath_+ FRAM_SUFFIX;
        FILE* fp;
        if((fp = fopen(filePath.c_str(), "r")) != NULL){
            throw TABLE_EXIST;
        }
        if((fp = fopen(filePath.c_str(), "w")) == NULL){
            throw DIR_ERROR;
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(&table_->page_size_, 2, 1, fp);
        fwrite(table_->table_name_, 32, 1, fp);
        fwrite(&table_->parm_num_, 2, 1, fp);
        fwrite(table_->parm_types_, 2, table_->parm_num_, fp);
        fwrite(table_->parm_names_, 32, table_->parm_num_, fp);
        fwrite(&table_->prim_key_, 2, 1, fp);
        fwrite(&table_->row_take_up_, 2, 1, fp);
        fwrite(&table_->max_rows_per_page_, 2, 1, fp);
        fclose(fp);
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw FILE_DAMAGED;
    }

}

Table* Memorizer::TableLoad(DataBase* db, string name,int mode){
    try{
        string filePath = name,dataPath = name;
        if(mode == RELATIVE_PATH){
            filePath = db->getDir() + filePath;
            dataPath = db->getDir() + dataPath;
        }
        FILE* fp;
        if((fp = fopen((filePath+FRAM_SUFFIX).c_str(), "r")) == NULL){
            throw FILE_NOT_FOUND;
        }
        fseek(fp, 0, SEEK_SET);
        Table* table = new Table(db, " ");
        size_t res;
        res = fread(&table->page_size_, 2, 1, fp);
        res = fread(table->table_name_, 32, 1, fp);
        res = fread(&table->parm_num_, 2, 1, fp);
        table->parm_types_ = new DATA_TYPE[table->parm_num_];
        table->parm_names_ = new char[table->parm_num_][32]{{0}};
        res = fread(table->parm_types_, 2, table->parm_num_, fp);
        res = fread(table->parm_names_, 32, table->parm_num_, fp);
        res = fread(&table->prim_key_, 2, 1, fp);
        res = fread(&table->row_take_up_, 2, 1, fp);
        res = fread(&table->max_rows_per_page_, 2, 1, fp);
        table->key_type_ = table->getKeyType();
        if(res < 0){
            delete table;
            throw FILE_DAMAGED;
        }
        fclose(fp);
        //主键索引树重构
        table->filePath_ = filePath;
        if((fp = fopen((dataPath+DATA_SUFFIX).c_str(), "r")) == NULL){
            //cout << "<W> DATA SOURCE FILE NOT FOUND : " << table_name << endl;
            return table;
        }
        res = fread(&table->max_offset, 2, 1, fp);
        bool notEmpty = true;
        for(__uint16_t i = 0; i < table->max_offset; i++){
            fseek(fp, DATA_OFFSET + table->page_size_ * i, SEEK_SET);
            res = fread(&notEmpty, 1, 1, fp);
            if(!notEmpty){
                table->add_empty_page(i);
                continue;
            }
            Index index;
            index.type_ = table->key_type_;
            res = fread(&index.index_, index.getSize(), 1, fp);
            table->pages_tree_->InsertData(index, i);
        }
        fclose(fp);
        return table;
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw FILE_DAMAGED;
    }
}


Page* Memorizer::PageLoad(__uint16_t offset){
    try{
        if(table_ == NULL){
            throw TABLE_NOT_FOUND;
        }
        string filePath = table_->filePath_ + DATA_SUFFIX;
        FILE* fp;
        if((fp = fopen(filePath.c_str(), "r")) == NULL){
            throw FILE_NOT_FOUND;
        }
        size_t res;
        fseek(fp, DATA_OFFSET + offset * table_->page_size_, SEEK_SET);
        bool notEmpty;
        res = fread(&notEmpty, 1, 1, fp);
        if(!notEmpty) return NULL;
        Page* page = new Page(table_);
        page->not_empty_ = true;
        res = fread(&page->page_index_.index_, page->page_index_.getSize(), 1, fp);
        res = fread(&page->is_full_, 1, 1, fp);
        res = fread(&page->cursor_pos_, 2, 1, fp);
        for(__uint16_t i = 0; i < page->cursor_pos_; i++){
            Row* row = new Row(table_);
            for(__uint16_t j = 0; j < table_->parm_num_; j++){
                res = fread(row->content_[j], kTypeSize[table_->parm_types_[j]], 1, fp);
                row->index_update();
            }
            page->rows_[i] = row;
        }
        if(res < 0) return NULL;
        fclose(fp);
        return page;
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw FILE_DAMAGED;
    }
}

void Memorizer::PageFlush(__uint16_t offset){
    try{
        if(table_ == NULL){
            throw TABLE_NOT_FOUND;
        }
        string filePath = table_->filePath_ + DATA_SUFFIX;
        FILE* fp;
        fp = fopen(filePath.c_str(), "ab+");
        fclose(fp);
        fp = fopen(filePath.c_str(), "r+");
        fwrite(&table_->max_offset, 2, 1, fp);
        fseek(fp, DATA_OFFSET + offset * table_->page_size_, SEEK_SET);
        char ch[table_->page_size_ + 1]{{0}};
        //memset(ch, 0, sizeof(ch));
        fwrite(ch, table_->page_size_, 1, fp);
        fclose(fp);
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw FILE_DAMAGED;
    }
}


void Memorizer::PageStore(__uint16_t offset, Page* page){
    if(page == NULL) return;
    try{
        if(table_ == NULL){
            throw TABLE_NOT_FOUND;
        }
        string filePath = table_->filePath_ + DATA_SUFFIX;
        FILE* fp;
        fp = fopen(filePath.c_str(), "ab+");
        fclose(fp);
        fp = fopen(filePath.c_str(), "r+");
        fwrite(&page->table_ptr_->max_offset, 2, 1, fp);
        fseek(fp, DATA_OFFSET + offset * table_->page_size_, SEEK_SET);
        fwrite(&page->not_empty_, 1, 1, fp);
        fwrite(&page->page_index_.index_, page->page_index_.getSize(), 1, fp);
        fwrite(&page->is_full_, 1, 1, fp);
        fwrite(&page->cursor_pos_, 2, 1, fp);
        for(__uint16_t i = 0; i < page->cursor_pos_; i++){
            Row* row = page->rows_[i];
            for(__uint16_t j = 0; j < page->table_ptr_->parm_num_; j++){
                fwrite(row->content_[j], kTypeSize[page->table_ptr_->parm_types_[j]], 1, fp);
            }
        }
        fclose(fp);
    }
    catch(NEexception& e){
        throw e;
    }
    catch(exception& e){
        throw FILE_DAMAGED;
    }
}

void Memorizer::TableDrop(){
    try{
        string filePath = table_->filePath_ + FRAM_SUFFIX;
        string dataPath = table_->filePath_ + DATA_SUFFIX;
        FILE* fp = NULL;
        if((fp = fopen(filePath.c_str(), "r")) == NULL){
            throw FILE_NOT_FOUND;
        }
        remove(filePath.c_str());
        if((fp = fopen(dataPath.c_str(), "r")) != NULL){
            remove(dataPath.c_str());
        }
    }
    catch(NEexception& e){
        throw e;
    }
}
