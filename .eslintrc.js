module.export = {
    plugins: ["jest"],
    extends: ["eslint:recommended", "plugin:jest/recommended"],
    env: {
        node: true,
        es6: true
    },
    rules: {
        indent: ["error", 4],
        semi: ["error", "always"],
        "no-cond-assign": ["error", "always"]
    }
};
