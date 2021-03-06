import { realType, typeType } from "./analyzer_common.js";

function isArgument(type, node) {
    return node["arguments"].includes(type);
}

function isParentField(type, context, ast) {
    return context["base"] && isField(type, ast[context["base"]], ast);
}

function isField(type, context, ast) {
    return (
        context["type"] == "object" &&
        (context["fields"].includes(type) || isParentField(type, context, ast))
    );
}

function isArrayType(type, context) {
    return context["type"] == "array" && context["arrayType"] == type;
}

function isVariant(type, context) {
    return context["type"] == "variant" && context["variants"].includes(type);
}

function expressionType(expression, node, context, ast) {
    if (context) {
        if (isField(expression["value"], context, ast)) {
            return "field";
        }

        if (isArrayType(expression["value"], context)) {
            return "arrayType";
        }

        if (isVariant(expression["value"], context)) {
            return "variant";
        }
    }

    if (!expression["parent"]) {
        if (isArgument(expression["value"], node)) {
            return "argument";
        }

        if (!isNaN(expression["value"])) {
            return "number";
        }

        return "new";
    }

    throw `The '${expression["value"]}' cannot be accessed via '${expression["parent"]["value"]}'.`;
}

function expressionContext(expression, object, ast) {
    const parent = expression["parent"];

    if (parent) {
        return ast[parent["value"]];
    }

    return object;
}

function analyzeExpressionPart(expression, node, object, ast) {
    const context = expressionContext(expression, object, ast);

    if (expression["parent"]) {
        analyzeExpressionPart(expression["parent"], node, object, ast);
    }

    expression["type"] = expressionType(expression, node, context, ast);
}

function analyzeReturn(expression, node, object, ast) {
    const context = expressionContext(expression, object, ast);

    if (expression["parent"]) {
        analyzeExpressionPart(expression["parent"], node, object, ast);
    }

    expression["returnType"] = expressionType(expression, node, context, ast);
}

function isRightVariantOfLeft(left, right, ast) {
    return (
        ast[left] &&
        "variants" in ast[left] &&
        ast[left]["variants"].includes(right)
    );
}

function validateAssignment(expression, ast) {
    const leftType = realType(expression["left"]["value"], ast);
    const leftTypeType = typeType(leftType, ast);
    const rightType = realType(expression["right"]["value"], ast);
    const rightTypeType = typeType(rightType, ast);

    if (leftType == rightType) {
        return;
    }

    if (leftTypeType == "native" && leftTypeType == rightTypeType) {
        return;
    }

    if (isRightVariantOfLeft(leftType, rightType, ast)) {
        return;
    }

    throw `Cannot assign '${expression["right"]["value"]}' (${rightTypeType}) to '${expression["left"]["value"]}' (${leftTypeType}).`;
}

function analyzeAssignment(expression, node, object, ast) {
    validateAssignment(expression, ast);
    analyzeExpressionPart(expression["left"], node, object, ast);
    analyzeExpressionPart(expression["right"], node, object, ast);
}

export function analyzeExpression(expression, node, object, ast) {
    switch (expression["type"]) {
        case "return":
            analyzeReturn(expression, node, object, ast);
            break;
        case "assignment":
            analyzeAssignment(expression, node, object, ast);
            break;
        default:
            throw `Unknown expression type '${expression["type"]}'.`;
    }
}
