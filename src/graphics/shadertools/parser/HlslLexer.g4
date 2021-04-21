lexer grammar HlslLexer;

// --- IGNORED TOKENS --- //
Newline: [\n\r] -> skip;
Whitespace: [ \t] -> skip;

SingleComment: '//' ~Newline* -> skip;
BlockComment: '/*' .*? '*/' -> skip;

// --- PREPROCESSOR --- //
Preprocessor: '#';
Pragma: 'pragma';
VertexDirective: 'vertex';
FragmentDirective: 'fragment';
FeatureDirective: 'feature';
