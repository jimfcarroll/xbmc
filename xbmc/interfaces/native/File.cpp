/*
 *      Copyright (C) 2005-2011 Team XBMC
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

#include "File.h"

namespace XBMCAddon
{

  namespace xbmcvfs
  {
//    void* File::read(unsigned long numBytes)
//    {
//      DelayedCallGuard dg;
//      int64_t size = file->GetLength();
//      int64_t readBytes = numBytes;
//      if (!numBytes || readBytes > size)
//        readBytes = size;
//      char* buffer = new char[readBytes + 1];
//      if (buffer)
//      {
//        int64_t bytesRead = file->Read(buffer, readBytes);
//        buffer[std::min(bytesRead, readBytes)] = 0;
//      }
//      return buffer;
//    }

    bool File::write(const char* pBuffer)
    {
      DelayedCallGuard dg;
      return file->Write( (void*) pBuffer, strlen( pBuffer ) + 1 ) > 0;
    }

  }
}
