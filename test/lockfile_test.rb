##
## Lockfile Test
##

assert("Lockfile.new") do
  system "mkdir -p ./tmp"

  t = Lockfile.new "./tmp/test001.lock"
  assert_equal(Lockfile, t.class)

  system "rm -f ./tmp/test001.lock"
end

assert("Lockfile#lock") do
  system "mkdir -p ./tmp"

  t = Lockfile.new "./tmp/test001.lock"
  assert_true(t.lock)

  system "rm -f ./tmp/test001.lock"
end
