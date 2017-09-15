/*
** mrb_lockfile.c - Lockfile class
**
** Copyright (c) Uchio Kondo 2017
**
** See Copyright Notice in LICENSE
*/

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
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
  pid_t pid; /* unused for Lockfile */
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
  mrb_int len, mode = (mrb_int)0666;

  DATA_TYPE(self) = &mrb_lockfile_data_type;
  mrb_get_args(mrb, "s|i", &str, &len, &mode);
  data = (mrb_lockfile_data *)mrb_malloc(mrb, sizeof(mrb_lockfile_data));
  data->path = mrb_malloc(mrb, (size_t)len + 1);
  strncpy(data->path, str, (size_t)len + 1);
  data->fd = open(data->path, O_WRONLY | O_CREAT | O_CLOEXEC | O_NONBLOCK, (mode_t)mode);
  if (data->fd < 0) {
    mrb_sys_fail(mrb, "initial open");
  }
  data->pid = -1;

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

static void mrb__get_flock(mrb_state *mrb, int fd, struct flock *f)
{
  if (fcntl(fd, F_GETLK, f) < 0) {
    if (errno != EACCES && errno != EAGAIN)
      mrb_raise(mrb, E_RUNTIME_ERROR, "cannot get lock info");
  }
}

static mrb_value mrb_lockfile_is_locked(mrb_state *mrb, mrb_value self)
{
  struct flock f = {.l_type = 0};
  mrb_lockfile_data *data = DATA_PTR(self);
  mrb__get_flock(mrb, data->fd, &f);
  return mrb_bool_value((mrb_bool)(f.l_type != F_UNLCK));
}

static mrb_value mrb_lockfile_locking_pid(mrb_state *mrb, mrb_value self)
{
  struct flock f = {.l_type = 0};
  mrb_lockfile_data *data = DATA_PTR(self);
  mrb__get_flock(mrb, data->fd, &f);
  if (f.l_type != F_UNLCK) {
    return mrb_fixnum_value(f.l_pid);
  } else {
    return mrb_nil_value();
  }
}

static mrb_value mrb_lockfile_path(mrb_state *mrb, mrb_value self)
{
  mrb_lockfile_data *data = DATA_PTR(self);
  return mrb_str_new_cstr(mrb, data->path);
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

/* Pidfile: Lockfile's subclass to control pidfile */
static mrb_value mrb_pidfile_write(mrb_state *mrb, mrb_value self)
{
  pid_t cur = getpid();
  size_t len = 2;
  char *buf;
  int wk = (int)cur, wk2, detect_fail = 0;
  while ((wk = wk / 10) != 0)
    len++;

  buf = mrb_malloc(mrb, len);
  snprintf(buf, len, "%d", cur);

  mrb_lockfile_data *data = DATA_PTR(self);

  if (ftruncate(data->fd, 0) < 0)
    mrb_sys_fail(mrb, "mrb_pidfile_write: ftruncate");

  wk = 0;
  while (wk < len) {
    wk2 = write(data->fd, buf, len);
    if (wk2 < 0)
      detect_fail = 1;
    wk += wk2;
  }
  mrb_free(mrb, buf);
  if (detect_fail)
    mrb_sys_fail(mrb, "mrb_pidfile_write: write");

  if (fsync(data->fd) < 0)
    mrb_sys_fail(mrb, "mrb_pidfile_write: fsync");

  data->pid = cur;
  return mrb_fixnum_value(cur);
}

static mrb_value mrb_pidfile_pid(mrb_state *mrb, mrb_value self)
{
  struct flock f = {.l_type = 0};
  mrb_lockfile_data *data = DATA_PTR(self);
  mrb__get_flock(mrb, data->fd, &f);
  if (f.l_type == F_UNLCK) {
    if (data->pid < 0) {
      data->pid = getpid();
    }
    return mrb_fixnum_value(data->pid);
  }

  {
#define PID_BUF_MAX 16
    int pid, readfd;
    char buf[PID_BUF_MAX];
    memset(buf, 0, PID_BUF_MAX);

    readfd = open(data->path, O_RDONLY);
    if (read(readfd, buf, PID_BUF_MAX) < 0)
      mrb_sys_fail(mrb, "mrb_pidfile_pid: read");
    close(readfd);

    pid = strtol(buf, NULL, 0);
    if (pid > 0) {
      return mrb_fixnum_value(pid);
    } else {
      return mrb_nil_value();
    }
  }
}

void mrb_mruby_lockfile_gem_init(mrb_state *mrb)
{
  struct RClass *lockfile, *pidfile;
  lockfile = mrb_define_class(mrb, "Lockfile", mrb->object_class);
  MRB_SET_INSTANCE_TT(lockfile, MRB_TT_DATA);
  mrb_define_method(mrb, lockfile, "initialize", mrb_lockfile_init, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, lockfile, "lock", mrb_lockfile_do_lock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "lockwait", mrb_lockfile_lockwait, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "locked?", mrb_lockfile_is_locked, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "unlock", mrb_lockfile_do_unlock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "trylock", mrb_lockfile_trylock, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "utime", mrb_lockfile_utime, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "locking_pid", mrb_lockfile_locking_pid, MRB_ARGS_NONE());
  mrb_define_method(mrb, lockfile, "path", mrb_lockfile_path, MRB_ARGS_NONE());

  mrb_define_class_method(mrb, lockfile, "exist?", mrb_lockfile_exists, MRB_ARGS_REQ(1));

  pidfile = mrb_define_class(mrb, "Pidfile", lockfile);
  mrb_define_method(mrb, pidfile, "write", mrb_pidfile_write, MRB_ARGS_NONE());
  mrb_define_method(mrb, pidfile, "pid", mrb_pidfile_pid, MRB_ARGS_NONE());

  DONE;
}

void mrb_mruby_lockfile_gem_final(mrb_state *mrb)
{
}
