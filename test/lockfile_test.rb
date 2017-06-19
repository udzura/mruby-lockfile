##
## Lockfile Test
##

assert("Lockfile#hello") do
  t = Lockfile.new "hello"
  assert_equal("hello", t.hello)
end

assert("Lockfile#bye") do
  t = Lockfile.new "hello"
  assert_equal("hello bye", t.bye)
end

assert("Lockfile.hi") do
  assert_equal("hi!!", Lockfile.hi)
end
