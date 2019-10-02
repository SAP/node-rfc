module.exports = {
	plugins: ['mocha'],
	extends: 'eslint:recommended',
	env: {
		node: true,
		es6: true,
		mocha: true,
	},
	rules: {
		'mocha/no-exclusive-tests': 'error',
		indent: ['error', 4],
		semi: ['error', 'always'],
		'no-cond-assign': ['error', 'always'],
	},
};
