assert("Pidfile.new") do
  system "mkdir -p ./tmp"

  t = Pidfile.new "./tmp/test001.pid"
  assert_equal(Pidfile, t.class)

  system "rm -f ./tmp/test001.pid"
end

assert("Pidfile.create") do
  system "mkdir -p ./tmp"

  t = Pidfile.create "./tmp/test001.pid"
  assert_false(t.locked?)
  assert_true(t.pid > 0)
  pid1 = t.pid

  pid2 = File.read "./tmp/test001.pid"
  assert_equal(pid1, pid2.to_i)

  system "rm -f ./tmp/test001.pid"
end
