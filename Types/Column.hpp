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



class df_process;
template<typename object_t> class df_column;



// ==== df_object_chunk ====

template<typename object_t>
class df_object_chunk : TLinkable {
  template<typename T> friend class df_column;

  df_object<object_t>* objects;
  int usage;
  int capacity;


  inline void init(int _capacity);


  // create block without any settings
  df_object_chunk(int capacity);



  // copy for other array
  void fill(int start, const df_object<object_t>* source_objects, int length);

  void fill(int start, const std::initializer_list<df_object<object_t>>& source_objects);

  inline void release_objects_only();


  inline void from_move(df_object_chunk& other);

public:
  ~df_object_chunk();


  df_object_chunk();


  // == move ==

  df_object_chunk(df_object_chunk&& other);

  df_object_chunk& operator=(df_object_chunk&& other);


  // == copy ==

  df_object_chunk(const df_object_chunk& other);

  df_object_chunk operator=(const df_object_chunk& other) noexcept;
};






// ==== df_column ==

template<typename object_t = DF_DEFAULT_TYPE>
class df_column {
  template<typename T> friend class df_column;

  TLinkableList<df_object_chunk<object_t>> chunks;
  int length;
  uint8_t type;

  std::set<df_column<object_t>*> be_foregined;  // SQL foregined by who (not usable)
  df_column<object_t>* foregin_to = NULL;       // SQL foregin key (not usable)

  
  df_object<object_t>& get_object_at(int index) const;

  df_object_chunk<object_t>* extend_chunk(int size, int* lessing_size = DF_DEFAULT_RETURN_POINTER);


  template<typename T>
  void typed_merge(const df_column<T>& other);



public:

  ~df_column() noexcept(false);



  df_column(const std::initializer_list<df_object<object_t>>& objects);


  df_column(int capacity = 0);

    // advaned constructor
  df_column(df_type objtype, int capacity = 0);


  
  // === get information ===

  inline df_type get_type() const;

  inline int get_chunk_count() const;

  inline int get_length() const;



  // === iterator ===

  class iterator;

  class const_iterator : public iterator;


  inline iterator begin();

  inline iterator end();

  inline const_iterator begin() const;

  inline const_iterator end() const;



  // === operator ===

  // == move ==

  df_column(df_column&& src);

  df_column& operator=(df_column&& src);

  // == copy ==

  df_column(const df_column& src);

  df_column operator=(const df_column& src);


  // == cast ==

  template<typename Dest>
  operator df_column<Dest>&();

  template<typename Dest>
  operator const df_column<Dest>&() const;



  // == set_foregin ==

  df_column& set_foregin(df_column* column);


  // == add_object ==

  df_column& add_object(const df_object<object_t>& object);

  df_column& add_object(df_object<object_t>&& object);


  // == get_object ==

  inline const df_object<object_t>& operator[](int index) const;

  inline df_object<object_t>& operator[](int index);



  // == merge ==

  template<typename other_t>
  df_column<object_t>& merge_with(const df_column<other_t>& other);

  template<typename other_t>
  df_column<object_t> merge_to(const df_column<other_t>& other) const;



  // == print ==

  std::ostream& write_stream(std::ostream& os, const char* colname) const;

  friend std::ostream& operator<<(std::ostream& os, const df_column<object_t>& col);


  // == SQL ==

  df_process as(const char* name) const;


  // == vector ==

  df_process operator+(int num) const;
  df_process operator+(df_number num) const;

  df_process operator-(int num) const;
  df_process operator-(df_number num) const;

  df_process operator*(int num) const;
  df_process operator*(df_number num) const;
  
  df_process operator/(int num) const;
  df_process operator/(df_number num) const;
  
  df_process operator%(int num) const;
  df_process operator%(df_number num) const;

  // also in Math.hpp
  // df_process& DfSqrt(df_process)
  // df_process& DfAvg(df_process)
  // ...
};




#endif // _DF_COLUMN_HPP_