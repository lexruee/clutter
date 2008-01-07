/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *             Neil Jagdish Patel <njp@o-hand.com>
 *             Emmanuele Bassi <ebassi@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:clutter-model-default
 * @short_description: Default model implementation
 *
 * #ClutterModelDefault is a #ClutterModel implementation provided by
 * Clutter. #ClutterModelDefault uses a #GSequence for storing the
 * values for each row, so it's optimized for insertion and look up
 * in sorted lists.
 *
 * #ClutterModelDefault is a terminal class: it cannot be subclassed,
 * only instantiated. 
 *
 * #ClutterModelDefault is available since Clutter 0.6
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include "clutter-model.h"
#include "clutter-model-default.h"
#include "clutter-private.h"
#include "clutter-debug.h"

#define CLUTTER_TYPE_MODEL_DEFAULT_ITER                 \
        (clutter_model_default_iter_get_type())
#define CLUTTER_MODEL_DEFAULT_ITER(obj)                 \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),              \
         CLUTTER_TYPE_MODEL_DEFAULT_ITER,               \
         ClutterModelDefaultIter))
#define CLUTTER_IS_MODEL_DEFAULT_ITER(obj)              \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),              \
         CLUTTER_TYPE_MODEL_DEFAULT_ITER))
#define CLUTTER_MODEL_DEFAULT_ITER_CLASS(klass)         \
        (G_TYPE_CHECK_CLASS_CAST ((klass),              \
         CLUTTER_TYPE_MODEL_DEFAULT_ITER,               \
         ClutterModelDefaultIterClass))
#define CLUTTER_IS_MODEL_DEFAULT_ITER_CLASS(klass)      \
        (G_TYPE_CHECK_CLASS_TYPE ((klass),              \
         CLUTTER_TYPE_MODEL_DEFAULT_ITER))
#define CLUTTER_MODEL_DEFAULT_ITER_GET_CLASS(obj)       \
        (G_TYPE_INSTANCE_GET_CLASS ((obj),              \
         CLUTTER_TYPE_MODEL_DEFAULT_ITER,               \
         ClutterModelDefaultIterClass))

#define CLUTTER_MODEL_DEFAULT_CLASS(klass)              \
        (G_TYPE_CHECK_CLASS_CAST ((klass),              \
         CLUTTER_TYPE_MODEL_DEFAULT,                    \
         ClutterModelDefaultClass))
#define CLUTTER_IS_MODEL_DEFAULT_CLASS(klass)           \
        (G_TYPE_CHECK_CLASS_TYPE ((klass),              \
         CLUTTER_TYPE_MODEL_DEFAULT))
#define CLUTTER_MODEL_DEFAULT_GET_CLASS(obj)            \
        (G_TYPE_INSTANCE_GET_CLASS ((obj),              \
         CLUTTER_TYPE_MODEL_DEFAULT,                    \
         ClutterModelDefaultClass))

typedef struct _ClutterModelDefaultIter ClutterModelDefaultIter;
typedef struct _ClutterModelIterClass   ClutterModelDefaultIterClass;

typedef struct _ClutterModelClass       ClutterModelDefaultClass;

struct _ClutterModelDefault
{
  ClutterModel parent_instance;

  GSequence *sequence;
};

struct _ClutterModelDefaultIter
{
  ClutterModelIter parent_instance;

  GSequenceIter *seq_iter;
};



/*
 * ClutterModelDefault
 */

G_DEFINE_TYPE (ClutterModelDefaultIter,
               clutter_model_default_iter,
               CLUTTER_TYPE_MODEL_ITER);

static void
clutter_model_default_iter_get_value (ClutterModelIter *iter,
                                      guint             column,
                                      GValue           *value)
{
  ClutterModelDefaultIter *iter_default;
  GValueArray *value_array;
  GValue *iter_value;
  GValue real_value = { 0, };
  gboolean converted = FALSE;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  value_array = g_sequence_get (iter_default->seq_iter);
  iter_value = g_value_array_get_nth (value_array, column);
  g_assert (iter_value != NULL);

  if (!g_type_is_a (G_VALUE_TYPE (value), G_VALUE_TYPE (iter_value)))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), 
                                    G_VALUE_TYPE (iter_value)) &&
          !g_value_type_compatible (G_VALUE_TYPE (iter_value), 
                                    G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          return;
        }

      if (!g_value_transform (value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s",
                     G_STRLOC, 
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          g_value_unset (&real_value);
        }

      converted = TRUE;
    }
  
  if (converted)
    {
      g_value_copy (&real_value, value);
      g_value_unset (&real_value);
    }
  else
    g_value_copy (iter_value, value);
}

static void
clutter_model_default_iter_set_value (ClutterModelIter *iter,
                                      guint             column,
                                      const GValue     *value)
{
  ClutterModelDefaultIter *iter_default;
  GValueArray *value_array;
  GValue *iter_value;
  GValue real_value = { 0, };
  gboolean converted = FALSE;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  value_array = g_sequence_get (iter_default->seq_iter);
  iter_value = g_value_array_get_nth (value_array, column);
  g_assert (iter_value != NULL);

  if (!g_type_is_a (G_VALUE_TYPE (value), G_VALUE_TYPE (iter_value)))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), 
                                    G_VALUE_TYPE (iter_value)) &&
          !g_value_type_compatible (G_VALUE_TYPE (iter_value), 
                                    G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s\n",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          return;
        }

      if (!g_value_transform (value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s\n",
                     G_STRLOC, 
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          g_value_unset (&real_value);
        }

      converted = TRUE;
    }
 
  if (converted)
    {
      g_value_copy (&real_value, iter_value);
      g_value_unset (&real_value);
    }
  else
    g_value_copy (value, iter_value);
}

static gboolean
clutter_model_default_iter_is_first (ClutterModelIter *iter)
{
  ClutterModelDefaultIter *iter_default;
  ClutterModel *model;
  ClutterModelIter *temp_iter;
  GSequenceIter *begin, *end;
  guint row;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter);

  begin = g_sequence_get_begin_iter (CLUTTER_MODEL_DEFAULT (model)->sequence);
  end   = iter_default->seq_iter;

  temp_iter = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (begin))
    {
      CLUTTER_MODEL_DEFAULT_ITER (temp_iter)->seq_iter = begin;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        {
          end = begin;
          break;
        }

      begin = g_sequence_iter_next (begin);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* This is because the 'begin_iter' is always *before* the last valid
   * iter, otherwise we'd have endless loops 
   */
  end = g_sequence_iter_prev (end);

  return iter_default->seq_iter == end;
}

static gboolean
clutter_model_default_iter_is_last (ClutterModelIter *iter)
{
  ClutterModelDefaultIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model;
  GSequenceIter *begin, *end;
  guint row;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  if (g_sequence_iter_is_end (iter_default->seq_iter))
    return TRUE;

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter);

  begin = g_sequence_get_end_iter (CLUTTER_MODEL_DEFAULT (model)->sequence);
  begin = g_sequence_iter_prev (begin);
  end   = iter_default->seq_iter;

  temp_iter = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (begin))
    {
      CLUTTER_MODEL_DEFAULT_ITER (temp_iter)->seq_iter = begin;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        {
          end = begin;
          break;
        }

      begin = g_sequence_iter_prev (begin);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* This is because the 'end_iter' is always *after* the last valid iter.
   * Otherwise we'd have endless loops 
   */
  end = g_sequence_iter_next (end);

  return iter_default->seq_iter == end;
}

static ClutterModelIter *
clutter_model_default_iter_next (ClutterModelIter *iter)
{
  ClutterModelDefaultIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model = NULL;
  GSequenceIter *filter_next;
  guint row;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter) + 1;

  filter_next = g_sequence_iter_next (iter_default->seq_iter);
  g_assert (filter_next != NULL);

  temp_iter = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_end (filter_next))
    {
      CLUTTER_MODEL_DEFAULT_ITER (temp_iter)->seq_iter = filter_next;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        break;

      filter_next = g_sequence_iter_next (filter_next);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* We do this because the 'end_iter' is always *after* the last valid iter.
   * Otherwise loops will go on forever
   */
  if (filter_next == iter_default->seq_iter)
    filter_next = g_sequence_iter_next (filter_next);

  /* update the iterator and return it */
  g_object_set (G_OBJECT (iter_default), "model", model, "row", row, NULL);
  iter_default->seq_iter = filter_next;

  return CLUTTER_MODEL_ITER (iter_default);
}

static ClutterModelIter *
clutter_model_default_iter_prev (ClutterModelIter *iter)
{
  ClutterModelDefaultIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model;
  GSequenceIter *filter_prev;
  guint row;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter) - 1;

  filter_prev = g_sequence_iter_prev (iter_default->seq_iter);
  g_assert (filter_prev != NULL);

  temp_iter = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (filter_prev))
    {
      CLUTTER_MODEL_DEFAULT_ITER (temp_iter)->seq_iter = filter_prev;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        break;

      filter_prev = g_sequence_iter_prev (filter_prev);
      row -= 1;
    }

  g_object_unref (temp_iter);

  /* We do this because the 'end_iter' is always *after* the last valid iter.
   * Otherwise loops will go on forever
   */
  if (filter_prev == iter_default->seq_iter)
    filter_prev = g_sequence_iter_prev (filter_prev);

  /* update the iterator and return it */
  g_object_set (G_OBJECT (iter_default), "model", model, "row", row, NULL);
  iter_default->seq_iter = filter_prev;

  return CLUTTER_MODEL_ITER (iter_default);
}
static void
clutter_model_default_iter_class_init (ClutterModelDefaultIterClass *klass)
{
  ClutterModelIterClass *iter_class = CLUTTER_MODEL_ITER_CLASS (klass);

  iter_class->get_value = clutter_model_default_iter_get_value;
  iter_class->set_value = clutter_model_default_iter_set_value;
  iter_class->is_first  = clutter_model_default_iter_is_first;
  iter_class->is_last   = clutter_model_default_iter_is_last;
  iter_class->next      = clutter_model_default_iter_next;
  iter_class->prev      = clutter_model_default_iter_prev;
}

static void
clutter_model_default_iter_init (ClutterModelDefaultIter *iter)
{
  iter->seq_iter = NULL;
}

/*
 * ClutterModelDefault
 */

G_DEFINE_TYPE (ClutterModelDefault, clutter_model_default, CLUTTER_TYPE_MODEL);

static ClutterModelIter *
clutter_model_default_get_iter_at_row (ClutterModel *model,
                                       guint         row)
{
  ClutterModelDefault *model_default = CLUTTER_MODEL_DEFAULT (model);
  ClutterModelDefaultIter *retval;

  if (row >= g_sequence_get_length (model_default->sequence))
    return NULL;

  retval = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                         "model", model,
                         "row", row,
                         NULL);
  retval->seq_iter = g_sequence_get_iter_at_pos (model_default->sequence, row);

  return CLUTTER_MODEL_ITER (retval);
}

static ClutterModelIter *
clutter_model_default_insert_row (ClutterModel *model,
                                  gint          index_)
{
  ClutterModelDefault *model_default = CLUTTER_MODEL_DEFAULT (model);
  ClutterModelDefaultIter *retval;
  guint n_columns, i, pos;
  GValueArray *array;
  GSequenceIter *seq_iter;

  n_columns = clutter_model_get_n_columns (model);
  array = g_value_array_new (n_columns);

  for (i = 0; i < n_columns; i++)
    {
      GValue *value = NULL;

      g_value_array_append (array, NULL);

      value = g_value_array_get_nth (array, i);
      g_value_init (value, clutter_model_get_column_type (model, i));
    }

  if (index_ < 0)
    {
      seq_iter = g_sequence_append (model_default->sequence, array);
      pos = g_sequence_get_length (model_default->sequence);
    }
  else if (index_ == 0)
    {
      seq_iter = g_sequence_prepend (model_default->sequence, array);
      pos = 0;
    }
  else
    {
      seq_iter = g_sequence_get_iter_at_pos (model_default->sequence, index_);
      seq_iter = g_sequence_insert_before (seq_iter, array);
      pos = index_;
    }

  retval = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                         "model", model,
                         "row", pos,
                         NULL);
  retval->seq_iter = seq_iter;

  return CLUTTER_MODEL_ITER (retval);
}

static void
clutter_model_default_remove_row (ClutterModel *model,
                                  guint         row)
{
  ClutterModelDefault *model_default = CLUTTER_MODEL_DEFAULT (model);
  GSequenceIter *seq_iter;
  guint pos = 0;

  seq_iter = g_sequence_get_begin_iter (model_default->sequence);
  while (!g_sequence_iter_is_end (seq_iter))
    {
      if (clutter_model_filter_row (model, pos))
        {
          if (pos == row)
            {  
              ClutterModelIter *iter;

              iter = g_object_new (CLUTTER_TYPE_MODEL_DEFAULT_ITER,
                                   "model", model,
                                   "row", pos,
                                   NULL);
              CLUTTER_MODEL_DEFAULT_ITER (iter)->seq_iter = seq_iter;

              /* the actual row is removed from the sequence inside
               * the ::row-removed signal class handler, so that every
               * handler connected to ::row-removed will still get
               * a valid iterator, and every signal connected to
               * ::row-removed with the AFTER flag will get an updated
               * model
               */
              g_signal_emit_by_name (model, "row-removed", iter);

              g_object_unref (iter);

              break;
            }
        }

      pos += 1;
      seq_iter = g_sequence_iter_next (seq_iter);
    }
}

static guint
clutter_model_default_get_n_rows (ClutterModel *model)
{
  ClutterModelDefault *model_default = CLUTTER_MODEL_DEFAULT (model);

  return g_sequence_get_length (model_default->sequence);
}

typedef struct
{
  ClutterModel *model;
  guint column;
  ClutterModelSortFunc func;
  gpointer data;
} SortClosure;

static gint
sort_model_default (gconstpointer a,
                    gconstpointer b,
                    gpointer      data)
{
  GValueArray *row_a = (GValueArray *) a;
  GValueArray *row_b = (GValueArray *) b;
  SortClosure *clos = data;

  return clos->func (clos->model,
                     g_value_array_get_nth (row_a, clos->column),
                     g_value_array_get_nth (row_b, clos->column),
                     clos->data);
}

static void
clutter_model_default_resort (ClutterModel         *model,
                              ClutterModelSortFunc  func,
                              gpointer              data)
{
  SortClosure sort_closure = { NULL, 0, NULL, NULL };

  sort_closure.model  = model;
  sort_closure.column = clutter_model_get_sorting_column (model);
  sort_closure.func   = func;
  sort_closure.data   = data;

  g_sequence_sort (CLUTTER_MODEL_DEFAULT (model)->sequence,
                   sort_model_default,
                   &sort_closure);
}

static void
clutter_model_default_row_removed (ClutterModel     *model,
                                   ClutterModelIter *iter)
{
  ClutterModelDefaultIter *iter_default;
  GValueArray *array;

  iter_default = CLUTTER_MODEL_DEFAULT_ITER (iter);

  array = g_sequence_get (iter_default->seq_iter);
  g_value_array_free (array);

  g_sequence_remove (iter_default->seq_iter);
  iter_default->seq_iter = NULL;
}

static void
clutter_model_default_finalize (GObject *gobject)
{
  ClutterModelDefault *model = CLUTTER_MODEL_DEFAULT (gobject);
  GSequenceIter *iter;

  iter = g_sequence_get_begin_iter (model->sequence);
  while (!g_sequence_iter_is_end (iter))
    {
      GValueArray *value_array = g_sequence_get (iter);

      g_value_array_free (value_array);
      iter = g_sequence_iter_next (iter);
    }
  g_sequence_free (model->sequence);

  G_OBJECT_CLASS (clutter_model_default_parent_class)->finalize (gobject);
}

static void
clutter_model_default_class_init (ClutterModelDefaultClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterModelClass *model_class = CLUTTER_MODEL_CLASS (klass);

  gobject_class->finalize = clutter_model_default_finalize;

  model_class->get_n_rows      = clutter_model_default_get_n_rows;
  model_class->get_iter_at_row = clutter_model_default_get_iter_at_row;
  model_class->insert_row      = clutter_model_default_insert_row;
  model_class->remove_row      = clutter_model_default_remove_row;
  model_class->resort          = clutter_model_default_resort;

  model_class->row_removed     = clutter_model_default_row_removed;
}

static void
clutter_model_default_init (ClutterModelDefault *model)
{
  model->sequence = g_sequence_new (NULL);
}
