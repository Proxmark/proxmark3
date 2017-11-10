//-----------------------------------------------------------------------------
// Copyright (C) 2017, Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// EMV constants and functions
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>


// Got from here. Thanks)
// https://eftlab.co.uk/index.php/site-map/knowledge-base/211-emv-aid-rid-pix
const char *PSElist [] = { 
	"325041592E5359532E4444463031", // 2PAY.SYS.DDF01 - Visa Proximity Payment System Environment - PPSE
	"315041592E5359532E4444463031"  // 1PAY.SYS.DDF01 - Visa Payment System Environment - PSE
}

const char *AIDlist [] = { 
	// Visa International
	"A00000000305076010",	// VISA ELO Credit	
	"A0000000031010",		// VISA Debit/Credit (Classic)	
	"A000000003101001",		// VISA Credit	
	"A000000003101002",		// VISA Debit	
	"A0000000032010",		// VISA Electron
	"A0000000032020",		// VISA	
	"A0000000033010",		// VISA Interlink	
	"A0000000034010",		// VISA Specific	
	"A0000000035010",		// VISA Specific	
	"A0000000036010",		// Domestic Visa Cash Stored Value	
	"A0000000036020",		// International Visa Cash Stored Value	
	"A0000000038002",		// VISA Auth, VisaRemAuthen EMV-CAP (DPA)	
	"A0000000038010",		// VISA Plus	
	"A0000000039010",		// VISA Loyalty	
	"A000000003999910",		// VISA Proprietary ATM	
	// Visa USA
	"A000000098",			// Debit Card
	"A0000000980848",		// Debit Card
	// Mastercard International
	"A00000000401",			// MasterCard PayPass	
	"A0000000041010",		// MasterCard Credit
	"A00000000410101213",	// MasterCard Credit
	"A00000000410101215",	// MasterCard Credit
	"A0000000042010",		// MasterCard Specific
	"A0000000043010",		// MasterCard Specific
	"A0000000043060",		// Maestro (Debit)
	"A000000004306001",		// Maestro (Debit)
	"A0000000044010",		// MasterCard Specific
	"A0000000045010",		// MasterCard Specific
	"A0000000046000",		// Cirrus
	"A0000000048002",		// SecureCode Auth EMV-CAP
	"A0000000049999",		// MasterCard PayPass	
	// American Express
	"A000000025",
	"A0000000250000",
	"A00000002501",
	"A000000025010402",
	"A000000025010701",
	"A000000025010801",
	// Groupement des Cartes Bancaires "CB"
	"A0000000421010",		// Cartes Bancaire EMV Card	
	"A0000000422010",		
	"A0000000423010",		
	"A0000000424010",		
	"A0000000425010",		
	// JCB CO., LTD.
	"A00000006510",			// JCB	
	"A0000000651010",		// JCB J Smart Credit	
	"A0000001544442",		// Banricompras Debito - Banrisul - Banco do Estado do Rio Grande do SUL - S.A.
	"F0000000030001",		// BRADESCO
	"A0000005241010",		// RuPay - RuPay
	"D5780000021010"		// Bankaxept - Bankaxept	
}
