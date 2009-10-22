//
//  MAGenerator.h
//  MAGenerator
//
//  Created by Michael Ash on 10/21/09.
//

#import <Cocoa/Cocoa.h>


/* Experimental generator support in Objective-C using blocks
 
 A generator is a special case of a coroutine. A generator looks much like
 a normal function, except that it uses 'yield' instead of 'return'. The
 difference is that when you call a generator again, control resumes after
 the 'yield' statement, rather than at the top.
 
 To make one, start with GENERATOR. The first parameter is the return type
 one the generator (what it returns on each call). Second parameter is the
 name of the generator creator function AND the parameters that it takes.
 Third parameter is the parameters that the generator itself takes, at each
 call. (This is confusing, so look at the examples.)
 
 Next, declare local variables. Variables declared within the block won't
 survive across invocations, so don't use them. Instead, declare variables
 ahead of time here. Because generators are blocks, mutable locals need to
 be __block qualified.
 
 Next, use GENERATOR_BEGIN. Pass it the list of parameters for the
 generator, same as the last parameter to GENERATOR. I wish this
 duplication weren't necessary but I didn't see a way around it.
 
 In the body, use GENERATOR_YIELD to yield values. The compiler won't be
 happy with implicit conversions here, so you may need to explicitly cast
 to the generator return type.
 
 Optionally, use GENERATOR_CLEANUP next. The code that follows this defines
 a cleanup block that runs when the generator is destroyed. This allows you
 to release any __block-qualified object variables, or free any other
 memory allocations.
 
 Finally, use GENERATOR_END to terminate the generator definition.
 
 You can use GENERATOR_DECL in a header file to create a prototype for one.
 
 To use a generator, declare a block of the appropriate generator type. For
 a generator which takes an int and returns an object, you'd declare:
 
 id (^generator)(int);
 
 Then create the generator and assign it:
 
 generator = GeneratorFunction();
 
 Finally, call it at will:
 
 id result = generator(42);
 
 Generators are objects, so manage their memory like you normally would.
 
 There are a few caveats to using this stuff:
 
 - Local variables declared after GENERATOR_BEGIN will not remember their
 values! This includes implicit locals such as are generated by a for/in
 loop in ObjC, so don't use those. (You can get away with it if the body
 of the loop contains no GENERATOR_YIELD calls, but you don't want to
 play with fire here.)
 
 - Non-__block locals can be initialized to hold mutable object references.
 When autoreleased, they'll clean themselves up appropriately. Very handy.
 
 - __block locals pointing to objects need to be retained! The caller could
 conceivably wrap each call to your generator in an autorelease pool. Since
 __block variables aren't automatically retained, you must manage their
 memory manually.
 
 - Since you shouldn't count on your generator being called to completion,
 use GENERATOR_CLEANUP to destroy your retained __block object references
 when they're no longer needed.
 
 - Due to the macro craziness behind GENERATOR_YIELD, you must never, ever
 put more than one call to GENERATOR_YIELD on the same source code line.
 
 */

#define GENERATOR_DECL(returnType, nameAndCreationParams, perCallParams) \
    returnType (^nameAndCreationParams) perCallParams

#define GENERATOR(returnType, nameAndCreationParams, perCallParams) \
    returnType (^nameAndCreationParams) perCallParams \
    { \
        returnType GENERATOR_zeroReturnValue; \
        bzero(&GENERATOR_zeroReturnValue, sizeof(GENERATOR_zeroReturnValue)); \
        returnType (^GENERATOR_cleanupBlock)(void) = nil;

#define GENERATOR_BEGIN(...) \
        __block int GENERATOR_where = -1; \
        NSMutableArray *GENERATOR_cleanupArray = MAGeneratorMakeCleanupArray(); \
        id GENERATOR_mainBlock = ^ (__VA_ARGS__) { \
            [GENERATOR_cleanupArray self]; \
            switch(GENERATOR_where) \
            { \
                case -1:

#define GENERATOR_YIELD(...) \
                    do { \
                        GENERATOR_where = __LINE__; \
                        return __VA_ARGS__; \
                case __LINE__: ; \
                    } while(0)

#define GENERATOR_CLEANUP \
            } \
            return GENERATOR_zeroReturnValue; \
        }; \
        GENERATOR_cleanupBlock = ^{{
        
#define GENERATOR_END \
            } \
            return GENERATOR_zeroReturnValue; \
        }; \
        if(GENERATOR_cleanupBlock) \
            [GENERATOR_cleanupArray addObject: ^{ GENERATOR_cleanupBlock(); }]; \
        return [[GENERATOR_mainBlock copy] autorelease]; \
    }



NSMutableArray *MAGeneratorMakeCleanupArray(void);