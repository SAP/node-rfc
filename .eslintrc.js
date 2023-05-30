module.exports = {
    root: true,
    extends: [
        "eslint:recommended",
        "eslint-config-prettier",
        "plugin:@typescript-eslint/eslint-recommended",
        "plugin:@typescript-eslint/recommended",
        "plugin:@typescript-eslint/recommended-requiring-type-checking",
    ],
    plugins: ["@typescript-eslint"],
    parser: "@typescript-eslint/parser",
    parserOptions: {
        ecmaVersion: 2020,
        project: true, // "./tsconfig.json",
        // tsconfigRootDir: __dirname,
    },
    env: {
        es6: true,
        browser: false,
        node: true,
        jest: true,
    },
    rules: {
        "no-console": "error",
        "@typescript-eslint/ban-types": [
            "error",
            {
                types: {
                    Object: false,
                    object: false,
                    Function: false,
                },
                extendDefaults: true,
            },
        ],
    },
};
