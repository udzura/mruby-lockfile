/*
** mrb_lockfile.c - Lockfile class
**
** Copyright (c) Uchio Kondo 2017
**
** See Copyright Notice in LICENSE
*/

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/error.h>

#include "mrb_lockfile.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  char *path;
  int fd;
} mrb_lockfile_data;

void mrb_lockfile_free(mrb_state *mrb, void *p)
{
  mrb_lockfile_data *d = (mrb_lockfile_data *)p;
  if (d) {
    if (d->fd > 0)
      close(d->fd);
    if (d->path)
      mrb_free(mrb, d->path);
    mrb_free(mrb, d);
  }
}

static const struct mrb_data_type mrb_lockfile_data_type = {
    "mrb_lockfile_data", mrb_lockfile_free,
};

static mrb_value mrb_lockfile_init(mrb_state *mrb, mrb_value self)
{
  mrb_lockfile_data *data;
  char *str;
  int len, mode = 0666;

  DATA_TYPE(self) = &mrb_lockfile_data_type;
  mrb_get_args(mrb, "s|i", &str, &len, &mode);
  data = (mrb_lockfile_data *)mrb_malloc(mrb, sizeof(mrb_lockfile_data));
  data->path = mrb_malloc(mrb, len + 1);
  strncpy(data->path, str, (size_t)(len + 1));
  data->fd = open(data->path, O_WRONLY | O_CREAT | O_CLOEXEC | O_NONBLOCK, (mode_t)mode);
  if (data->fd < 0) {
    mrb_sys_fail(mrb, "initial open");
  }

  DATA_PTR(self) = data;

  return self;
}

static mrb_value mrb_lockfile_do_lock(mrb_state *mrb, mrb_value self)
{
  struct flock f = {
      .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0,
  };
  mrb_lockfile_data *data = DATA_PTR(self);
  if (fcntl(data->fd, F_SETLK, &f) < 0) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot set lock");
  }

  return mrb_true_value();
}

static mrb_value mrb_lockfile_utime(mrb_state *mrb, mrb_value self)
{
  mrb_lockfile_data *data = DATA_PTR(self);
  if (futimens(data->fd, NULL) < 0) {
    mrb_warn(mrb, "futimens was failed but skip...");
  }

  return mrb_true_value();
}

static mrb_value mrb_lockfile_lockwait(mrb_state *mrb, mrb_value self)
{
  struct flock f = {
      .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0,
  };
  mrb_lockfile_data *data = DATA_PTR(self);
  if (fcntl(data->fd, F_SETLKW, &f) < 0) {
    if (errno == EINTR)
      mrb_raise(mrb, E_RUNTIME_ERROR, "lock wait interrupted");
    else if (errno == EDEADLK)
      mrb_raise(mrb, E_RUNTIME_ERROR, "deadlock detected");
    else
      mrb_raise(mrb, E_RUNTIME_ERROR, "cannot set lock anyway");
  }

  return mrb_true_value();
}

static int mrb__file_is_locked(mrb_state *mrb, int fd)
{
  struct flock f = {
      .l_type = 0,
  };
  if (fcntl(fd, F_GETLK, &f) < 0) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot get lock info");
  }
  return (f.l_type != F_UNLCK);
}

static mrb_value mrb_lockfile_is_locked(mrb_state *mrb, mrb_value self)
{
  mrb_lockfile_data *data = DATA_PTR(self);
  return mrb_bool_value((mrb_bool)mrb__file_is_locked(mrb, data->fd));
}

static mrb_value mrb_lockfile_exists(mrb_state *mrb, mrb_value self)
{
  char *str;
  mrb_get_args(mrb, "z", &str);
  return mrb_bool_value((mrb_bool)(!access(str, F_OK)));
}

static mrb_value mrb_lockfile_do_unlock(mrb_state *mrb, mrb_value self)
{
  struct flock f = {
      .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0,
  };
  mrb_lockfile_data *data = DATA_PTR(self);
  if (fcntl(data->fd, F_SETLK, &f) < 0) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot remove lock");
  }
  if (futimens(data->fd, NULL) < 0) {
    mrb_warn(mrb, "futimens was failed but skip...");
  }

  return mrb_true_value();
}

static mrb_value mrb_lockfile_trylock(mrb_state *mrb, mrb_value self)
{
  struct flock f = {
      .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0,
  };
  mrb_lockfile_data *data = DATA_PTR(self);
  if (fcntl(data->fd, F_SETLK, &f) < 0) {
    if (errno == EACCES || errno == EAGAIN) {
      return mrb_false_value();
    }

    mrb_raise(mrb, E_RUNTIME_ERROR, "unexpected error when locking");
  }
  if (futimens(data->fd, NULL) < 0) {
    mrb_warn(mrb, "futimens was failed but skip...");
  }

  return mrb_true_value();
}

void mrb_mruby_lockfile_gem_init(mrb_state *mrb)
{
  struct RClass *lockfile;
  lockfile = mrb_define_class(mrb, "Lockfile", mrb->object_class);
  MRB_SET_INSTANCE_TT(lockfile, MRB_TT_DATA);
  mrb_define_method(mrb, lockfile, "initialize", mrb_lockfile_init, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, lockfile, "lock", mrb_lockfile_do_lock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "lockwait", mrb_lockfile_lockwait, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "locked?", mrb_lockfile_is_locked, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "unlock", mrb_lockfile_do_unlock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "trylock", mrb_lockfile_trylock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "utime", mrb_lockfile_utime, MRB_ARGS_NONE());

  mrb_define_class_method(mrb, lockfile, "exist?", mrb_lockfile_exists, MRB_ARGS_REQ(1));

  DONE;
}

void mrb_mruby_lockfile_gem_final(mrb_state *mrb)
{
}
