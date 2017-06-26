assert("Lockfile#path") do
  system "mkdir -p ./tmp"

  t = Lockfile.new "./tmp/test021.lock"
  assert_equal("./tmp/test021.lock", t.path)

  system "rm -f ./tmp/test021.lock"
end
