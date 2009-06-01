/*
 * Copyright (c) 2004,2005, University of Cincinnati, Ohio.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the OBR Center for 
 *      Distributed and Mobile Computing lab at the University of Cincinnati.
 * 4. Neither the name of the University nor of the lab may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "baseband.h"
#include "bt-channel.h"

BTChannel::BTChannel()
{
    numNode = 0;
}

void BTChannel::recv(Packet *p)
{
    hdr_bt *bh = HDR_BT(p);
    int nokeep = bh->nokeep;
    if (nokeep) {
	bh->nokeep = 0;
    }

    // printf("num: %d s: %d ", numNode, HDR_BT(p)->sender);
    for (int i = 0; i < numNode; i++) {
	// printf(" %d", bb_[i]->bd_addr_);
	if (bb_[i]->bd_addr_ != bh->sender && 
		bb_[i]->_trx_st == Baseband::RX_ON) {
	    bb_[i]->recv_handle(p->copy());
	}
    }
    if (nokeep) {
	Packet::free(p);
    }
    // printf("\n");
}

void BTChannel::add(Baseband *bb)
{
    bb_[numNode++] = bb;
}
