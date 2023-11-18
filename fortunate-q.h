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

#ifndef _fortunate_q_h_
#define _fortunate_q_h_

#include <QHostAddress>
#include <QPointer>
#include <QSslSocket>
#include <QTimer>
#include <QtDebug>

class fortunate_q: public QObject
{
  Q_OBJECT

 public:
  fortunate_q(void)
  {
    m_accumulator_maximum_length = 8 * 1024 * 1024; // 8 MiB is huge!
    m_tcp_socket_connection_timer.setInterval(500);
  }

  ~fortunate_q()
  {
  }

  void set_accumulator_size(const qsizetype size)
  {
    m_accumulator = m_accumulator.left
      (qMax(size, static_cast<qsizetype> (1024)));
    m_accumulator_maximum_length = qMax(size, static_cast<qsizetype> (1024));
  }

  void set_send_byte(const char byte, const int interval)
  {
    connect(&m_periodic_write_timer,
	    &QTimer::timeout,
	    this,
	    &fortunate_q::slot_send_byte,
	    Qt::UniqueConnection);
    m_periodic_write_timer.start(interval);
    m_send_byte[0] = byte;
  }

  void set_tcp_peer(const QString &address, const quint16 port)
  {
    connect(&m_tcp_socket,
	    &QSslSocket::connected,
	    this,
	    &fortunate_q::slot_tcp_socket_connected,
	    Qt::UniqueConnection);
    connect(&m_tcp_socket,
	    &QSslSocket::disconnected,
	    this,
	    &fortunate_q::slot_tcp_socket_disconnected,
	    Qt::UniqueConnection);
    connect(&m_tcp_socket,
	    &QSslSocket::readyRead,
	    this,
	    &fortunate_q::slot_ready_read,
	    Qt::UniqueConnection);
    connect(&m_tcp_socket_connection_timer,
	    &QTimer::timeout,
	    this,
	    &fortunate_q::slot_tcp_socket_disconnected,
	    Qt::UniqueConnection);
    m_device = &m_tcp_socket;
    m_tcp_address = QHostAddress(address);
    m_tcp_port = port;
    m_tcp_socket.abort();
    m_tcp_socket_connection_timer.start();
  }

 private:
  QByteArray m_accumulator;
  QHostAddress m_tcp_address;
  QPointer<QIODevice> m_device;
  QSslSocket m_tcp_socket;
  QTimer m_periodic_write_timer;
  QTimer m_tcp_socket_connection_timer;
  char m_send_byte[1];
  qsizetype m_accumulator_maximum_length;
  quint16 m_tcp_port;

 private slots:
   void slot_ready_read(void)
   {
     while(m_device && m_device->bytesAvailable() > 0)
       if(m_accumulator.size() < m_accumulator_maximum_length)
	 {
	   auto bytes(m_device->readAll());

	   m_accumulator.append
	     (bytes.
	      mid(0, -m_accumulator.size() + m_accumulator_maximum_length));
	 }
       else
	 Q_UNUSED(m_device->readAll());
   }

   void slot_send_byte(void)
   {
     if(!m_device || !m_device->isOpen())
       return;

     m_device->write(m_send_byte, static_cast<qsizetype> (1));
   }

   void slot_tcp_socket_connected(void)
   {
     m_tcp_socket_connection_timer.stop();
   }

   void slot_tcp_socket_disconnected(void)
   {
     if(m_tcp_socket.state() == QAbstractSocket::UnconnectedState)
       {
	 m_tcp_socket.connectToHost(m_tcp_address, m_tcp_port);
	 m_tcp_socket_connection_timer.start();
       }
   }
};

#endif
