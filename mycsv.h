#include <stdio.h>
#include <stdlib.h>
#include <time.h>


enum CsvDataTypes {
    CSV_DT_NONE,
    CSV_DT_POINTER,
    CSV_DT_NUMBER,
    CSV_DT_STRING,
    CSV_DT_DATE,
};


typedef union CsvData {
    void* asPointer;
    int* asNumber;
    char* asString;
    time_t asDate;
} CsvData;


typedef struct CsvTable {
    const char** colNames;
    CsvDataTypes* colDataTypes;

    CsvData** data;

    int colCount;
    int rowCount;
} CsvTable;



#define CsvForeachRowData(row, data)
#define CsvForeachRow(table, row)
#define CsvForeachColumn(table, name)




CsvTable CsvLoad(const char* path) {
    CsvTable table = {};

    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        return table;
    }

    size_t bufLength = 0, bufCapacity = 1024;
    char* buf = (char*)malloc(bufCapacity);

    if (buf == NULL) {
        return table;
    }
}




void CsvDestroyRow(CsvRow* row) {
    
}



void CsvDestroyTable(CsvTable* csv) {
    for (int i = 0; i < csv->rowCount; i++) {
        if (csv->)
    }
    
    for (int i = 0; i < csv->colCount; i++) {
        free(csv->colNames);
    }

    free(csv->colNames);
    free(csv->colNames);
    free(csv->colDataTypes)
}