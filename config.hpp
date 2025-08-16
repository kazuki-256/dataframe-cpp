#ifndef _DF_CONFIG_HPP_
#define _DF_CONFIG_HPP_

#define DF_VERSION "beta 1.0.0"

#if __cplusplus < 201703L
  #error "Azuki's DataFrame requires C++17 or higher. Please enable -std=c++17 or later."
#endif


#define DF_DEFAULT_TYPE double

long DF_DEFAULT_RETURN;
const void* DF_DEFAULT_RETURN_POINTER = &DF_DEFAULT_RETURN;

#define DF_CHUNK_GROWTH_FACTOR 2




#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif





#endif // _DF_CONFIG_HPP_