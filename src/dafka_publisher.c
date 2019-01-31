/*  =========================================================================
    dafka_publisher - class description

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/*
@header
    dafka_publisher -
@discuss
@end
*/

#include "dafka_classes.h"

//  Structure of our class

struct _dafka_publisher_t {
    zsock_t *socket;            // Socket to publish messages to
    const char *topic;          // Name of the topic
    zuuid_t *address;           // Name of this publisher
    int sequence;               // Sequence number for reliable messages
};


//  --------------------------------------------------------------------------
//  Create a new dafka_publisher

dafka_publisher_t *
dafka_publisher_new (char *topic)
{
    dafka_publisher_t *self = (dafka_publisher_t *) zmalloc (sizeof (dafka_publisher_t));
    assert (self);
    //  Initialize class properties here
    self->socket = zsock_new_pub("@tcp://*:*");
    self->topic = topic;
    self->address = zuuid_new ();
    self->sequence = 0;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the dafka_publisher

void
dafka_publisher_destroy (dafka_publisher_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        dafka_publisher_t *self = *self_p;
        //  Free class properties here
        zsock_destroy (&self->socket);
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Publish content

int
dafka_publisher_publish (dafka_publisher_t *self, zframe_t *content) {
    dafka_proto_t *msg = dafka_proto_new ();
    dafka_proto_set_id (msg, DAFKA_PROTO_MSG);
    dafka_proto_set_topic (msg, self->topic);
    dafka_proto_set_content (msg, &content);
    return dafka_proto_send (msg, self->socket);
}


//  --------------------------------------------------------------------------
//  Publish content reliable

int
dafka_publisher_publish_reliable (dafka_publisher_t *self, zframe_t *content) {
    dafka_proto_t *msg = dafka_proto_new ();
    dafka_proto_set_id (msg, DAFKA_PROTO_RELIABLE);
    dafka_proto_set_topic (msg, self->topic);
    dafka_proto_set_address (msg, zuuid_str (self->address));
    dafka_proto_set_sequence (msg, self->sequence++);
    dafka_proto_set_content (msg, &content);
    return dafka_proto_send (msg, self->socket);
}


//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

void
dafka_publisher_test (bool verbose)
{
    printf (" * dafka_publisher: ");

    //  @selftest
    dafka_publisher_t *self = dafka_publisher_new ("hello");
    assert (self);

    // Send MSG
    zframe_t *content = zframe_new ("HELLO", 5);
    int rc = dafka_publisher_publish (self, content);
    assert (rc == 0);

    // Send RELIABLE
    content = zframe_new ("HELLO", 5);
    rc = dafka_publisher_publish_reliable (self, content);
    assert (rc == 0);

    dafka_publisher_destroy (&self);
    //  @end
    printf ("OK\n");
}