# mruby-lockfile   [![Build Status](https://travis-ci.org/udzura/mruby-lockfile.svg?branch=master)](https://travis-ci.org/udzura/mruby-lockfile)

File-based lock utilities.

* `Lockfile` by `fnctl(2)`
* `Pidfile` [WIP]

## install by mrbgems

Add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|
  conf.gem github: 'udzura/mruby-lockfile'
end
```

## example

```ruby
l = Lockfile.new('/var/run/test.lock')
l.lock
# If you want to unlock
l.unlock

## In another process:
l = Lockfile.new('/var/run/test.lock')
l.lock #=> Exception!

l.trylock #=> false if failed

l.lockwait # Block until lock is released
# ...
```

Please see tests.

## License

Under the MIT License:

- see LICENSE file
