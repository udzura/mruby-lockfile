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

## Performance

Comparing with [mruby-io](https://github.com/iij/mruby-io)'s `File#flock`

```console
$ time ./mruby/bin/mruby -e '10.times{|i| 10000.times {|j| l = Lockfile.lock("/home/vagrant/testpid/#{i}/test.#{j}");l.unlock }}'

## Files absent on start
real    0m1.002s
user    0m0.324s
sys     0m0.656s

## Files existing
real    0m0.588s
user    0m0.224s
sys     0m0.344s
```

```console
$ time ./mruby/bin/mruby -e '10.times{|i| 10000.times {|j| l = File.open("/home/vagrant/testpid/#{i}/test.#{j}", "w");l.flock(File::LOCK_EX);l.flock(File::LOCK_UN) }}'

## Files absent on start
real    0m1.327s
user    0m0.568s
sys     0m0.724s

## Files existing
real    0m1.150s
user    0m0.652s
sys     0m0.476s
```

## License

Under the MIT License:

- see LICENSE file
