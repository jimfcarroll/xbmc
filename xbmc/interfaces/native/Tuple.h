/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

/**
 * This file contains a few templates to define various length
 * Tuples. 
 */
namespace XBMCAddon
{
  template<class T1, class T2> class Tuple2
  {
  protected:
    T1 v1;
    T2 v2;
    int numValuesSet;

  public:
    inline Tuple2() : numValuesSet(0) {}
    inline Tuple2(T1 pv1) : v1(pv1), numValuesSet(1) {}
    inline Tuple2(T1 pv1, T2 pv2) : v1(pv1), v2(pv2), numValuesSet(2) {}
    inline Tuple2(const Tuple2& other) : v1(other.v1), v2(other.v2), numValuesSet(other.numValuesSet) {}

    inline T1& first() { return v1; }
    inline const T1& first() const { return v1; }

    inline T2& second() { return v2; }
    inline const T2& second() const { return v2; }

    inline int GetNumValuesSet() const { return numValuesSet; }
    inline void SetNumValuesSet(int pcount) { numValuesSet = pcount; }
  };

  template<class T1, class T2, class T3> class Tuple3 : public Tuple2<T1, T2>
  {
  private:
    T3 v3;
  public:
    inline Tuple3() {}
    inline Tuple3(T1 pv1) : Tuple2<T1,T2>(pv1) { }
    inline Tuple3(T1 pv1, T2 pv2) : Tuple2<T1,T2>(pv1,pv2) { }
    inline Tuple3(T1 pv1, T2 pv2, T3 pv3) : Tuple2<T1,T2>(pv1,pv2), v3(pv3) { this->SetNumValuesSet(3); }
    inline Tuple3(const Tuple3& other) : Tuple2<T1,T2>(other), v3(other.v3) { }

    inline T3& third() { return v3; }
    inline const T3& third() const { return v3; }
  };
}
