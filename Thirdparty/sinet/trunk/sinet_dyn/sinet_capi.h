#ifndef SINET_CAPI_H
#define SINET_CAPI_H

#include "string_capi.h"
#include "buffer_capi.h"
#include "stringmap_capi.h"
#include "intlist_capi.h"
#include "../sinet/api_types.h"
#include "../sinet/task_observer.h"

using namespace sinet;

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct __base_t
  {
    size_t size;

    int (SINET_DYN_CALLBACK *add_ref)(struct __base_t* self);
    int (SINET_DYN_CALLBACK *release)(struct __base_t* self);
    int (SINET_DYN_CALLBACK *get_refct)(struct __base_t* self);
  }_base_t;

#define _MEMBER_EXISTS(s, f)   \
  ((int)&((s)->f) - (int)(s) + sizeof((s)->f) <= (s)->base.size)

#define _MEMBER_MISSING(s, f)  (!_MEMBER_EXISTS(s, f) || !((s)->f))

  typedef struct __config_t
  {
    _base_t base;

    int (SINET_DYN_CALLBACK *get_strvar)(struct __config_t* self, int id, _string_t* strvarout);
    void (SINET_DYN_CALLBACK *set_strvar)(struct __config_t* self, int id, _string_t strvarin);
    int (SINET_DYN_CALLBACK *remove_strvar)(struct __config_t* self, int id);

  }_config_t;

  SINET_DYN_API _config_t* _config_create_instance();


  typedef struct __postdataelem_t
  {
    _base_t base;

    void (SINET_DYN_CALLBACK *set_name)(struct __postdataelem_t* self, const wchar_t* fieldname);
    _string_t (SINET_DYN_CALLBACK *get_name)(struct __postdataelem_t* self);

    void (SINET_DYN_CALLBACK *setto_empty)(struct __postdataelem_t* self);
    void (SINET_DYN_CALLBACK *setto_file)(struct __postdataelem_t* self, const wchar_t* filename);
    void (SINET_DYN_CALLBACK *setto_buffer)(struct __postdataelem_t* self, const void* bytes_in, const size_t size_in);
    void (SINET_DYN_CALLBACK *setto_text)(struct __postdataelem_t* self, const wchar_t* text);

    postdataelem_type_t (SINET_DYN_CALLBACK *get_type)(struct __postdataelem_t* self);

    _string_t (SINET_DYN_CALLBACK *get_file)(struct __postdataelem_t* self);
    size_t (SINET_DYN_CALLBACK *get_buffer_size)(struct __postdataelem_t* self);
    size_t (SINET_DYN_CALLBACK *copy_buffer_to)(struct __postdataelem_t* self, void* bytes_inout, size_t size_in);
    _string_t (SINET_DYN_CALLBACK *get_text)(struct __postdataelem_t* self);

  }_postdataelem_t;

  SINET_DYN_API _postdataelem_t* _postdataelem_create_instance();

  typedef struct __postdata_t
  {
    _base_t base;

    void (SINET_DYN_CALLBACK *clear)(struct __postdata_t* self);
    void (SINET_DYN_CALLBACK *add_elem)(struct __postdata_t* self, _postdataelem_t* elem);
    int (SINET_DYN_CALLBACK *remove_elem)(struct __postdata_t* self, _postdataelem_t* elem);
    _postdataelem_t* (SINET_DYN_CALLBACK *get_elements)(struct __postdata_t* self, int elemindex);
    int (SINET_DYN_CALLBACK *get_element_count)(struct __postdata_t* self);

  }_postdata_t;

  SINET_DYN_API _postdata_t* _postdata_create_instance();

  typedef struct __request_t
  {
    _base_t base;

    void (SINET_DYN_CALLBACK *set_request_method)(struct __request_t* self, const wchar_t* method);
    _string_t (SINET_DYN_CALLBACK *get_request_method)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_request_url)(struct __request_t* self, const wchar_t* url);
    _string_t (SINET_DYN_CALLBACK *get_request_url)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_request_header)(struct __request_t* self, _stringmap_t* header);
    _stringmap_t (SINET_DYN_CALLBACK *get_request_header)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_postdata)(struct __request_t* self, _postdata_t* postdata);
    _postdata_t* (SINET_DYN_CALLBACK *get_postdata)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_response_header)(struct __request_t* self, _stringmap_t* header);
    _stringmap_t (SINET_DYN_CALLBACK *get_response_header)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_response_buffer)(struct __request_t* self, _buffer_t* buffer);
    _buffer_t (SINET_DYN_CALLBACK *get_response_buffer)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_response_size)(struct __request_t* self, size_t size_in);
    size_t (SINET_DYN_CALLBACK *get_response_size)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_retrieved_size)(struct __request_t* self, size_t size_in);
    size_t (SINET_DYN_CALLBACK *get_retrieved_size)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_response_errcode)(struct __request_t* self, int errcode);
    int (SINET_DYN_CALLBACK *get_response_errcode)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_request_outmode)(struct __request_t* self, int outmode);
    int (SINET_DYN_CALLBACK *get_request_outmode)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_outfile)(struct __request_t* self, const wchar_t* file);
    _string_t (SINET_DYN_CALLBACK *get_outfile)(struct __request_t* self);
    void (SINET_DYN_CALLBACK *set_appendbuffer)(struct __request_t* self, const void* data, size_t size);
    void (SINET_DYN_CALLBACK *close_outfile)(struct __request_t* self);

  }_request_t;
  SINET_DYN_API _request_t* _request_create_instance();

  typedef struct __task_t
  {
    _base_t base;

    void (SINET_DYN_CALLBACK *append_request)(struct __task_t* self, _request_t* request_in);
    int (SINET_DYN_CALLBACK *erase_request)(struct __task_t* self, int request_id);
    void (SINET_DYN_CALLBACK *clearall_requests)(struct __task_t* self);
    int (SINET_DYN_CALLBACK *get_request_count)(struct __task_t* self);
    void (SINET_DYN_CALLBACK *get_request_ids)(struct __task_t* self, _intlist_t* ids_out);
    _request_t* (SINET_DYN_CALLBACK *get_request)(struct __task_t* self, int request_id);
    void (SINET_DYN_CALLBACK *set_status)(struct __task_t* self, int status);
    int (SINET_DYN_CALLBACK *get_status)(struct __task_t* self);
    void (SINET_DYN_CALLBACK *attach_observer)(struct __task_t* self, itask_observer* observer_in);
    void (SINET_DYN_CALLBACK *detach_observer)(struct __task_t* self);
    itask_observer* (SINET_DYN_CALLBACK *get_observer)(struct __task_t* self);
    void (SINET_DYN_CALLBACK *use_config)(struct __task_t* self, _config_t* config);
    _config_t* (SINET_DYN_CALLBACK *get_config)(struct __task_t* self);

  }_task_t;
  SINET_DYN_API _task_t* _task_create_instance();

  typedef struct __pool_t
  {
    _base_t base;

    void (SINET_DYN_CALLBACK *execute)(struct __pool_t* self, _task_t* task);
    void (SINET_DYN_CALLBACK *cancel)(struct __pool_t* self, _task_t* task);
    void (SINET_DYN_CALLBACK *clear_all)(struct __pool_t* self);

    int (SINET_DYN_CALLBACK *is_running)(struct __pool_t* self, _task_t* task);
    int (SINET_DYN_CALLBACK *is_queued)(struct __pool_t* self, _task_t* task);
    int (SINET_DYN_CALLBACK *is_running_or_queued)(struct __pool_t* self, _task_t* task);
  }_pool_t;

  SINET_DYN_API _pool_t* _pool_create_instance();

  SINET_DYN_API int sinet_urlencode(const wchar_t* str_in, wchar_t* str_out, int* length_inout);
  SINET_DYN_API int sinet_urldecode(const wchar_t* str_in, wchar_t* str_out, int* length_inout);

#ifdef __cplusplus
}
#endif

#endif // SINET_CAPI_H