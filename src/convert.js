const fs = require("fs");
const path = require("path");

const htmlFiles = [{ name: "indexHTML", path: "page.html" }];

const outputFilePath = path.join(__dirname, "htmlData.h");

let outputContent = `#ifndef ARDUINO_H\n#define ARDUINO_H\n#include <Arduino.h>\n#endif\n\n`;

htmlFiles.forEach((file) => {
  const htmlFilePath = path.join(__dirname, file.path);
  try {
    const data = fs.readFileSync(htmlFilePath, "utf8");

    // Escape double quotes and newlines for C-style string
    const escapedHtml = data
      .replace(/"/g, '\\"') // Escape double quotes
      .replace(/\n/g, "\\n") // Escape newlines
      .replace(/\r/g, "") // Remove carriage returns
      .replaceAll(/\s+/g, " "); // Remove spaces

    const arduinoString = `"${escapedHtml}"`;
    outputContent += `const String ${file.name} = F(${arduinoString});\n`;
    console.log(`Successfully converted ${file.path} to Arduino string.`);
  } catch (err) {
    console.error(`Error processing ${file.path}:`, err);
  }
});

try {
  fs.writeFileSync(outputFilePath, outputContent, "utf8");
  console.log("Successfully wrote all HTML data to htmlData.h");
} catch (err) {
  console.error("Error writing to htmlData.h:", err);
}
