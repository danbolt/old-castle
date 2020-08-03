
const fs = require('fs');

const FileNames = [
  'foyer_dialogues'
];

const CFilePrefix = '#include "dialogueline.h"\n\n';
const StructName = 'DialogueLine';
const NewLine = '\n';

const processData = (data) => {
  let output = '';
  const dataName = data.name;
  const listing = [];

  output += CFilePrefix;

  data.dialogues.forEach((dialogue, i) => {
    const dialogueName = dataName + '_' + i +'_';

    let declarationOutput = '';
    let definitionOutput = '';
    dialogue.lines.forEach((line, lineIndex) => {
      const itemName = dialogueName + lineIndex;
      const nextItemName = (lineIndex === dialogue.lines.length - 1) ? '0x0' : ('&' + dialogueName + (lineIndex + 1));

      if (lineIndex === 0) {
        listing.push(itemName);
      }

      declarationOutput += StructName + ' ' + itemName + ';' + NewLine;
      definitionOutput += StructName + ' ' + itemName + ' = { "' + line.text.replace(/\n/g, '\\n') + '", ' + nextItemName + ' };' + NewLine;
    });
    output += declarationOutput;
    output += definitionOutput;

    output += NewLine;
  });

  output += NewLine + NewLine;

  output += StructName + '* ' + dataName + '[' + listing.length + '] = { ';

  listing.forEach((listHeadName, i) => {
    console.log(listHeadName);
    output += '&';
    output += listHeadName;
    output += (i === (listing.length - 1)) ? '' : ', ';
  })

  output += ' };';

  output += NewLine;

  output += 'const int ' + dataName + '_count = ' + listing.length + ';' + NewLine;

  output += NewLine;

  return output;
};

FileNames.forEach((filename) => {
  const path = './' + filename + '.json';
  const outputPath = '../' + filename + '.c';
  console.log('opening ' + path + '...');

  fs.readFile(path, (error, content) => {
    if (error) { throw error; }

    const data = JSON.parse(content.toString());
    console.log('parsing ' + path + '...');

    const processedData = processData(data);

    console.log('writing data to' + outputPath + '...');

    fs.writeFile(outputPath, processedData, (error) => {
      if (error) { throw error; }

      console.log('written ' + outputPath + ' to disk!');
    });
  });
});


//console.log(processData(dummyData));
