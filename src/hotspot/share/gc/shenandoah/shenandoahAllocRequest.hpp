/*
 * Copyright (c) 2018, Red Hat, Inc. All rights reserved.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHALLOCREQUEST_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHALLOCREQUEST_HPP

#include "memory/allocation.hpp"

class ShenandoahAllocRequest : StackObj {
public:
  enum Type {
    _alloc_shared,      // Allocate common, outside of TLAB
    _alloc_shared_gc,   // Allocate common, outside of GCLAB
    _alloc_tlab,        // Allocate TLAB
    _alloc_gclab,       // Allocate GCLAB
    _alloc_temp_region,
    _ALLOC_LIMIT
  };

  static const char* alloc_type_to_string(Type type) {
    switch (type) {
      case _alloc_shared:
        return "Shared";
      case _alloc_shared_gc:
        return "Shared GC";
      case _alloc_tlab:
        return "TLAB";
      case _alloc_gclab:
        return "GCLAB";
      case _alloc_temp_region:
        return "Temp region for rdma";
      default:
        ShouldNotReachHere();
        return "";
    }
  }

private:
  size_t _min_size;
  size_t _requested_size;
  size_t _actual_size;
  Type _alloc_type;
  bool _is_remote;
#ifdef ASSERT
  bool _actual_size_set;
#endif

  ShenandoahAllocRequest(size_t _min_size, size_t _requested_size, Type _alloc_type, bool _is_remote) :
          _min_size(_min_size), _requested_size(_requested_size),
          _actual_size(0), _alloc_type(_alloc_type), _is_remote(_is_remote)
#ifdef ASSERT
          , _actual_size_set(false)
#endif
  {}

public:
  static inline ShenandoahAllocRequest for_tlab(size_t min_size, size_t requested_size) {
    return ShenandoahAllocRequest(min_size, requested_size, _alloc_tlab, false);
  }

  static inline ShenandoahAllocRequest for_gclab(size_t min_size, size_t requested_size, bool is_remote=false) {
    return ShenandoahAllocRequest(min_size, requested_size, _alloc_gclab, is_remote);
  }

  static inline ShenandoahAllocRequest for_shared_gc(size_t requested_size) {
    return ShenandoahAllocRequest(0, requested_size, _alloc_shared_gc, false);
  }

  static inline ShenandoahAllocRequest for_shared(size_t requested_size) {
    return ShenandoahAllocRequest(0, requested_size, _alloc_shared, false);
  }

  inline size_t size() {
    return _requested_size;
  }

  inline Type type() {
    return _alloc_type;
  }

  inline const char* type_string() {
    return alloc_type_to_string(_alloc_type);
  }

  inline size_t min_size() {
    assert (is_lab_alloc(), "Only access for LAB allocs");
    return _min_size;
  }

  inline size_t actual_size() {
    assert (_actual_size_set, "Should be set");
    return _actual_size;
  }

  inline void set_actual_size(size_t v) {
#ifdef ASSERT
    assert (!_actual_size_set, "Should not be set");
    _actual_size_set = true;
#endif
    _actual_size = v;
  }

  inline bool is_mutator_alloc() {
    switch (_alloc_type) {
      case _alloc_tlab:
      case _alloc_shared:
        return true;
      case _alloc_gclab:
      case _alloc_shared_gc:
        return false;
      default:
        ShouldNotReachHere();
        return false;
    }
  }

  inline bool is_gc_alloc() {
    switch (_alloc_type) {
      case _alloc_tlab:
      case _alloc_shared:
        return false;
      case _alloc_gclab:
      case _alloc_shared_gc:
        return true;
      default:
        ShouldNotReachHere();
        return false;
    }
  }

  inline bool is_lab_alloc() {
    switch (_alloc_type) {
      case _alloc_tlab:
      case _alloc_gclab:
        return true;
      case _alloc_shared:
      case _alloc_shared_gc:
        return false;
      default:
        ShouldNotReachHere();
        return false;
    }
  }

  inline bool is_remote() {
    return _is_remote;
  }
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHALLOCREQUEST_HPP
