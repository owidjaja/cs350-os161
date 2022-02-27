#include "assignment.h"

void
consume_enter(struct resource *resource)
{
    // FILL ME IN

    resource->num_consumers += 1;
}

void
consume_exit(struct resource *resource)
{
    // FILL ME IN

    resource->num_consumers -= 1;
}

void
produce_enter(struct resource *resource)
{
    // FILL ME IN

}

void
produce_exit(struct resource *resource)
{
    // FILL ME IN

}


