// libTorrent - BitTorrent library
// Copyright (C) 2005, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

#ifndef LIBTORRENT_NET_SOCKET_SET_H
#define LIBTORRENT_NET_SOCKET_SET_H

#include <vector>
#include <list>

#include "torrent/exceptions.h"
#include "socket_base.h"

namespace torrent {

// SocketSet's Base is a vector of active SocketBase
// instances. 'm_table' is a vector with the size 'openMax', each
// element of which points to an active instance in the Base vector.

class SocketSet : private std::vector<SocketBase*> {
public:
  typedef uint32_t                 size_type;
  typedef std::vector<SocketBase*> Base;
  typedef std::vector<size_type>   Table;

  static const size_type npos = static_cast<size_type>(-1);

  using Base::value_type;

  using Base::iterator;
  using Base::reverse_iterator;
  using Base::empty;
  using Base::size;

  using Base::begin;
  using Base::end;
  using Base::rbegin;
  using Base::rend;

  bool                has(SocketBase* s) const               { return _index(s) != npos; }

  iterator            find(SocketBase* s);
  void                insert(SocketBase* s);
  void                erase(SocketBase* s);

  // Remove all erased elements from the container.
  void                prepare();
  // Allocate storage for fd's with up to 'openMax' value. TODO: Remove reserve
  void                reserve(size_t openMax)                { m_table.resize(openMax, npos); Base::reserve(openMax); }

private:
  size_type&          _index(SocketBase* s)                  { return m_table[s->get_fd().get_fd()]; }
  const size_type&    _index(SocketBase* s) const            { return m_table[s->get_fd().get_fd()]; }

  inline void         _replace_with_last(size_type idx);

  // TODO: Table of indexes or iterators?
  Table               m_table;
  Table               m_erased;
};

inline SocketSet::iterator
SocketSet::find(SocketBase* s) {
  if (_index(s) == npos)
    return end();

  return begin() + _index(s);
}

inline void
SocketSet::insert(SocketBase* s) {
  if (static_cast<size_type>(s->get_fd().get_fd()) >= m_table.size())
    throw internal_error("Tried to insert an out-of-bounds file descriptor to SocketSet");

  if (_index(s) != npos)
    return;
  
  _index(s) = size();
  Base::push_back(s);
}

inline void
SocketSet::erase(SocketBase* s) {
  if (static_cast<size_type>(s->get_fd().get_fd()) >= m_table.size())
    throw internal_error("Tried to erase an out-of-bounds file descriptor from SocketSet");

  size_type idx = _index(s);

  if (idx == npos)
    return;

  _index(s) = npos;

  *(begin() + idx) = NULL;
  m_erased.push_back(idx);
}

}

#endif
