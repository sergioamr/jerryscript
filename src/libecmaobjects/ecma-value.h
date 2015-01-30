/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecma-compressed-pointers.h"
#include "ecma-globals.h"
#include "jrt-bit-fields.h"

#ifndef ECMA_VALUE_H
#define ECMA_VALUE_H

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmavalue ECMA value on-stack storage
 * @{
 */

/**
 * Description of ecma-value on-stack storage
 */
class ecma_value_t
{
  public:
    /* Constructors */
    __attribute_always_inline__
    ecma_value_t (ecma_type_t type, /**< ecma-value type */
                  uintptr_t value) /**< compressed pointer
                                    *   or simple value */
    {
      _packed = pack (type, value);
    } /* ecma_value_t */

    __attribute_always_inline__
    ecma_value_t (ecma_simple_value_t simple_value)
      : ecma_value_t (ECMA_TYPE_SIMPLE, simple_value) { }

    __attribute_always_inline__
    ecma_value_t () : ecma_value_t (ECMA_SIMPLE_VALUE_EMPTY) { }

    __attribute_always_inline__
    explicit ecma_value_t (ecma_number_t *num_p)
    {
      *this = num_p;
    }

    __attribute_always_inline__
    explicit ecma_value_t (ecma_string_t *str_p)
    {
      *this = str_p;
    }

    __attribute_always_inline__
    explicit ecma_value_t (ecma_object_t *obj_p)
    {
      *this = obj_p;
    }

    __attribute_always_inline__
    explicit ecma_value_t (ecma_value_packed_t v)
    {
      *this = v;
    }

    ecma_value_t (const ecma_value_t&) = delete;
    ecma_value_t (ecma_value_t&) = delete;
    ecma_value_t (ecma_value_t&&) = delete;
    ecma_value_t& operator = (ecma_value_t &) = delete;
    ecma_value_t& operator = (ecma_value_t &&) = delete;

    /* Extraction of packed representation */
    __attribute_always_inline__
    explicit operator ecma_value_packed_t () const { return _packed; }

    /* Assignment operators */
    __attribute_always_inline__
    ecma_value_t& operator = (ecma_simple_value_t v)
    {
      _packed = pack (ECMA_TYPE_SIMPLE, v);

      return *this;
    }

    ecma_value_t& operator = (ecma_number_t* num_p)
    {
      JERRY_ASSERT(num_p != NULL);

      uint16_t num_cp;
      ECMA_SET_NON_NULL_POINTER (num_cp, num_p);

      _packed = pack (ECMA_TYPE_NUMBER, num_cp);

      return *this;
    }

    ecma_value_t& operator = (ecma_string_t* str_p)
    {
      JERRY_ASSERT(str_p != NULL);

      uint16_t str_cp;
      ECMA_SET_NON_NULL_POINTER (str_cp, str_p);

      _packed = pack (ECMA_TYPE_STRING, str_cp);

      return *this;
    }

    ecma_value_t& operator = (ecma_object_t *obj_p)
    {
      JERRY_ASSERT(obj_p != NULL);

      uint16_t obj_cp;
      ECMA_SET_NON_NULL_POINTER (obj_cp, obj_p);

      _packed = pack (ECMA_TYPE_OBJECT, obj_cp);

      return *this;
    }

    ecma_value_t& operator = (ecma_value_packed_t packed)
    {
      _packed = packed;

      return *this;
    }

    ecma_value_t& operator = (const ecma_value_t &v)
    {
      this->_packed = v._packed;

      return *this;
    }
  private:
    /**
     * Combining type and value fields to packed representation
     *
     * @return packed ecma-value representation
     */
    __attribute_always_inline__
    ecma_value_packed_t pack (ecma_type_t type,
                              uintptr_t value)
    {
      ecma_value_packed_t packed;

      packed = (ecma_value_packed_t) jrt_set_bit_field_value (0,
                                                              type,
                                                              ECMA_VALUE_TYPE_POS,
                                                              ECMA_VALUE_TYPE_WIDTH);

      packed = (ecma_value_packed_t) jrt_set_bit_field_value (packed,
                                                              value,
                                                              ECMA_VALUE_VALUE_POS,
                                                              ECMA_VALUE_VALUE_WIDTH);

      return packed;
    }

    ecma_value_packed_t _packed; /**< packed value representation */
};

/**
 * Description of a block completion value
 *
 * See also: ECMA-262 v5, 8.9.
 */
class ecma_completion_value_t : public ecma_value_t
{
  public:
    /* Constructors */
    ecma_completion_value_t ()
      : ecma_value_t (),
        _type (ECMA_COMPLETION_TYPE_NORMAL) { }

    ecma_completion_value_t (ecma_completion_type_t type,
                             const ecma_value_t &value)
      : ecma_value_t ((ecma_value_packed_t) value),
      _type (type) { }

    /* Assignment operators */
    ecma_completion_value_t & operator = (ecma_completion_type_t type)
    {
      _type = type;

      return *this;
    }

    ecma_completion_value_t & operator = (const ecma_value_t& value)
    {
      *static_cast<ecma_value_t*> (this) = (ecma_value_packed_t) value;

      return *this;
    }

    /* Completion type extraction */
    explicit operator ecma_completion_type_t () const
    {
      return _type;
    }

  private:
    ecma_completion_type_t _type; /**< completion type */
};

/**
 * Get type field of ecma-value
 *
 * @return type field
 */
inline ecma_type_t __attribute_pure__ __attribute_always_inline__
ecma_get_value_type_field (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_type_t) jrt_extract_bit_field ((ecma_value_packed_t) value,
                                              ECMA_VALUE_TYPE_POS,
                                              ECMA_VALUE_TYPE_WIDTH);
} /* ecma_get_value_type_field */

/**
 * Get value field of ecma-value
 *
 * @return value field
 */
inline uintptr_t __attribute_pure__ __attribute_always_inline__
ecma_get_value_value_field (const ecma_value_t& value) /**< ecma-value */
{
  return (uintptr_t) jrt_extract_bit_field ((ecma_value_packed_t) value,
                                            ECMA_VALUE_VALUE_POS,
                                            ECMA_VALUE_VALUE_WIDTH);
} /* ecma_get_value_value_field */

/**
 * Check if the value is empty.
 *
 * @return true - if the value contains implementation-defined empty simple value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_empty (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_EMPTY);
} /* ecma_is_value_empty */

/**
 * Check if the value is undefined.
 *
 * @return true - if the value contains ecma-undefined simple value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_undefined (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_UNDEFINED);
} /* ecma_is_value_undefined */

/**
 * Check if the value is null.
 *
 * @return true - if the value contains ecma-null simple value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_null (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_NULL);
} /* ecma_is_value_null */

/**
 * Check if the value is boolean.
 *
 * @return true - if the value contains ecma-true or ecma-false simple values,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_boolean (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && (ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_TRUE
              || ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_FALSE));
} /* ecma_is_value_boolean */

/**
 * Check if the value is true.
 *
 * Warning:
 *         value must be boolean
 *
 * @return true - if the value contains ecma-true simple value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_true (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && ecma_get_value_value_field (value) == ECMA_SIMPLE_VALUE_TRUE);
} /* ecma_is_value_true */

/**
 * Check if the value is ecma-number.
 *
 * @return true - if the value contains ecma-number value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_number (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_NUMBER);
} /* ecma_is_value_number */

/**
 * Check if the value is ecma-string.
 *
 * @return true - if the value contains ecma-string value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_string (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_STRING);
} /* ecma_is_value_string */

/**
 * Check if the value is object.
 *
 * @return true - if the value contains object value,
 *         false - otherwise.
 */
inline bool __attribute_pure__ __attribute_always_inline__
ecma_is_value_object (const ecma_value_t& value) /**< ecma-value */
{
  return (ecma_get_value_type_field (value) == ECMA_TYPE_OBJECT);
} /* ecma_is_value_object */

extern ecma_number_t* __attribute_pure__ ecma_get_number_from_value (const ecma_value_t& value);
extern ecma_string_t* __attribute_pure__ ecma_get_string_from_value (const ecma_value_t& value);
extern ecma_object_t* __attribute_pure__ ecma_get_object_from_value (const ecma_value_t& value);
extern void ecma_copy_value (ecma_value_t &ret, const ecma_value_t& value, bool do_ref_if_object);
extern void ecma_free_value (ecma_value_t& value, bool do_deref_if_object);

/**
 * Normal, throw, return, exit and meta completion values constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                            ecma_completion_type_t type, /**< type */
                            const ecma_value_t& value) /**< value */
{
  const bool is_type_ok = (type == ECMA_COMPLETION_TYPE_NORMAL
#ifdef CONFIG_ECMA_EXCEPTION_SUPPORT
                     || type == ECMA_COMPLETION_TYPE_THROW
#endif /* CONFIG_ECMA_EXCEPTION_SUPPORT */
                     || type == ECMA_COMPLETION_TYPE_RETURN
                     || type == ECMA_COMPLETION_TYPE_EXIT
                     || (type == ECMA_COMPLETION_TYPE_META
                         && ecma_is_value_empty (value)));

  JERRY_ASSERT (is_type_ok);

  ret_value = type;
  ret_value = value;
} /* ecma_make_completion_value */

/**
 * Simple normal completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_simple_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                                   ecma_simple_value_t simple_value) /**< simple ecma-value */
{
  JERRY_ASSERT(simple_value == ECMA_SIMPLE_VALUE_UNDEFINED
               || simple_value == ECMA_SIMPLE_VALUE_NULL
               || simple_value == ECMA_SIMPLE_VALUE_FALSE
               || simple_value == ECMA_SIMPLE_VALUE_TRUE);

  ecma_make_completion_value (ret_value,
                              ECMA_COMPLETION_TYPE_NORMAL,
                              ecma_value_t (simple_value));
} /* ecma_make_simple_completion_value */

/**
 * Normal completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_normal_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                                   const ecma_value_t& value) /**< value */
{
  ecma_make_completion_value (ret_value, ECMA_COMPLETION_TYPE_NORMAL, value);
} /* ecma_make_normal_completion_value */

/**
 * Throw completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_throw_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                                  const ecma_value_t& value) /**< value */
{
#ifdef CONFIG_ECMA_EXCEPTION_SUPPORT
  ecma_make_completion_value (ret_value, ECMA_COMPLETION_TYPE_THROW, value);
#else /* CONFIG_ECMA_EXCEPTION_SUPPORT */
  (void) value;

  jerry_exit (ERR_UNHANDLED_EXCEPTION);
#endif /* !CONFIG_ECMA_EXCEPTION_SUPPORT */
} /* ecma_make_throw_completion_value */

extern void ecma_make_throw_obj_completion_value (ecma_completion_value_t &ret_value, ecma_object_t *exception_p);

/**
 * Empty completion value constructor.
 *
 * @return (normal, empty, reserved) completion value.
 */
inline void __attribute_always_inline__
ecma_make_empty_completion_value (ecma_completion_value_t &ret_value) /**< out: completion value */
{
  ecma_make_completion_value (ret_value,
                              ECMA_COMPLETION_TYPE_NORMAL,
                              ecma_value_t ());
} /* ecma_make_empty_completion_value */

/**
 * Return completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_return_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                                   const ecma_value_t& value) /**< value */
{
  ecma_make_completion_value (ret_value, ECMA_COMPLETION_TYPE_RETURN, value);
} /* ecma_make_return_completion_value */

/**
 * Exit completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_exit_completion_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                                 bool is_successful) /**< does completion value indicate
                                                          successfulness completion
                                                          of script execution (true) or not (false) */
{
  ecma_make_completion_value (ret_value,
                              ECMA_COMPLETION_TYPE_EXIT,
                              ecma_value_t (is_successful ? ECMA_SIMPLE_VALUE_TRUE
                                            : ECMA_SIMPLE_VALUE_FALSE));
} /* ecma_make_exit_completion_value */

/**
 * Meta completion value constructor
 *
 * @return completion value
 */
inline void __attribute_always_inline__
ecma_make_meta_completion_value (ecma_completion_value_t &ret_value) /**< out: completion value */
{
  ecma_make_completion_value (ret_value,
                              ECMA_COMPLETION_TYPE_META,
                              ecma_value_t (ECMA_SIMPLE_VALUE_EMPTY));
} /* ecma_make_meta_completion_value */

/**
 * Get ecma-value from specified completion value
 *
 * @return ecma-value
 */
inline void __attribute_always_inline__
ecma_get_completion_value_value (ecma_value_t &ret, /**< out: ecma-value */
                                 const ecma_completion_value_t& completion_value) /**< completion value */
{
  const ecma_completion_type_t type = (ecma_completion_type_t) completion_value;

  const bool is_type_ok = (type == ECMA_COMPLETION_TYPE_NORMAL
#ifdef CONFIG_ECMA_EXCEPTION_SUPPORT
                           || type == ECMA_COMPLETION_TYPE_THROW
#endif /* CONFIG_ECMA_EXCEPTION_SUPPORT */
                           || type == ECMA_COMPLETION_TYPE_RETURN
                           || type == ECMA_COMPLETION_TYPE_EXIT);

  JERRY_ASSERT (is_type_ok);

  ret = (ecma_value_packed_t) completion_value;
} /* ecma_get_completion_value_value */

extern void ecma_copy_completion_value (ecma_completion_value_t& ret_value, const ecma_completion_value_t& value);
extern void ecma_free_completion_value (ecma_completion_value_t& completion_value);

/**
 * Check if the completion value is normal value.
 *
 * @return true - if the completion type is normal,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_normal (const ecma_completion_value_t& value) /**< completion value */
{
  return ((ecma_completion_type_t) value == ECMA_COMPLETION_TYPE_NORMAL);
} /* ecma_is_completion_value_normal */

/**
 * Check if the completion value is throw value.
 *
 * @return true - if the completion type is throw,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_throw (const ecma_completion_value_t& value) /**< completion value */
{
#ifdef CONFIG_ECMA_EXCEPTION_SUPPORT
  return ((ecma_completion_type_t) value == ECMA_COMPLETION_TYPE_THROW);
#else /* CONFIG_ECMA_EXCEPTION_SUPPORT */
  (void) value;

  return false;
#endif /* !CONFIG_ECMA_EXCEPTION_SUPPORT */
} /* ecma_is_completion_value_throw */

/**
 * Check if the completion value is return value.
 *
 * @return true - if the completion type is return,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_return (const ecma_completion_value_t& value) /**< completion value */
{
  return ((ecma_completion_type_t) value == ECMA_COMPLETION_TYPE_RETURN);
} /* ecma_is_completion_value_return */

/**
 * Check if the completion value is exit value.
 *
 * @return true - if the completion type is exit,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_exit (const ecma_completion_value_t& value) /**< completion value */
{
  if ((ecma_completion_type_t) value == ECMA_COMPLETION_TYPE_EXIT)
  {
#ifndef JERRY_NDEBUG
    ecma_value_t v ((ecma_value_packed_t) value);

    JERRY_ASSERT (ecma_is_value_boolean (v));
#endif /* !JERRY_NDEBUG */

    return true;
  }
  else
  {
    return false;
  }
} /* ecma_is_completion_value_exit */

/**
 * Check if the completion value is meta value.
 *
 * @return true - if the completion type is meta,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_meta (const ecma_completion_value_t& value) /**< completion value */
{
  if ((ecma_completion_type_t) value == ECMA_COMPLETION_TYPE_META)
  {
#ifndef JERRY_NDEBUG
    ecma_value_t v ((ecma_value_packed_t) value);

    JERRY_ASSERT (ecma_is_value_empty (v));
#endif /* !JERRY_NDEBUG */

    return true;
  }
  else
  {
    return false;
  }
} /* ecma_is_completion_value_meta */

/**
 * Check if the completion value is specified normal simple value.
 *
 * @return true - if the completion type is normal and
 *                value contains specified simple ecma-value,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_normal_simple_value (const ecma_completion_value_t& value, /**< completion value */
                                              ecma_simple_value_t simple_value) /**< simple value to check
                                                                                     for equality with */
{
  return (ecma_is_completion_value_normal (value)
          && ecma_get_value_type_field (value) == ECMA_TYPE_SIMPLE
          && ecma_get_value_value_field (value) == simple_value);
} /* ecma_is_completion_value_normal_simple_value */

/**
 * Check if the completion value is normal true.
 *
 * @return true - if the completion type is normal and
 *                value contains ecma-true simple value,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_normal_true (const ecma_completion_value_t& value) /**< completion value */
{
  return ecma_is_completion_value_normal_simple_value (value, ECMA_SIMPLE_VALUE_TRUE);
} /* ecma_is_completion_value_normal_true */

/**
 * Check if the completion value is normal false.
 *
 * @return true - if the completion type is normal and
 *                value contains ecma-false simple value,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_normal_false (const ecma_completion_value_t& value) /**< completion value */
{
  return ecma_is_completion_value_normal_simple_value (value, ECMA_SIMPLE_VALUE_FALSE);
} /* ecma_is_completion_value_normal_false */

/**
 * Check if the completion value is normal empty value.
 *
 * @return true - if the completion type is normal and
 *                value contains empty simple value,
 *         false - otherwise.
 */
inline bool __attribute_const__ __attribute_always_inline__
ecma_is_completion_value_empty (const ecma_completion_value_t& value) /**< completion value */
{
  if (ecma_is_completion_value_normal (value))
  {
    ecma_value_t v ((ecma_value_packed_t) value);

    return ecma_is_value_empty (v);
  }
  else
  {
    return false;
  }
} /* ecma_is_completion_value_empty */

extern void ecma_check_value_type_is_spec_defined (const ecma_value_t& value);

/**
 * @}
 * @}
 */

#endif /* !ECMA_VALUE_H */