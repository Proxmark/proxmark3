# How to configure continuous integration

Here 2 CI configuration files:

1. for [travis](travis-ci.org)
2. for [appveyor](appveyor.com)

It needs to put files from this directory to repository root and then configure CI from appropriate WEB portal.

## travis

- Copy .travis.yml and travis_test_commands.scr files to repository root 
- Configure CI from http://travis-ci.org
- It needs to fork https://github.com/Proxmark/homebrew-proxmark3 from your proxmark repository home
- Put to file `proxmark3.rb` in line `head "https://github.com/proxmark/proxmark3.git"` your repository link. As sample: `head "https://github.com/merlokk/proxmark3.git"`


## appveyor

- Just copy appveyor.yml file to root and configure it from http://appveyor.com
