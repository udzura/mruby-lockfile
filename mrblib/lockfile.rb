class Lockfile
  def self.lock(path)
    l = new(path)
    l.lock
    return l
  end

  def self.locked?(path)
    new(path).locked?
  end

  def self.lockable?(path)
    !locked?(path)
  end

  def self.lockwait(path)
    l = new(path)
    l.lockwait
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
