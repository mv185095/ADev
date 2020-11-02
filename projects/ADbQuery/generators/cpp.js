import fs from "fs";

function cppType(type) {
    switch (type) {
        case "byte":
            return "char";
        case "int64":
            return "std::int64_t";
        case "string":
            return "std::string";
        default:
            return type;
    }
}

function generateAlias(type, ast) {
    return `using ${type} = ${cppType(ast[type]["aliasedType"])};\n`;
}

function generateArray(type, ast) {
    return `using ${type} = std::vector<${cppType(ast[type]["arrayType"])}>;\n`;
}

function generateFunction(type, ast) {
    return "";
}

function objectBase(type, ast) {
    if (ast[type]["base"]) {
        return ` : ${ast[type]["base"]}`;
    } else {
        return "";
    }
}

function objectField(field) {
    return `m${field}`;
}

function objectFields(type, ast) {
    let buffer = "";

    for (const field of ast[type]["fields"]) {
        buffer += `    ${field} ${objectField(field)};\n`;
    }

    return buffer;
}

function generateObject(type, ast) {
    return `\nstruct ${type}${objectBase(type, ast)}
{
${objectFields(type, ast)}};\n`;
}

function generateVariants(type, ast) {
    let variants = [];

    for (const variant of ast[type]["variants"]) {
        if (variant == "Query") {
            variants.push("std::unique_ptr<Query>");
        } else {
            variants.push(variant);
        }
    }

    return variants.join(",\n    ");
}

function generateVariant(type, ast) {
    return `\nusing ${type} = std::variant<
    ${generateVariants(type, ast)}>;\n`;
}

function generateType(type, ast) {
    switch (ast[type]["type"]) {
        case "alias":
            return generateAlias(type, ast);
        case "array":
            return generateArray(type, ast);
        case "function":
            return generateFunction(type, ast);
        case "object":
            return generateObject(type, ast);
        case "variant":
            return generateVariant(type, ast);
        default:
            throw `C++ Generator: unknown type '${ast[type]["type"]}'.`;
    }
}

function generateTypes(ast) {
    let buffer = "";

    for (const type in ast) {
        buffer += generateType(type, ast);
    }

    return buffer;
}

function generateCpp(ast) {
    return `#ifndef ADBQUERY_HPP
#define ADBQUERY_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace adb
{
struct Query;
${generateTypes(ast)}
}

#endif
`;
}

export function generate(ast) {
    fs.writeFileSync("ADbQuery.hpp", generateCpp(ast));
}
