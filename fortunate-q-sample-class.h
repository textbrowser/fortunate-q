/*
** Copyright (c) 2023, Alexis Megas.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from FortunateQ without specific prior written permission.
**
** FORTUNATEQ IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** FORTUNATEQ, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _fortunate_q_sample_class_h_
#define _fortunate_q_sample_class_h_

#include "fortunate-q.h"

class fortunate_q_sample_class: public QObject
{
  Q_OBJECT

 public:
  fortunate_q_sample_class(void):QObject()
  {
    m_f = new fortunate_q(this);
    m_f->set_file_peer("/dev/urandom");
    m_f->set_send_byte(0, 5);
    m_f->set_tcp_peer("192.168.178.85", false, 5000);
    connect(m_f,
	    SIGNAL(pool_filled(const int, const int)),
	    this,
	    SLOT(slot_pool_filled(const int, const int)));
  }

 private:
  fortunate_q *m_f;

 private slots:
  void slot_pool_filled(const int index, const int source)
  {
    Q_UNUSED(index);
    Q_UNUSED(source);
    qDebug() << m_f->random_data(15).toHex();
  }
};

#endif
