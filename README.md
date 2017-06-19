# mruby-lockfile   [![Build Status](https://travis-ci.org/udzura/mruby-lockfile.svg?branch=master)](https://travis-ci.org/udzura/mruby-lockfile)
Lockfile class
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'udzura/mruby-lockfile'
end
```
## example
```ruby
p Lockfile.hi
#=> "hi!!"
t = Lockfile.new "hello"
p t.hello
#=> "hello"
p t.bye
#=> "hello bye"
```

## License
under the MIT License:
- see LICENSE file
