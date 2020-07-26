const Jimp = require('jimp');
const fs = require('fs');

const pathsToConvert = [];

process.argv.forEach((arg) => {
  if (arg.endsWith('.png')) {
    pathsToConvert.push(arg);
  }
})

console.log(pathsToConvert);

pathsToConvert.forEach((path) => {
  Jimp.read(path, (err, img) => {
    if (err) {
      throw err;
    }

    const buffer = Buffer.alloc((img.bitmap.width * img.bitmap.height) / 2);

    let i = 0;
    let holder = 0;
    img.scan(0, 0, img.bitmap.width, img.bitmap.height, function (x, y, idx) {
      var red = this.bitmap.data[idx + 0];
      var green = this.bitmap.data[idx + 1];
      var blue = this.bitmap.data[idx + 2];
      var alpha = this.bitmap.data[idx + 3];

      const whiteVal = ~~((Math.min(254, alpha) / 255) * 8);

      if ((i % 2) === 0) {
        holder = whiteVal;
        holder = (holder << 1);
        if (alpha > 0) {
          holder = (holder | 0x1);
        }

      } else {
        let holder2 = holder << 1;
        if (alpha > 0) {
          holder2 = (holder2 | 0x1);
        }

        holder = holder << 4;
        holder = holder | holder2;

        // push to buffer
        buffer.writeUInt8(holder, i / 2);

        holder = 0;
      }

      i++;
    });

    const newFileName = path.replace('.png', '.bin');

    fs.writeFile(newFileName, buffer);
  });
})
