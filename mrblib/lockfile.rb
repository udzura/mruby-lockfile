class Lockfile
  def self.lock(path)
    l = new(path)
    l.lock
    return l
  end

  def self.trylock(path)
    l = new(path)
    if l.trylock
      return l
    else
      return false
    end
  end
end
