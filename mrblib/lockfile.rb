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

  def inspect
    "#<Lockfile path=#{self.path}>"
  end
end

class Pidfile < Lockfile
  def self.create(path)
    p = new(path)
    p.lock
    p.write
    return p
  end

  def self.trycreate(path)
    p = new(path)
    if p.trylock
      p.write
      return p
    else
      return false
    end
  end

  def self.pidof(path)
    new(path).pid
  end

  def inspect
    "#<Pidfile path=#{self.path}>"
  end
end
