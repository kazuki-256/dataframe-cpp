#ifndef _DF_COLUMN_HPP_
#define _DF_COLUMN_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_OBJECT_HPP_
#include "object.hpp"
#endif

#ifndef _LINKABLE_HPP_
#include "../linkable/linkable.hpp"
#endif

#include <list>
#include <set>



// ==== df_mem_block_t ====

class df_mem_block_t : TLinkable {
  friend class df_column_t;

  size_t usage;
  size_t capacity;
  df_type_t type;

  // create block without any settings
  df_mem_block_t(df_type_t type, size_t capacity) {
    this->type = type;
    this->capacity = capacity;
    this->usage = 0;
  }


  static df_mem_block_t* create(df_type_t type, int capacity) {
    const size_t DATA_SIZE = df_type_get_size(type);
    size_t back_size = capacity * DATA_SIZE;
    back_size = (back_size + 256) & ~255; // align to 256 bytes

    void* mem = malloc(sizeof(df_mem_block_t) + back_size);
    return new (mem) df_mem_block_t(type, back_size / DATA_SIZE);
  }




  // copy for other array
  void fill_unsafe(int start, const uint8_t* src, df_type_t src_type, int length) {
    const int DATA_SIZE = df_type_get_size(type);

    df_mem_callback2_t convertor = DF_MEM_CONVERT_TABLE[df_type_get_typeid(type)][df_type_get_typeid(src_type)];
    if (convertor == NULL) {
      df_debug6("invalid convert: %s -> %s", df_type_get_string(type), df_type_get_string(src_type));
      return;
    }

    uint8_t* dest = (uint8_t*)get_start() + start * DATA_SIZE;
    uint8_t* end = dest + length * DATA_SIZE;

    for (; dest < end; dest += DATA_SIZE, src += DATA_SIZE) {
      convertor(src, dest);
    }
    usage = DF_MAX(usage, start + length);
  }

  void fill_unsafe(int start, const std::initializer_list<df_object_t>& source_objects) {
    const int DATA_SIZE = df_type_get_size(type);

    df_mem_callback2_t convertor = DF_MEM_CONVERT_TABLE[df_type_get_typeid(type)][df_type_get_typeid(type)];
    if (convertor == NULL) {
      df_debug6("invalid convert: %s -> %s", df_type_get_string(type), df_type_get_string(type));
      return;
    }

    uint8_t* dest = (uint8_t*)get_start() + start * DATA_SIZE;
    for (const df_object_t& object : source_objects) {
      if (object.type != type) {
        df_debug3("detacted initing %s object in %s column, auto converting...", df_type_get_string(object.type), df_type_get_string(type));
        df_mem_convert(object.mem, object.type, dest, type);
        dest += DATA_SIZE;
        continue;
      }

      convertor(object.mem, dest);
      dest += DATA_SIZE;
    }
    usage = DF_MAX(usage, start + source_objects.size());
  }

  inline uint8_t* get_start() const {
    return (uint8_t*)this + sizeof(df_mem_block_t);
  }

public:
  ~df_mem_block_t() {
    if (!df_type_is_struct(type)) {
      return;
    }

    if (type == DF_TYPE_TEXT) {
      const int SIZE = df_type_get_size(type);
      for (uint8_t* p = get_start(), *END = p + usage; p < END; p += SIZE) {
        df_mem_release_string(p);
      }
      return;
    }
    df_debug6("deleting unknown structed type");
  }

  // == move ==
  df_mem_block_t(df_mem_block_t&& other) = delete;
  df_mem_block_t& operator=(df_mem_block_t&& other) = delete;

  // == copy ==
  df_mem_block_t(const df_mem_block_t& other) = delete;
  df_mem_block_t operator=(const df_mem_block_t& other) = delete;
};






// ==== df_column_t ==

class df_column_t {
  TLinkableList<df_mem_block_t> blocks;
  int length;
  df_type_t data_type;

  std::set<df_column_t*> be_foregined;  // SQL foregined by who (not usable)
  df_column_t* foregin_to = NULL;       // SQL foregin key (not usable)

  
  df_object_t& get_object_at(int index) const;

  df_mem_block_t* extend_chunk(int size, int* lessing_size);


  template<typename T>
  void typed_merge(const df_column_t& other);



public:

  ~df_column_t() noexcept(false) {
    df_debug3("delete column with %d blocks", blocks.tlLength());
  }


  df_column_t(const std::initializer_list<df_object_t>& objects) {
    df_debug3("create column with %lu objects", objects.size());

    data_type = objects.begin()->get_type();
    length = objects.size();

    df_mem_block_t* block = blocks.tlAdd(df_mem_block_t::create(data_type, objects.size()));
    block->fill_unsafe(0, objects);
  }

    // advaned constructor
  df_column_t(df_type_t objtype, int capacity = 0);


  
  // === get information ===

  inline df_type_t get_data_type() const {
    return data_type;
  }

  inline int get_block_count() const {
    return blocks.tlLength();
  }

  inline int get_length() const {
    return length;
  }



  // === iterator ===

  class mem_iterator {
  protected:
    df_mem_block_t* block;
    uint8_t* p;
    uint8_t* end;
    size_t data_size;

  public:
    mem_iterator(df_mem_block_t* block) {
      this->block = block;
      this->data_size = df_type_get_size(block->type);
      this->p = block->get_start();
      this->end = p + block->usage * data_size;
    }

    mem_iterator(int) {
      this->block = NULL;
    }


    inline uint8_t* operator*() {
      return p;
    }

    inline mem_iterator& operator++() {
      p += data_size;
      if (p >= end) {
        block = (df_mem_block_t*)block->tlNext();
        
        if (block) {
          p = block->get_start();
          end = p + block->usage * data_size;
        }
      }
      return *this;
    }

    inline mem_iterator operator++(int) {
      mem_iterator old = *this;
      ++(*this);
      return old;
    }

    inline bool operator!=(const mem_iterator& other) const {
      return block != NULL;
    }
  };

  class const_mem_iterator : public mem_iterator {
  public:
    const_mem_iterator(df_mem_block_t* block) : mem_iterator(block) {}

    const_mem_iterator(int) : mem_iterator(0) {}
  };

  class iterator : public mem_iterator {
    df_object_t user_object;
  public:
    iterator(df_mem_block_t* block) : mem_iterator(block) {}

    iterator(int) : mem_iterator(0) {}


    inline df_object_t& operator*() {
      user_object.set_target(p, block->type);
      return user_object;
    }
  };

  class const_iterator : public mem_iterator {
    df_object_t user_object;
  public:
    const_iterator(df_mem_block_t* block) : mem_iterator(block) {}

    const_iterator(int) : mem_iterator(0) {}


    inline const df_object_t& operator*() {
      user_object.set_target(p, block->type);
      return user_object;
    }
  };



  inline mem_iterator mem_begin() {
    return mem_iterator(blocks.tlFront());
  }

  inline mem_iterator mem_end() {
    return mem_iterator(0);
  }

  inline const_mem_iterator mem_begin() const {
    return const_mem_iterator(blocks.tlFront());
  }

  inline const_mem_iterator mem_end() const {
    return const_mem_iterator(0);
  }


  inline iterator begin() {
    return iterator(blocks.tlFront());
  }

  inline iterator end() {
    return iterator(0);
  }

  inline const_iterator begin() const {
    return const_iterator(blocks.tlFront());
  }

  inline const_iterator end() const {
    return const_iterator(0);
  }



  // === operator ===

  // == move ==

  df_column_t(df_column_t&& src) {
    df_debug3("move column with %d blocks", src.blocks.tlLength());

    this->blocks = std::move(src.blocks);
    this->length = src.length;
    this->data_type = src.data_type;

    this->be_foregined = std::move(src.be_foregined);
    this->foregin_to = src.foregin_to;

    src.length = 0;
    src.data_type = DF_TYPE_NULL;
  }

  df_column_t& operator=(df_column_t&& src) {
    df_debug3("move column with %d blocks", src.blocks.tlLength());

    if (this == &src) {
      return *this;
    }

    this->blocks = std::move(src.blocks);
    this->length = src.length;
    this->data_type = src.data_type;

    this->be_foregined = std::move(src.be_foregined);
    this->foregin_to = src.foregin_to;

    src.length = 0;
    src.data_type = DF_TYPE_NULL;
    return *this;
  }

  // == copy ==

  df_column_t(const df_column_t& src) {
    df_debug3("copy column with %d blocks", src.blocks.tlLength());

    this->data_type = src.data_type;
    
    df_mem_block_t* new_block = df_mem_block_t::create(src.data_type, src.length);
    blocks.tlAdd(new_block);

    for (df_mem_block_t* block : src.blocks) {
      new_block->fill_unsafe(length, block->get_start(), src.data_type, block->usage);
      length += block->usage;
    }

    this->be_foregined = src.be_foregined;
    this->foregin_to = src.foregin_to;
  }

  df_column_t operator=(const df_column_t& src) {
    df_debug3("copy column with %d blocks", src.blocks.tlLength());

    if (this == &src) {
      return *this;
    }

    blocks.tlClear();

    data_type = src.data_type;
    length = 0;
    
    df_mem_block_t* new_block = df_mem_block_t::create(src.data_type, src.length);
    blocks.tlAdd(new_block);

    for (df_mem_block_t* block : src.blocks) {
      new_block->fill_unsafe(length, block->get_start(), src.data_type, block->usage);
      length += block->usage;
    }

    this->be_foregined = src.be_foregined;
    this->foregin_to = src.foregin_to;
    return *this;
  }



  // == set_foregin ==

  df_column_t& set_foregin(df_column_t* column);


  // == add_object ==

  df_column_t& add_object(const df_object_t& object);

  df_column_t& add_object(df_object_t&& object);


  // == get_object ==

  inline const df_object_t& operator[](int index) const;

  inline df_object_t& operator[](int index);



  // == merge ==

  template<typename other_t>
  df_column_t& merge_with(const df_column_t& other);

  template<typename other_t>
  df_column_t merge_to(const df_column_t& other) const;


  // == print ==

  std::ostream& write_stream(std::ostream& os, const char* colname) const {
    df_mem_callback2_t callback = DF_MEM_CONVERT_TABLE[df_type_get_typeid(data_type)][DF_TYPEID_TEXT];
    if (callback == NULL) {
      df_debug6("couldn't print column with type %s", df_type_get_string(data_type));
      return os;
    }

    df_string_t buffer;

    os << "| " << colname << " |\n";
    for (mem_iterator it = mem_begin(); it != mem_end(); it++) {
      callback(*it, &buffer);
      os << "| " << buffer.value_or("null") << " |\n";
    }
    return os;
  }

  friend std::ostream& operator<<(std::ostream& os, const df_column_t& column) {
    return column.write_stream(os, "x");
  }



  // == SQL ==

  df_query_t as(const char* name) const;


  // == vector ==

  df_query_t operator+(long num) const;
  df_query_t operator+(double num) const;

  df_query_t operator-(long num) const;
  df_query_t operator-(double num) const;

  df_query_t operator*(long num) const;
  df_query_t operator*(double num) const;
  
  df_query_t operator/(long num) const;
  df_query_t operator/(double num) const;
  
  df_query_t operator%(long num) const;
  df_query_t operator%(double num) const;

  // also in Math.hpp
  // df_query_t& DfSqrt(df_query_t)
  // df_query_t& DfAvg(df_query_t)
  // ...
};




std::ostream& operator<<(std::ostream& os, const std::pair<std::string, df_column_t>& named_column) {
    return named_column.second.write_stream(os, named_column.first.c_str());
}






#endif // _DF_COLUMN_HPP_
