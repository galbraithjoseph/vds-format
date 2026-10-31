// Single-statement bodies stay unbraced before a conditional.
void beforeConditional(bool ready) { if(ready) run(); while(ready) wait(); }

// A conditional must not duplicate separators for following functions.
#if defined(PERSONAL_STYLE_EXAMPLE)
void conditionalBranch();
#else
void alternativeBranch();
#endif
void afterConditional();

// Single-statement bodies stay unbraced after a conditional.
void afterConditionalBody(bool ready) { if(ready) run(); while(ready) wait(); }

// Preprocessor hashes stay in column 1, including nested conditions.
namespace DirectiveExample {
    #if defined(FEATURE)
        #define FEATURE_VALUE 1
        #if defined(NESTED_FEATURE)
            #pragma message("nested feature")
        #endif
    #else
        #define FEATURE_VALUE 0
    #endif
}
