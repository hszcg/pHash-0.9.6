# pHash-0.9.6 patched version

pHash-0.9.6 was released at 2013-04-23, but there are serveral problems for certain usage

Based on [yftzeng's work](https://github.com/yftzeng/pHash-0.9.6-patches) for two patches:

1. Compatible with new PHP version (Compile/Install passed).
2. Change `ph_dct_imagehash()` function to return hash string for further operation.

And in addition, I added two patches for ffmpeg 2.3+ compatibility, which is also for the [ruby binding](https://github.com/toy/pHash) compatibility 

1. Add init NULL value for pFormatCtx (Fixed Video hash)
2. extern "C" for all functions in audiophash.h (Fixed Audio hash)

## Installation

### Ubuntu

#### 1. pHash

```
$ sudo apt-get install libavformat-dev libmpg123-dev libsamplerate-dev libsndfile-dev
$ sudo apt-get install cimg-dev libavcodec-dev ffmpeg libswscale-dev
```

```
$ cd pHash-0.9.6
$ ./configure
$ make && sudo make install
```

#### 2. Ruby binding

https://github.com/toy/pHash

```
gem install pHash
```

#### 3. PHP binding

```
$ cd bindings/php
$ phpize
$ ./configure LIBS="-lpthread"
$ make && sudo make install
```

If you have PHP 5:
```
$ sudo vi /etc/php5/conf.d/pHash.so
extension=pHash.so
```

```
$ sudo php5enmod pHash
$ php -m
pHash
```

If you have PHP 7:
```
sudo echo "extension=pHash.so" | sudo tee /etc/php/7.0/mods-available/pHash.ini
sudo phpenmod pHash
php -m
# you should see pHash in the list
```



## Reference

1. https://github.com/lucidix/phash/commit/5be2d454c932152e9b2395e21f97a008c6bd8766
2. https://github.com/eaccelerator/eaccelerator/issues/33
3. http://serverfault.com/questions/491730/compile-phash-on-centos-php-extension
4. http://stackoverflow.com/q/23414036/96823

## License

Same as [pHash](http://www.phash.org/), [GPL-3.0](http://www.gnu.org/licenses/gpl-3.0.html).
