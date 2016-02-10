// Executes `before_expr` at the beginning of the following scope and `after_expr` when the scope is exited.
//
// Usage:
//
// SE350_SCOPE(__disable_irq(), __enable_irq()) {
//     // no interrupts
// }
//
// Note: `break` and `continue` will affect this scope, not an outer scope.
// Note: `return` will not evaluate the end-of-scope expression.
// Credit: https://gustedt.wordpress.com/2010/08/14/scope-bound-resource-management-with-for-scopes/
//
// We use a variable, _se350_loop_has_run, that is local to the loop to ensure that the loop runs exactly once.
// Since the loop condition is evaluated before running the loop, we evaluate `before_expr` here, but only if the loop has not run.
// Since the loop increment expression is evaluated after running the loop, we evaluate `after_expr` here and make sure the loop does not run again.
// Finally, we have an inner loop so that a `break` in the scope will not skip evaluation of `after_expr`.
#define SE350_SCOPE(before_expr, after_expr) \
    for (_Bool _se350_loop_has_run = 0; _se350_loop_has_run ? 0 : ((void)(before_expr), 1); ((void)(after_expr), _se350_loop_has_run = 1)) \
        for (; !_se350_loop_has_run; _se350_loop_has_run = 1)

// Executes `after_expr` when the scope is exited.
//
// Usage:
//
// int* p = request_memory_block();
// SE350_SCOPE_EXIT(release_memory_block(p)) {
//     // use p
// }
//
// Note: `break` and `continue` will affect this scope, not an outer scope.
// Note: `return` will not evaluate the end-of-scope expression.
// Credit: https://gustedt.wordpress.com/2010/08/14/scope-bound-resource-management-with-for-scopes/
//
// We simply evaluate 0 as the before expr.
#define SE350_SCOPE_EXIT(after_expr) SE350_SCOPE(0, after_expr)

// Declares a variable using `decl`, without a semicolon, and executes `after_expr` when the scope is exited.
//
// Usage:
//
// SE350_USING(int* p = request_memory_block(), release_memory_block(p)) {
//     // use p
// }
//
// Note: `break` and `continue` will affect this scope, not an outer scope.
// Note: `return` will not evaluate the end-of-scope expression.
// Inspired by https://gustedt.wordpress.com/2010/08/14/scope-bound-resource-management-with-for-scopes/
//
// We dedicate the outer loop to declaring _se350_loop_has_run. The loop allows for the usage above, whereas the usual do-while and if-else don't.
// We then do the given declaration and evaluate `after_expr` once the loop has run once, and also ensure that it does not run again.
// Finally, we have an inner loop so that a `break` in the scope will not skip evaluation of `after_expr`.
#define SE350_USING(decl, after_expr) \
    for (_Bool _se350_loop_has_run = 0; !_se350_loop_has_run;) \
        for (decl; !_se350_loop_has_run; ((void)(after_expr), _se350_loop_has_run = 1)) \
            for (; !_se350_loop_has_run; _se350_loop_has_run = 1)

// Breaks out of a scope used with one of the other scope macros and evaluates the end-of-scope expression.
//
// Usage:
//
// SE350_SCOPE(__disable_irq(), __enable_irq()) {
//     if (earlyCondition) { SE350_SCOPE_BREAK; }
//     // no interrupts
// }
#define SE350_SCOPE_BREAK break

#include "printf.h"
#include "rtx.h"

void log_release(void* p) {
    printf("Freeing pointer\n");
    release_memory_block(p);
}

#define INTERRUPT_FREE_BLOCK SE350_SCOPE(__disable_irq(), __enable_irq())

int main(void) {
    INTERRUPT_FREE_BLOCK {
        // no interrupts
    }

    int* p2 = request_memory_block();
    SE350_SCOPE_EXIT(log_release(p2)) {
        *p2 = 6;
        printf("*p2: %d\n", *p2);
    }

    SE350_USING(int* p3 = request_memory_block(), log_release(p3)) {
        *p3 = 7;
        printf("*p3: %d\n", *p3);
    }

    // Similar to C#'s double-up using
    SE350_USING(int* p4 = request_memory_block(), log_release(p4))
    SE350_USING(int* p5 = request_memory_block(), log_release(p5)) {
        *p4 = 8;
        *p5 = 9;
        printf("(*p4, *p5) = (%d, %d)\n", *p4, *p5);
    }
}
