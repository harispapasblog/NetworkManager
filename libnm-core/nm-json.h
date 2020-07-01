// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2017, 2018 Red Hat, Inc.
 */

#ifndef __NM_JSON_H__
#define __NM_JSON_H__

#define json_array              nm_json_array
#define json_array_append_new   nm_json_array_append_new
#define json_array_get          nm_json_array_get
#define json_array_size         nm_json_array_size
#define json_delete             nm_json_delete
#define json_dumps              nm_json_dumps
#define json_false              nm_json_false
#define json_integer            nm_json_integer
#define json_integer_value      nm_json_integer_value
#define json_loads              nm_json_loads
#define json_object             nm_json_object
#define json_object_del         nm_json_object_del
#define json_object_get         nm_json_object_get
#define json_object_iter        nm_json_object_iter
#define json_object_iter_key    nm_json_object_iter_key
#define json_object_iter_next   nm_json_object_iter_next
#define json_object_iter_value  nm_json_object_iter_value
#define json_object_key_to_iter nm_json_object_key_to_iter
#define json_object_set_new     nm_json_object_set_new
#define json_object_size        nm_json_object_size
#define json_string             nm_json_string
#define json_string_value       nm_json_string_value
#define json_true               nm_json_true

/*****************************************************************************/

#if WITH_JANSSON

#include <jansson.h>

typedef struct {
	gboolean loaded;
	char       *(*nm_json_dumps)              (const json_t *json, size_t flags);
	const char *(*nm_json_object_iter_key)    (void *iter);
	const char *(*nm_json_string_value)       (const json_t *json);
	int         (*nm_json_array_append_new)   (json_t *json, json_t *value);
	int         (*nm_json_object_del)         (json_t *json, const char *key);
	int         (*nm_json_object_set_new)     (json_t *json, const char *key, json_t *value);
	json_int_t  (*nm_json_integer_value)      (const json_t *json);
	json_t     *(*nm_json_array)              (void);
	json_t     *(*nm_json_array_get)          (const json_t *json, size_t index);
	json_t     *(*nm_json_false)              (void);
	json_t     *(*nm_json_integer)            (json_int_t value);
	json_t     *(*nm_json_loads)              (const char *string, size_t flags, json_error_t *error);
	json_t     *(*nm_json_object)             (void);
	json_t     *(*nm_json_object_get)         (const json_t *json, const char *key);
	json_t     *(*nm_json_object_iter_value)  (void *);
	json_t     *(*nm_json_string)             (const char *value);
	json_t     *(*nm_json_true)               (void);
	size_t      (*nm_json_array_size)         (const json_t *json);
	size_t      (*nm_json_object_size)        (const json_t *json);
	void        (*nm_json_delete)             (json_t *json);
	void       *(*nm_json_object_iter)        (json_t *json);
	void       *(*nm_json_object_iter_next)   (json_t *json, void *iter);
	void       *(*nm_json_object_key_to_iter) (const char *key);
} NMJsonVt;

extern const NMJsonVt *_nm_json_vt_ptr;

const NMJsonVt *_nm_json_vt_init (void);

static inline const NMJsonVt *
_nm_json_vt (void)
{
	const NMJsonVt *vt;

	vt = g_atomic_pointer_get ((gpointer *) &_nm_json_vt_ptr);
	if (G_UNLIKELY (!vt)) {
		vt = _nm_json_vt_init ();
		nm_assert (vt);
	}
	return vt;
}

static inline const NMJsonVt *
nm_json_vt (void)
{
	const NMJsonVt *vt;

	vt = _nm_json_vt();
	return vt->loaded ? vt : NULL;
}

static inline const NMJsonVt *
nm_json_vt_assert (void)
{
	const NMJsonVt *vt;

	vt = _nm_json_vt();
	nm_assert (vt->loaded);
	return vt;
}

const NMJsonVt *nmtst_json_vt_reset (gboolean loaded);

/*****************************************************************************/

#define nm_json_boolean(vt, val) \
	((val) ? (vt)->nm_json_true () : (vt)->nm_json_false ())

static inline void
nm_json_decref (const NMJsonVt *vt, json_t *json)
{
	/* Our ref-counting is not threadsafe, unlike libjansson's. But we never
	 * share one json_t instance between threads, and if we would, we would very likely
	 * wrap a mutex around it. */
	if (   json
	    && json->refcount != (size_t) -1
	    && --json->refcount == 0)
		vt->nm_json_delete (json);
}

/*****************************************************************************/

/* the following are implemented as pure macros in jansson.h.
 * They can be used directly, however, add a nm_json* variant,
 * to make it explict we don't accidentally use jansson ABI. */

#define nm_json_is_boolean(json)                json_is_boolean (json)
#define nm_json_is_integer(json)                json_is_integer (json)
#define nm_json_is_string(json)                 json_is_string (json)
#define nm_json_is_object(json)                 json_is_object (json)
#define nm_json_is_array(json)                  json_is_array (json)
#define nm_json_is_true(json)                   json_is_true (json)
#define nm_json_boolean_value(json)             json_boolean_value (json)
#define nm_json_array_foreach(a, b, c)          json_array_foreach (a, b, c)
#define nm_json_object_foreach(a, b, c)         json_object_foreach (a, b, c)
#define nm_json_object_foreach_safe(a, b, c, d) json_object_foreach_safe (a, b, c, d)

/*****************************************************************************/

/* Added in Jansson v2.7 */
#ifndef json_boolean_value
#define json_boolean_value json_is_true
#endif

/* Added in Jansson v2.8 */
#ifndef json_object_foreach_safe
#define json_object_foreach_safe(object, n, key, value)     \
    for (key = json_object_iter_key(json_object_iter(object)), \
             n = json_object_iter_next(object, json_object_key_to_iter(key)); \
         key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
         key = json_object_iter_key(n), \
             n = json_object_iter_next(object, json_object_key_to_iter(key)))
#endif

NM_AUTO_DEFINE_FCN0 (json_t *, _nm_auto_decref_json, json_decref)
#define nm_auto_decref_json nm_auto(_nm_auto_decref_json)

/*****************************************************************************/

static inline int
nm_jansson_json_as_bool (const json_t *elem,
                         bool *out_val)
{
	if (!elem)
		return 0;

	if (!json_is_boolean (elem))
		return -EINVAL;

	NM_SET_OUT (out_val, json_boolean_value (elem));
	return 1;
}

static inline int
nm_jansson_json_as_int32 (const json_t *elem,
                          gint32 *out_val)
{
	json_int_t v;

	if (!elem)
		return 0;

	if (!json_is_integer (elem))
		return -EINVAL;

	v = json_integer_value (elem);
	if (   v < (gint64) G_MININT32
	    || v > (gint64) G_MAXINT32)
		return -ERANGE;

	NM_SET_OUT (out_val, v);
	return 1;
}

static inline int
nm_jansson_json_as_int (const json_t *elem,
                        int *out_val)
{
	json_int_t v;

	if (!elem)
		return 0;

	if (!json_is_integer (elem))
		return -EINVAL;

	v = json_integer_value (elem);
	if (   v < (gint64) G_MININT
	    || v > (gint64) G_MAXINT)
		return -ERANGE;

	NM_SET_OUT (out_val, v);
	return 1;
}

static inline int
nm_jansson_json_as_string (const json_t *elem,
                           const char **out_val)
{
	if (!elem)
		return 0;

	if (!json_is_string (elem))
		return -EINVAL;

	NM_SET_OUT (out_val, json_string_value (elem));
	return 1;
}

/*****************************************************************************/

#ifdef NM_VALUE_TYPE_DEFINE_FUNCTIONS

#include "nm-glib-aux/nm-value-type.h"

static inline gboolean
nm_value_type_from_json (NMValueType value_type,
                         const json_t *elem,
                         gpointer out_val)
{
	switch (value_type) {
	case NM_VALUE_TYPE_BOOL:   return (nm_jansson_json_as_bool   (elem, out_val) > 0);
	case NM_VALUE_TYPE_INT32:  return (nm_jansson_json_as_int32  (elem, out_val) > 0);
	case NM_VALUE_TYPE_INT:    return (nm_jansson_json_as_int    (elem, out_val) > 0);

	/* warning: this overwrites/leaks the previous value. You better have *out_val
	 * point to uninitialized memory or NULL. */
	case NM_VALUE_TYPE_STRING: return (nm_jansson_json_as_string (elem, out_val) > 0);

	case NM_VALUE_TYPE_UNSPEC:
		break;
	}
	nm_assert_not_reached ();
	return FALSE;
}

#endif /* NM_VALUE_TYPE_DEFINE_FUNCTIONS */

#endif /* WITH_JANSSON */

#endif /* __NM_JSON_H__ */
