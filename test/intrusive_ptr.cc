// Copyright (c) 2011, Robert Escriva
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of this project nor the names of its contributors may
//       be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma GCC diagnostic ignored "-Wfloat-equal"

// e
#include "th.h"
#include "e/intrusive_ptr.h"

namespace
{

class foo
{
    public:
        foo()
            : m_ref(0)
        {
        }

    private:
        friend class e::intrusive_ptr<foo>;

    private:
#ifdef _MSC_VER
		void inc() { System::Threading::Interlocked::Increment(m_ref); }
		void dec() { if(System::Threading::Interlocked::Decrement(m_ref) == 0) delete this; }
#else
        void inc() { __sync_add_and_fetch(&m_ref, 1); }
        void dec() { if (__sync_sub_and_fetch(&m_ref, 1) == 0) delete this; }
#endif

    private:
        size_t m_ref;
};

TEST(IntrusivePtr, CtorAndDtor)
{
    e::intrusive_ptr<foo> a;
    e::intrusive_ptr<foo> b(new foo());
    e::intrusive_ptr<foo> c(b);
}

class ctordtor
{
    public:
        ctordtor(bool& ctor, bool& dtor)
            : m_ref(0)
            , m_ctor(ctor)
            , m_dtor(dtor)
        {
            m_ctor = true;
        }

        ~ctordtor() throw ()
        {
            m_dtor = true;
        }

    public:
#ifdef _MSC_VER
		void inc() { System::Threading::Interlocked::Increment(m_ref); }
		void dec() { if(System::Threading::Interlocked::Decrement(m_ref) == 0) delete this; }
#else
        void inc() { __sync_add_and_fetch(&m_ref, 1); }
        void dec() { if (__sync_sub_and_fetch(&m_ref, 1) == 0) delete this; }
#endif

    public:
        size_t m_ref;
        bool& m_ctor;
        bool& m_dtor;
};

TEST(IntrusivePtr, Nesting)
{
    bool ctor = false;
    bool dtor = false;
    ASSERT_FALSE(ctor);
    ASSERT_FALSE(dtor);

    {
        e::intrusive_ptr<ctordtor> a(new ctordtor(ctor, dtor));
        ASSERT_TRUE(ctor);
        ASSERT_FALSE(dtor);

        {
            e::intrusive_ptr<ctordtor> b(a);
            ASSERT_TRUE(ctor);
            ASSERT_FALSE(dtor);

            {
                e::intrusive_ptr<ctordtor> c(b);
                ASSERT_TRUE(ctor);
                ASSERT_FALSE(dtor);

                {
                    e::intrusive_ptr<ctordtor> d(c);
                    ASSERT_TRUE(ctor);
                    ASSERT_FALSE(dtor);
                }

                ASSERT_TRUE(ctor);
                ASSERT_FALSE(dtor);
            }

            ASSERT_TRUE(ctor);
            ASSERT_FALSE(dtor);
        }

        ASSERT_TRUE(ctor);
        ASSERT_FALSE(dtor);
    }

    ASSERT_TRUE(ctor);
    ASSERT_TRUE(dtor);
}

class accessing
{
    public:
        accessing(int _a, double _b, char _c) : m_ref(0), a(_a), b(_b), c(_c) {}

    public:
#ifdef _MSC_VER
		void inc() { System::Threading::Interlocked::Increment(m_ref); }
		void dec() { if(System::Threading::Interlocked::Decrement(m_ref) == 0) delete this; }
#else
        void inc() { __sync_add_and_fetch(&m_ref, 1); }
        void dec() { if (__sync_sub_and_fetch(&m_ref, 1) == 0) delete this; }
#endif

    public:
        size_t m_ref;
        int a;
        double b;
        char c;
};

TEST(IntrusivePtr, Accessing)
{
    e::intrusive_ptr<accessing> ptr(new accessing(42, 3.1415, 'A'));
    ASSERT_EQ(42, (*ptr).a);
    ASSERT_EQ(42, ptr->a);
    ASSERT_EQ(3.1415, (*ptr).b);
    ASSERT_EQ(3.1415, ptr->b);
    ASSERT_EQ('A', (*ptr).c);
    ASSERT_EQ('A', ptr->c);
}

class assignment
{
    public:
        assignment() : m_ref(0) {}

    public:
#ifdef _MSC_VER
		void inc() { System::Threading::Interlocked::Increment(m_ref); }
		void dec() { if(System::Threading::Interlocked::Decrement(m_ref) == 0) delete this; }
#else
        void inc() { __sync_add_and_fetch(&m_ref, 1); }
        void dec() { if (__sync_sub_and_fetch(&m_ref, 1) == 0) delete this; }
#endif

    public:
        size_t m_ref;
};

TEST(IntrusivePtr, Assignment)
{
    e::intrusive_ptr<assignment> p(new assignment());
    e::intrusive_ptr<assignment> q;
    e::intrusive_ptr<assignment> r;
    ASSERT_EQ(1U, p->m_ref);
    q = p;
    ASSERT_EQ(2U, p->m_ref);
    r = p;
    ASSERT_EQ(3U, p->m_ref);
    r = q;
    ASSERT_EQ(3U, p->m_ref);
}

TEST(IntrusivePtr, Booleans)
{
    e::intrusive_ptr<assignment> p(new assignment());
    e::intrusive_ptr<assignment> q;

    if (!p)
    {
        FAIL();
    }

    if (q)
    {
        FAIL();
    }
}

TEST(IntrusivePtr, Compare)
{
    e::intrusive_ptr<assignment> p(new assignment());
    e::intrusive_ptr<assignment> q;

    ASSERT_GT(p, q);
    ASSERT_GE(p, q);
    ASSERT_NE(p, q);
    ASSERT_LE(q, p);
    ASSERT_LT(q, p);

    p = q;
    ASSERT_EQ(p, q);
}

struct node
{
    node() : ref(0), next() {}
#ifdef _MSC_VER
	void inc() { System::Threading::Interlocked::Increment(ref); }
	void dec() { if(System::Threading::Interlocked::Decrement(ref) == 0) delete this; }
#else
    void inc() { __sync_add_and_fetch(&ref, 1); }
    void dec() { if (__sync_sub_and_fetch(&ref, 1) == 0) delete this; }
#endif
    size_t ref;
    e::intrusive_ptr<node> next;
};

TEST(IntrusivePtr, HandOverHand)
{
    e::intrusive_ptr<node> head(new node());

    for (size_t i = 0; i < 10000000; ++i)
    {
        e::intrusive_ptr<node> tmp(new node());
        tmp->next = head;
        head = tmp;
    }

    while (head)
    {
        head = head->next;
    }
}

} // namespace
