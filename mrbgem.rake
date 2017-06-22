MRuby::Gem::Specification.new('mruby-lockfile') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Uchio Kondo'

  spec.add_test_dependency 'mruby-print'
  spec.add_test_dependency 'mruby-process'
  spec.add_test_dependency 'mruby-io'
end
