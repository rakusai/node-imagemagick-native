#! /usr/bin/env node

var fs   = require('fs');
var path = require('path');
var im   = require(path.join(__dirname, 'build', 'Debug', 'imagemagick.node'));
var log  = console.log;

fs.readFile(path.join(__dirname, 'test', 'test.jpg'), function (e, file) {
  log('step 1');
  if (e)
    throw e;
  log('step 1.5');
  im.convert({srcData: file, width: 100, height: 100, debug: 1}, function (e, res) {
    log('step 2');
    if (e)
      throw e;
    fs.writeFile(path.join(__dirname, 'test.out.jpg'), res, function (e) {
      log('step 3');
      if (e)
        throw e;
    });
  });
});
