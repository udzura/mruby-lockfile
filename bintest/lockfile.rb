require 'open3'

BIN_PATH = File.join(File.dirname(__FILE__), "../mruby/bin/mruby") unless defined?(BIN_PATH)

assert('lock from another process') do
  system "mkdir -p tmp"
  system %Q(#{BIN_PATH} -e "Lockfile.lock './tmp/test101.lock'; loop {}" &)
  sleep 0.5
  output, err, status = Open3.capture3(BIN_PATH, "-e", "Lockfile.lock './tmp/test101.lock'")

  assert_false status.success?
  assert_true err.include? 'cannot set lock'

  system "killall mruby"
  system "rm -f './tmp/test101.lock'"
end

assert('lockwait for another lock released') do
  system "mkdir -p tmp"
  system %Q(#{BIN_PATH} -e "Lockfile.lock './tmp/test102.lock'; sleep 1" &)
  sleep 0.5
  output, err, status = Open3.capture3(BIN_PATH, "-e", "Lockfile.lockwait './tmp/test102.lock'")

  assert_true status.success?

  system "killall mruby 2>/dev/null || true"
  system "rm -f './tmp/test102.lock'"
end

assert('trylock output') do
  system "mkdir -p tmp"
  output, err, status = Open3.capture3(BIN_PATH, "-e", "l = Lockfile.new './tmp/test103.lock'; puts l.trylock.to_s")
  assert_equal "true", output.chomp

  system %Q(#{BIN_PATH} -e "Lockfile.lock './tmp/test103.lock'; loop {}" &)
  sleep 0.5
  output, err, status = Open3.capture3(BIN_PATH, "-e", "l = Lockfile.new './tmp/test103.lock'; puts l.trylock.to_s")
  assert_equal "false", output.chomp

  system "killall mruby"
  system "rm -f './tmp/test103.lock'"
end

assert('lockwait with unlock') do
  system "mkdir -p tmp"
  system %Q(#{BIN_PATH} -e "l = Lockfile.lock './tmp/test104.lock'; sleep 1; l.unlock; loop {}" &)
  sleep 0.5
  output, err, status = Open3.capture3(BIN_PATH, "-e", "Lockfile.lockwait './tmp/test104.lock'")

  assert_true status.success?

  system "killall mruby"
  system "rm -f './tmp/test102.lock'"
end
