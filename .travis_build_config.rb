MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gembox 'default'
  conf.gem '../mruby-lockfile'
  conf.enable_test
end
