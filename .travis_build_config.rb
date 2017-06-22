MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gembox 'default'
  conf.gem '../mruby-lockfile'
  conf.enable_debug
  conf.enable_test

  conf.gem mgem: 'mruby-process'
  conf.gem mgem: 'mruby-io'
end
