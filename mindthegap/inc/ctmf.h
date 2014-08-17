/* Copyright (C) 2014 Rembrandt Bakker, Dhruv Kohli, Piotr Majka
   This file is part of mindthegap. It is free software and it is covered
   by the GNU General Public License. See the file LICENSE for details. */

//ctmf.h
#ifndef __CTMF_H__
#define __CTMF_H__ 1

//constant time median filter
//please refer ctmf.cpp for more detail
void ctmf(
        const unsigned char* const, unsigned char* const,
        const int, const int,
        const int, const int,
        const int, const int, const long unsigned int
        );

#endif