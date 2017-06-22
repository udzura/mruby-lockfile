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
  assert_true(t.lock) # 2 times

  system "rm -f ./tmp/test001.lock"
end

assert("Lockfile#trylock") do
  system "mkdir -p ./tmp"
  t = Lockfile.new "./tmp/test002.lock"
  assert_true(t.trylock)

  system "rm -f ./tmp/test002.lock"
end
