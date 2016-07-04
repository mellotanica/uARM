cask 'uarm' do
  version :latest
  sha256 :no_check

  url 'https://github.com/MartelliEnrico/uARM/archive/master.zip'
  name 'uARM'
  homepage 'http://mellotanica.github.io/uARM/'
  license :oss

  depends_on formula: ['qt5', 'libelf', 'boost']

  installer script: 'compile'
  app 'uarm.app', target: 'uARM.app'
  binary 'uarm.app/Contents/MacOS/uarm'
  binary 'elf2uarm'
  binary 'uarm-mkdev'
  artifact 'icons', target: '/usr/local/lib/uarm/icons'
  artifact 'facilities', target: '/usr/local/include/uarm'
  artifact 'ldscript', target: '/usr/local/include/uarm/ldscripts'
  artifact 'test', target: '/usr/local/share/doc/uarm/examples'
  artifact 'facilities/readuarm.py', target: '/usr/local/bin/uarm-readuarm'
  artifact 'default/uarm', target: '/etc/default/uarm'

  caveats do
    files_in_usr_local

    print "You need to install an ARM cross-compiler to build ARM executable."
    print "You can find it at this link: https://launchpad.net/gcc-arm-embedded/+download"
  end
end