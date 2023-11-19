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

#include "aes256.h"

#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QHostAddress>
#include <QPointer>
#include <QSocketNotifier>
#include <QSslSocket>
#include <QTimer>
#include <QtDebug>
#include <QtMath>

static qsizetype MIN_POOL_SIZE = 64;
static qsizetype POOLS = 32;

class fortunate_q: public QObject
{
  Q_OBJECT

 public:
  fortunate_q(QObject *parent):QObject(parent)
  {
    m_R = initialize_prng();
    m_source_indices.resize(POOLS);
    m_tcp_socket_connection_timer.setInterval(500);
  }

  ~fortunate_q()
  {
    m_periodic_write_timer.stop();
    m_tcp_socket_connection_timer.stop();
  }

  QByteArray random_data(const int n)
  {
    return random_data(n, m_R);
  }

  void set_file_peer(const QString &file_name)
  {
    m_file.close();
    m_file.setFileName(file_name);
    m_file.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    m_file_notifier = m_file_notifier ?
      (m_file_notifier->deleteLater(),
       new QSocketNotifier(m_file.handle(), QSocketNotifier::Read, this)) :
      (new QSocketNotifier(m_file.handle(), QSocketNotifier::Read, this));
    connect(m_file_notifier,
	    SIGNAL(activated(QSocketDescriptor, QSocketNotifier::Type)),
	    this,
	    SLOT(slot_file_ready_read(void)));
  }

  void set_send_byte(const char byte, const int interval)
  {
    /*
    ** Some devices require periodic data.
    */

    connect(&m_periodic_write_timer,
	    &QTimer::timeout,
	    this,
	    &fortunate_q::slot_send_byte,
	    Qt::UniqueConnection);
    m_periodic_write_timer.start(interval);
    m_send_byte[0] = byte;
  }

  void set_tcp_peer(const QString &address, const bool tls, const quint16 port)
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
	    &fortunate_q::slot_tcp_socket_ready_read,
	    Qt::UniqueConnection);
    connect(&m_tcp_socket,
	    SIGNAL(sslErrors(const QList<QSslError> &)),
	    this,
	    SLOT(slot_tcp_socket_ssl_erros(const QList<QSslError> &)),
	    Qt::UniqueConnection);
    connect(&m_tcp_socket_connection_timer,
	    &QTimer::timeout,
	    this,
	    &fortunate_q::slot_tcp_socket_disconnected,
	    Qt::UniqueConnection);
    m_tcp_address = QHostAddress(address);
    m_tcp_port = port;
    m_tcp_socket.abort();
    m_tcp_socket_tls = tls;
    m_tcp_socket_connection_timer.start();
  }

 private:
  enum class Devices
  {
    FILE = 0,
    TCP = 1
  };

  struct generator_state
  {
    QByteArray m_key;
    qint16 m_counter;
  };

  struct prng_state
  {
    QElapsedTimer m_lastReseed;
    QVector<QByteArray> m_P;
    generator_state m_G;
    int m_reseedCnt;
  };

  QFile m_file;
  QHostAddress m_tcp_address;
  QPointer<QSocketNotifier> m_file_notifier;
  QSslSocket m_tcp_socket;
  QTimer m_periodic_write_timer;
  QTimer m_tcp_socket_connection_timer;
  QVector<int> m_source_indices;
  bool m_tcp_socket_tls;
  char m_send_byte[1];
  prng_state m_R; // The magic pseudo-random number generator.
  quint16 m_tcp_port;

  static QByteArray E(const QByteArray &C, const QByteArray &K)
  {
    aes256 aes(K.constData());
    auto string(std::string(C.toHex().constData()));

    return QByteArray::fromHex
      (aes256::to_hex(aes.encrypt_block(aes256::from_hex(string))).data());
  }

  static QByteArray generate_blocks(const int k, generator_state &G)
  {
    QByteArray r;

    if(G.m_counter != 0)
      for(int i = 1; i <= k; i++)
	{
	  G.m_counter += 1;
	  r = r + E
	    (QByteArray::number(G.m_counter).rightJustified(16, '0', true),
	     G.m_key);
	}

    return r;
  }

  static QByteArray pseudo_random_data(const int n, generator_state &G)
  {
    QByteArray r;

    if(0 <= n && 1048576 >= n)
      {
	r = generate_blocks(qCeil(n / 16.0), G).mid(0, n);
	G.m_key = generate_blocks(2, G);
      }

    return r;
  }

  static QByteArray random_data(const int n, prng_state &R)
  {
    if(MIN_POOL_SIZE <= R.m_P[0].size() ||
       R.m_lastReseed.elapsed() > 100 ||
       R.m_lastReseed.isValid() == false)
      {
	QByteArray s;

	R.m_reseedCnt += 1;

	for(int i = 0; i < static_cast<int> (R.m_P.size()); i++)
	  if(R.m_reseedCnt % static_cast<int> (qPow(2.0, i)) == 0)
	    {
	      s = s + QCryptographicHash::hash
		(R.m_P[i], QCryptographicHash::Sha256);
	      R.m_P[i].clear();
	    }

	reseed(s, R.m_G);
	R.m_lastReseed.start();
      }

    if(R.m_reseedCnt == 0)
      return QByteArray(); // Error!
    else
      return pseudo_random_data(n, R.m_G);
  }

  static generator_state initialize_generator(void)
  {
    return generator_state{QByteArray(), 0};
  }

  static prng_state initialize_prng(void)
  {
    prng_state R;

    R.m_G = initialize_generator();
    R.m_P.resize(POOLS);
    R.m_reseedCnt = 0;
    return R;
  }

  static void reseed(const QByteArray &s, generator_state &G)
  {
    G.m_counter += 1;
    G.m_key = QCryptographicHash::hash
      (G.m_key + s, QCryptographicHash::Sha256);
  }

  void process_device(QIODevice *device, const int i, const int s)
  {
    if(device && device->isOpen())
      do
	{
	  auto e(device->read(32));

	  if(!e.isEmpty())
	    m_R.m_P[i] = m_R.m_P[i] +
	      QByteArray::number(s) +
	      QByteArray::number(e.size()) +
	      e;
	}
      while(device->bytesAvailable() > 0);
  }

 private slots:
  void slot_file_ready_read(void)
  {
    auto s = static_cast<int> (Devices::FILE);

    m_source_indices[s] = (m_source_indices[s] + 1) % POOLS;
    process_device(&m_file, m_source_indices[s], s);
  }

  void slot_send_byte(void)
  {
    if(m_tcp_socket.state() == QAbstractSocket::ConnectedState)
      m_tcp_socket.write(m_send_byte, static_cast<qsizetype> (1));
  }

  void slot_tcp_socket_connected(void)
  {
    m_tcp_socket_connection_timer.stop();
  }

  void slot_tcp_socket_disconnected(void)
  {
    if(m_tcp_socket.state() == QAbstractSocket::UnconnectedState)
      {
	if(m_tcp_socket_tls)
	  m_tcp_socket.connectToHostEncrypted
	    (m_tcp_address.toString(), m_tcp_port);
	else
	  m_tcp_socket.connectToHost(m_tcp_address, m_tcp_port);

	m_tcp_socket.ignoreSslErrors();
	m_tcp_socket_connection_timer.start();
      }
  }

  void slot_tcp_socket_ready_read(void)
  {
    auto s = static_cast<int> (Devices::TCP);

    m_source_indices[s] = (m_source_indices[s] + 1) % POOLS;
    process_device(&m_tcp_socket, m_source_indices[s], s);
  }

  void slot_tcp_socket_ssl_erros(const QList<QSslError> &errors)
  {
    Q_UNUSED(errors);
    m_tcp_socket.ignoreSslErrors();
  }
};

#endif
