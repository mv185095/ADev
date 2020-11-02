import fs from "fs";
import * as parser from "./src/parser.js";
import * as analyzer from "./src/analyzer.js";
import * as cpp from "./generators/cpp.js";

function main() {
    try {
        const data = JSON.parse(fs.readFileSync("ADb.json"));
        const ast = analyzer.analyze(parser.parse(data));
        cpp.generate(ast);
    } catch (e) {
        console.log(e);
        process.exit(1);
    }
}

main();
