#pragma once

#include "column.hpp"
#include "object.cpp"


// ==== df_mem_block_t ====

// == destroy ==

df_mem_block_t::~df_mem_block_t() {
    if (!df_type_is_struct(data_type)) {
        return;
    }

    if (data_type == DF_TYPE_TEXT) {
        for (uint8_t* p = get_data_start(), *END = p + usage; p < END; p += DF_TYPESIZE_TEXT) {
            df_value_release_string_mem(p);
        }
        return;
    }
    df_debug6("deleting unknown structed data_type");
}



// == create ==

df_mem_block_t::df_mem_block_t(df_type_t data_type, long capacity) {
    this->data_type = data_type;

    this->capacity = capacity;
    this->usage = 0;
}


df_mem_block_t* df_mem_block_t::create(df_type_t data_type, long capacity) {
    const int size_per_data = df_type_get_size(data_type);
    const long final_capacity = (capacity + 1024) & ~1023;

    void* memory = malloc(sizeof(df_mem_block_t) + final_capacity * (sizeof(bool) + size_per_data));
    return new (memory) df_mem_block_t(data_type, final_capacity);
}




uint8_t* df_mem_block_t::init_values(long start, long length, int size_per_data) {
    uint8_t* data_p = get_data_at(usage);
    uint8_t* data_end = get_data_at(start + length);

    df_value_init_callback_t init_callback = df_value_get_init_callback(data_type);
    if (init_callback != NULL) {
        for (; data_p < data_end; data_p += size_per_data) {
            init_callback(data_p);
        }
    }
    return data_end;
}


// == fill ==

inline void df_mem_block_t::fill_unsafe(long start, df_const_mem_iterator_t& iter,
        df_type_t src_type, long length, int size_per_data,
        df_value_load_callback_t loader, df_value_write_callback_t writer
) {
    const int DATA_SIZE = df_type_get_size(data_type);

    // == init values ==
    uint8_t* data_end = init_values(start, length, size_per_data);

    // == write ==

    uint8_t* data_p = get_data_at(start);
    uint8_t* null_p = get_null_at(start);

    for (; data_p < data_end; data_p += DATA_SIZE, iter++) {
        if ((*null_p++ = *iter.get_null())) {
            continue;
        }

        df_value_t value = loader(iter.get_data());
        writer(value, data_p);
    }
    usage = DF_MAX(usage, start + length);
}



inline void df_mem_block_t::fill_unsafe(long start, const std::initializer_list<df_object_t>& source_objects) {
    const int SIZE_PER_DATA = df_type_get_size(data_type);
    const int DATA_TYPE_ID = df_type_get_typeid(data_type);

    // == get callbacks ==

    df_value_load_callback_t default_loader = DF_VALUE_LOAD_CALLBACKS[DATA_TYPE_ID];
    df_value_write_callback_t default_writer = DF_VALUE_WRITE_CALLBACKS[DATA_TYPE_ID][DATA_TYPE_ID];

    if (default_loader == NULL || default_writer == NULL) {
        const char* TYPE_NAME = df_typeid_get_string(DATA_TYPE_ID);
        df_debug3("invalid convertion %s -> %s", TYPE_NAME, TYPE_NAME);
        return;
    }

    // == init values ==
    uint8_t* data_end = init_values(start, source_objects.size(), SIZE_PER_DATA);

    // == write values ==

    uint8_t* data_p = get_data_at(start);
    uint8_t* null_p = get_null_at(start);

    for (const df_object_t& object : source_objects) {
        if ((*null_p++ = object.is_null())) {
            data_p += SIZE_PER_DATA;
            continue;
        }

        if (object.target_type != data_type) {
            df_debug3("detacted initing %s object in %s column, auto converting...", df_type_get_string(object.target_type), df_typeid_get_string(DATA_TYPE_ID));

            df_value_write(object.target_preload, object.target_type, data_p, data_type);
            data_p += SIZE_PER_DATA;
            continue;
        }

        default_writer(object.target_preload, data_p);
        data_p += SIZE_PER_DATA;
    }

    // set variable
    usage = DF_MAX(usage, start + source_objects.size());
}




// == get ==

uint8_t* df_mem_block_t::get_null_start() const {
    return (uint8_t*)this + sizeof(df_mem_block_t);
}

uint8_t* df_mem_block_t::get_null_at(long index) const {
    return get_null_start() + usage;
}

uint8_t* df_mem_block_t::get_data_start() const {
    return (uint8_t*)this + sizeof(df_mem_block_t) + capacity;
}

uint8_t* df_mem_block_t::get_data_at(long index) const {
    return get_data_start() + index * df_type_get_size(data_type);
}





// ==== df_mem_iterator_t ====

void df_mem_iterator_t::forward(long offset) {
    data_p += offset * size_per_data;

    if (data_p < data_end) {
        null_p += offset;
        return;
    }
    offset -= block->usage;
    
    while ((block = (df_mem_block_t*)block->tlNext()) != NULL) {
        if (offset < block->usage) {
            // improve of:
            // null_p = block->get_null_at(offset);
            // data_p = block->get_data_at(offset);
            // data_end = block->get_data_at(block->usage);

            null_p = (data_end = block->get_null_start()) + offset;
            data_p = (data_end += block->capacity) + offset * size_per_data;
            data_end += block->usage * size_per_data;
            return;
        }
        offset -= block->usage;
    }
    return;   // block == NULL, meaning not found
}

void df_mem_iterator_t::basic_copy(const df_mem_iterator_t& other) {
    block = other.block;
    null_p = other.null_p;
    data_p = other.data_p;
    data_end = other.data_end;
    size_per_data = other.size_per_data;
}

df_mem_iterator_t::df_mem_iterator_t(df_mem_block_t* init_block) {
    block = init_block;
    size_per_data = df_type_get_size(block->data_type);

    null_p = block->get_null_start();
    data_p = block->get_data_start();
    data_end = data_p + block->usage * size_per_data;
}

constexpr df_mem_iterator_t::df_mem_iterator_t() {}


df_mem_iterator_t::df_mem_iterator_t(const df_mem_iterator_t& other) {
    basic_copy(other);
}

df_mem_iterator_t& df_mem_iterator_t::operator=(const df_mem_iterator_t& other) {
    if (this == &other) {
        return *this;
    }
    
    basic_copy(other);
    return *this;
}


bool* df_mem_iterator_t::get_null() const {
    return (bool*)null_p;
}

uint8_t* df_mem_iterator_t::get_data() const {
    return data_p;
}


df_mem_iterator_t& df_mem_iterator_t::operator++() {
    null_p++;
    data_p += size_per_data;

    if (data_p >= data_end) {
        block = (df_mem_block_t*)block->tlNext();

        if (block) {
            null_p = block->get_null_start();
            data_p = null_p + block->capacity;
            data_end = data_p + block->usage * size_per_data;
        }
    }
    return *this;
}

df_mem_iterator_t& df_mem_iterator_t::operator++(int) {
    ++(*this);
    return *this;
}

df_mem_iterator_t& df_mem_iterator_t::operator+=(long offset) {
    forward(offset);
    return *this;
}


bool df_mem_iterator_t::is_end() const {
    return block == NULL;
}

bool df_mem_iterator_t::operator!=(const df_mem_iterator_t& other) const {
    return block != other.block;
}



// ==== df_const_mem_iterator_t ====

df_const_mem_iterator_t::df_const_mem_iterator_t(df_mem_block_t* block) : df_mem_iterator_t(block) {}

constexpr df_const_mem_iterator_t::df_const_mem_iterator_t() {}

df_const_mem_iterator_t& df_const_mem_iterator_t::operator+=(long offset) {
    forward(offset);
    return *this;
}





// ==== df_object_iterator_t ====

df_object_iterator_t::df_object_iterator_t(df_mem_block_t* block) : df_mem_iterator_t(block) {
    loader = df_value_get_load_callback(block->data_type);
    user_object.target_type = block->data_type;
    user_object.lock();
}

constexpr df_object_iterator_t::df_object_iterator_t() {}


df_object_t& df_object_iterator_t::operator*() {
    user_object.set_target(null_p, data_p, loader);
    return user_object;
}



df_object_iterator_t& df_object_iterator_t::operator+=(long offset) {
    forward(offset);
    return *this;
}



// ==== df_const_object_iterator_t ====

df_const_object_iterator_t::df_const_object_iterator_t(df_mem_block_t* block) : df_object_iterator_t(block) {
    loader = df_value_get_load_callback(block->data_type);
    user_object.target_type = block->data_type;
    user_object.lock();
}

constexpr df_const_object_iterator_t::df_const_object_iterator_t() {}


const df_object_t& df_const_object_iterator_t::operator*() {
    user_object.set_target(null_p, data_p, loader);
    return user_object;
}

df_const_object_iterator_t& df_const_object_iterator_t::operator+=(long offset) {
    forward(offset);
    return *this;
}













// ==== df_column_t ====

// == destroy ==

df_column_t::~df_column_t() noexcept(false) {
    df_debug3("delete %s[%ld] column at %p", df_type_get_string(data_type), length, this);

    blocks.tlClear();
}



// == create ==

df_mem_block_t* df_column_t::init_normal(df_type_t data_type, long length, long start_capacity) {
    df_debug3("create %s[%ld] column at %p", df_type_get_string(data_type),length, this);

    this->data_type = data_type;
    this->length = length;

    return blocks.tlAdd(df_mem_block_t::create(data_type, start_capacity));
}


template<typename DEST, typename T>
void df_column_t::init_by_typed(df_type_t data_type, const std::initializer_list<T>& const_list) {
    df_debug4("typed-create %s[%ld] column at %p", df_type_get_string(df_type_get_type<DEST>), const_list.size(), this);

    this->data_type = data_type;
    this->length = const_list.size();

    df_mem_block_t* block = blocks.tlAdd(df_mem_block_t::create(data_type, length * 2));


    const int DATA_SIZE = df_type_get_size(data_type);
    uint8_t* p = block->get_data_start();

    memset(block->get_null_start(), false, length);

    for (T const_value : const_list) {
        new (p) DEST(const_value);
        p += DATA_SIZE;
    }
    block->usage = length;
}



df_column_t::df_column_t(int) {}



constexpr df_column_t::df_column_t() {}


df_column_t::df_column_t(df_type_t data_type, long start_capacity) {
    init_normal(data_type, 0, start_capacity);
}


df_column_t::df_column_t(const std::initializer_list<df_object_t>& objects) {
    df_mem_block_t* block = init_normal(objects.begin()->get_type(), objects.size(), objects.size() * 2);
    block->fill_unsafe(0, objects);
}




// == copy ==

void df_column_t::copy(const df_column_t& src) {
    df_debug3("copy %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.length, &src, this);
    data_type = src.data_type;

    df_mem_block_t* new_block = df_mem_block_t::create(src.data_type, src.length);
    blocks.tlAdd(new_block);

    df_const_mem_iterator_t iter = src.mem_begin();
    new_block->fill_unsafe(0, iter,
            src.data_type, src.length, df_type_get_size(src.data_type),
            df_value_get_load_callback(src.data_type), df_value_get_write_callback(src.data_type, src.data_type));
    length = src.length;
}



df_column_t::df_column_t(const df_column_t& src) {
    copy(src);
}

df_column_t df_column_t::operator=(const df_column_t& src) {
    if (this == &src) {
        return *this;
    }

    blocks.tlClear();
    length = 0;

    copy(src);
    return *this;
}



// == move ==

void df_column_t::move(df_column_t& src) noexcept {
    df_debug3("move %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.length, &src, this);

    blocks = std::move(src.blocks);
    length = src.length;
    data_type = src.data_type;

    src.length = 0;
}



df_column_t::df_column_t(df_column_t&& src) noexcept {
    move(src);
}

df_column_t& df_column_t::operator=(df_column_t&& src) noexcept {
    if (this != &src) {
        move(src);
    }
    return *this;
}





// == get ==

df_object_t df_column_t::get_object_at(long index) const {
    const int DATA_SIZE = df_type_get_size(data_type);
    const df_value_load_callback_t loader = df_value_get_load_callback(data_type);

    df_mem_block_t* block;
    df_object_t object;
    object.target_type = data_type;

    // == rfind ==

    if (index < 0) {
        index = -index;
        block = blocks.tlBack();

        while (block != NULL) {
            if (index <= block->usage) {
                index = block->usage - index;

                object.set_target(block->get_null_at(index), block->get_data_at(index), loader);
                return object;
            }

            index -= block->usage;
            block = (df_mem_block_t*)block->tlPrev();
        }

        // ERROR: not found
        throw df_exception_out_of_index();
    }

    // == find ==
    block = blocks.tlFront();

    while (block != NULL) {
        if (index < block->usage) {
            object.set_target(block->get_null_at(index), block->get_data_at(index), loader);
            return object;
        }

        index -= block->usage;
        block = (df_mem_block_t*)block->tlNext();
    }

    // ERROR: not found
    throw df_exception_out_of_index();
}



df_type_t df_column_t::get_data_type() const {
    return data_type;
}

int df_column_t::get_block_count() const {
    return blocks.tlLength();
}

long df_column_t::get_length() const {
    return length;
}



// == push ==


df_mem_block_t* df_column_t::expand_front(long sure_size, long& lessing) {
    df_mem_block_t* block = blocks.tlFront();

    if (block == NULL) {
        block = blocks.tlInsertBefore(0, df_mem_block_t::create(data_type, sure_size));
        lessing = block->capacity;
        return block;
    }

    lessing = block->capacity - block->usage;
    if (sure_size > lessing) {
        blocks.tlInsertBefore(0, df_mem_block_t::create(data_type, sure_size - lessing));
    }
    else if (lessing > sure_size) {
        lessing = sure_size;
    }

    return block;
}


df_mem_block_t* df_column_t::expand_back(long sure_size, long& lessing) {
    df_mem_block_t* block = blocks.tlBack();

    if (block == NULL) {
        block = blocks.tlAdd(df_mem_block_t::create(data_type, sure_size));
        lessing = sure_size;
        return block;
    }

    lessing = block->capacity - block->usage;
    if (sure_size > lessing) {
        blocks.tlAdd(df_mem_block_t::create(data_type, sure_size - lessing));
    }
    else if (lessing > sure_size) {
        lessing = sure_size;
    }

    return block;
}





df_column_t& df_column_t::push_front(const df_object_t& object) {
    const int SIZE_PER_DATA = df_type_get_size(data_type);
    long lessing;

    expand_front(1, lessing);

    df_mem_block_t* block = blocks.tlFront();
    uint8_t* start = block->get_data_start();

    // == move exists data ==

    uint8_t* null_p = block->get_null_at(block->usage);
    uint8_t* data_p = block->get_data_at(block->usage);

    while (data_p > start) {
        uint8_t* dest = null_p;
        *dest = *(--null_p);

        dest = data_p;
        memcpy(dest, --data_p, SIZE_PER_DATA);
    }

    // == write ==

    *null_p = object.is_null();
    df_value_write(object.target_preload, object.target_type, start, data_type);
    
    block->usage++;
    this->length++;

    return *this;
}



df_column_t& df_column_t::push_back(const df_object_t& object) {
    long lessing;
    expand_back(1, lessing);
    df_mem_block_t* block = blocks.tlBack();

    *block->get_null_at(block->usage) = object.is_null();
    df_value_write(object.target_preload, object.target_type, block->get_data_at(block->usage), data_type);
    
    block->usage++;
    this->length++;

    return *this;
}




// == merge ==

df_column_t& df_column_t::merge_with(const df_column_t& other) {
    const int SIZE_PER_DATA = df_type_get_size(data_type);
    long lessing;

    // == get callbacks ==

    df_value_load_callback_t loader = df_value_get_load_callback(other.data_type);
    df_value_write_callback_t writer = df_value_get_write_callback(other.data_type, this->data_type);

    // round 1: copy to existing block

    df_mem_block_t* block = expand_back(other.length, lessing);
    df_const_mem_iterator_t iter = other.mem_begin();

    block->fill_unsafe(block->usage, iter,
            other.data_type, lessing, SIZE_PER_DATA,
            loader, writer);

    // round 2: copy to new created block

    if (lessing < other.length) {
        block = blocks.tlBack();

        block->fill_unsafe(block->usage, iter,
            other.data_type, lessing, SIZE_PER_DATA,
            loader, writer);
    }

    // sum up
    this->length += other.length;
    return *this;
}



df_column_t df_column_t::merge_to(const df_column_t& other) const {
    df_column_t output(this->data_type, (this->length + other.length) * 2);

    return output.merge_with(*this).merge_with(other);
}



// == iterate ==

df_mem_iterator_t df_column_t::mem_begin() {
    return df_mem_iterator_t(blocks.tlFront());
}

df_mem_iterator_t df_column_t::mem_end() {
    return df_mem_iterator_t();
}

df_const_mem_iterator_t df_column_t::mem_begin() const {
    return df_const_mem_iterator_t(blocks.tlFront());
}

df_const_mem_iterator_t df_column_t::mem_end() const {
    return df_const_mem_iterator_t();
}


df_object_iterator_t df_column_t::begin() {
    return df_object_iterator_t(blocks.tlFront());
}

df_object_iterator_t df_column_t::end() {
    return df_object_iterator_t();
}

df_const_object_iterator_t df_column_t::begin() const {
    return df_const_object_iterator_t(blocks.tlFront());
}

df_const_object_iterator_t df_column_t::end() const {
    return df_const_object_iterator_t();
}




// == print ==

std::ostream& df_column_t::write_stream(std::ostream& os, const char* colname) const {
    int DATA_TYPE_ID = df_type_get_typeid(data_type);

    // == get callbacks ==

    df_value_load_callback_t loader = DF_VALUE_LOAD_CALLBACKS[DATA_TYPE_ID];
    df_value_write_callback_t writer = DF_VALUE_WRITE_CALLBACKS[DATA_TYPE_ID][DF_TYPEID_TEXT];

    if (loader == NULL || writer == NULL) {
        df_debug6("couldn't print column with data_type %s", df_typeid_get_string(DATA_TYPE_ID));
        return os;
    }

    // == print ==

    std::string buffer;
    df_value_t value;

    os << "| " << colname << " |\n";
    for (df_mem_iterator_t it = mem_begin(); it != mem_end(); it++) {
        if (*it.get_null()) {
            os << "| null |\n";
            continue;
        }

        value = loader(it.get_data());
        writer(value, &buffer);

        os << "| " << buffer << " |\n";
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const df_column_t& column) {
    return column.write_stream(os, DF_DEFAULT_COLUMN_NAME);
}




// ==== pair<std::string, df_column_t> ====

std::ostream& operator<<(std::ostream& os, const std::pair<std::string, df_column_t>& named_column) {
    return named_column.second.write_stream(os, named_column.first.c_str());
}




// ==== sub classes ====

df_column_int32_t::df_column_int32_t(const std::initializer_list<int>& int32_list) : df_column_t(0) {
    init_by_typed<int>(DF_TYPE_INT32, int32_list);
}



df_column_int64_t::df_column_int64_t(const std::initializer_list<long>& int64_list) : df_column_t(0) {
    init_by_typed<long>(DF_TYPE_INT64, int64_list);
}



df_column_float32_t::df_column_float32_t(const std::initializer_list<float>& float32_list) : df_column_t(0) {
    init_by_typed<float>(DF_TYPE_FLOAT32, float32_list);
}



df_column_float64_t::df_column_float64_t(const std::initializer_list<double>& float64_list) : df_column_t(0) {
    init_by_typed<double>(DF_TYPE_FLOAT64, float64_list);
  }



df_column_text_t::df_column_text_t(const std::initializer_list<const char*>& text_list) : df_column_t(0) {
    init_by_typed<std::string>(DF_TYPE_TEXT, text_list);
}

df_column_text_t::df_column_text_t(const std::initializer_list<std::string>& text_list) : df_column_t(0) {
    init_by_typed<std::string>(DF_TYPE_TEXT, text_list);
}



df_column_date_t::df_column_date_t(const std::initializer_list<df_date_t>& date_list) : df_column_t(0) {
    init_by_typed<df_date_t>(DF_TYPE_DATE, date_list);
}



df_column_time_t::df_column_time_t(const std::initializer_list<const char*>& text_list) : df_column_t(0) {
    df_mem_block_t* block = init_normal(DF_TYPE_TIME, text_list.size(), text_list.size() * 2);
    block->usage = length;

    uint8_t* p = block->get_data_start();
    const int DATA_SIZE = df_type_get_size(data_type);

    for (const char* const_text : text_list) {
        new (p) df_date_t(const_text, DF_TIME_FORMAT);
        p += DATA_SIZE;
    }
}



df_column_datetime_t::df_column_datetime_t(const std::initializer_list<df_date_t>& datetime_list) : df_column_t(0) {
    init_by_typed<df_date_t>(DF_TYPE_DATETIME, datetime_list);
}


