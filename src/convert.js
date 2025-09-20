const fs = require('fs');
const path = require('path');

const htmlFilePath = path.join(__dirname, 'page.html');
const outputFilePath = path.join(__dirname, '..', 'site.txt'); // Save in the parent directory of src

fs.readFile(htmlFilePath, 'utf8', (err, data) => {
    if (err) {
        console.error('Error reading HTML file:', err);
        return;
    }

    // Escape double quotes and newlines for C-style string
    const escapedHtml = data
        .replace(/"/g, '\\"') // Escape double quotes
        .replace(/\n/g, '\\n') // Escape newlines
        .replace(/\r/g, ''); // Remove carriage returns

    const arduinoString = `"${escapedHtml}"`;

    fs.writeFile(outputFilePath, arduinoString, 'utf8', (err) => {
        if (err) {
            console.error('Error writing to site.txt:', err);
            return;
        }
        console.log('Successfully converted page.html to Arduino string and saved to site.txt');
    });
});
