/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include <intern/m4_isomedia_dev.h>

// Rewrite the good dependancies when an OD AU is extracted from the file
M4Err Media_RewriteODFrame(MediaAtom *mdia, M4Sample *sample)
{
	M4Err e;
	LPODCODEC ODdecode;
	LPODCODEC ODencode;
	ODCommand *com;
	
	//the commands we proceed
	ESDescriptorUpdate *esdU, *esdU2;
	ESDescriptorRemove *esdR, *esdR2;
	ObjectDescriptorUpdate *odU, *odU2;

	//the desc they contain
	ObjectDescriptor *od;
	M4F_ObjectDescriptor *m4_od;
	ESDescriptor *esd;
	ES_ID_Ref *ref;
	Descriptor *desc;
	TrackReferenceTypeAtom *mpod;
	u32 i, j, skipped;

	if (!mdia || !sample || !sample->data || !sample->dataLength) return M4BadParam;

	mpod = NULL;
	e = Track_FindRef(mdia->mediaTrack, ODTrackReferenceAtomType, &mpod);
	if (e) return e;
	//no references, nothing to do...
	if (!mpod) return M4OK;

	ODdecode = OD_NewCodec(OD_READ);
	if (!ODdecode) return M4OutOfMem;
	ODencode = OD_NewCodec(OD_WRITE);
	if (!ODencode) {
		OD_DeleteCodec(ODdecode);
		return M4OutOfMem;
	}
	e = OD_SetBuffer(ODdecode, sample->data, sample->dataLength);
	if (e) goto err_exit;
	e = OD_DecodeAU(ODdecode);
	if (e) goto err_exit;

	while (1) {
		com = OD_GetCommand(ODdecode);
		if (!com) break;

		//we only need to rewrite commands with ESDs inside: ESDUpdate and ODUpdate
		switch (com->tag) {
		case ODUpdate_Tag:
			odU = (ObjectDescriptorUpdate *) com;
			odU2 = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);

			for (i = 0; i< ChainGetCount(odU->objectDescriptors); i++) {
				desc = (Descriptor*)ChainGetEntry(odU->objectDescriptors, i);
				switch (desc->tag) {
				case ObjectDescriptor_Tag:
				case MP4_OD_Tag:
				//IOD can be used in OD streams
				case MP4_IOD_Tag:
					break;
				default:
					return M4InvalidMP4File;
				}
				e = OD_DuplicateDescriptor(desc, (Descriptor **)&m4_od);
				if (e) goto err_exit;
					
				//create our OD...
				if (desc->tag == MP4_IOD_Tag) {
					od = (ObjectDescriptor *) malloc(sizeof(InitialObjectDescriptor));
				} else {
					od = (ObjectDescriptor *) malloc(sizeof(ObjectDescriptor));
				}
				if (!od) {
					e = M4OutOfMem;
					goto err_exit;
				}
				od->ESDescriptors = NewChain();
				//and duplicate...
				od->objectDescriptorID = m4_od->objectDescriptorID;
				od->tag = ObjectDescriptor_Tag;
				od->URLString = m4_od->URLString;
				m4_od->URLString = NULL;
				od->extensionDescriptors = m4_od->extensionDescriptors;
				m4_od->extensionDescriptors = NULL;
				od->IPMPDescriptorPointers = m4_od->IPMPDescriptorPointers;
				m4_od->IPMPDescriptorPointers = NULL;
				od->OCIDescriptors = m4_od->OCIDescriptors;
				m4_od->OCIDescriptors = NULL;
				
				//init as IOD
				if (m4_od->tag == MP4_IOD_Tag) {
					((InitialObjectDescriptor *)od)->audio_profileAndLevel = ((M4F_InitialObjectDescriptor *)m4_od)->audio_profileAndLevel;
					((InitialObjectDescriptor *)od)->inlineProfileFlag = ((M4F_InitialObjectDescriptor *)m4_od)->inlineProfileFlag;
					((InitialObjectDescriptor *)od)->graphics_profileAndLevel = ((M4F_InitialObjectDescriptor *)m4_od)->graphics_profileAndLevel;
					((InitialObjectDescriptor *)od)->OD_profileAndLevel = ((M4F_InitialObjectDescriptor *)m4_od)->OD_profileAndLevel;
					((InitialObjectDescriptor *)od)->scene_profileAndLevel = ((M4F_InitialObjectDescriptor *)m4_od)->scene_profileAndLevel;
					((InitialObjectDescriptor *)od)->visual_profileAndLevel = ((M4F_InitialObjectDescriptor *)m4_od)->visual_profileAndLevel;
				}
				
				//then rewrite the ESDesc
				for (j = 0; j < ChainGetCount(m4_od->ES_ID_RefDescriptors); j++) {
					ref = (ES_ID_Ref*)ChainGetEntry(m4_od->ES_ID_RefDescriptors, j);
					//if the ref index is not valid, skip this desc...
					if (!mpod->trackIDs || GetTrackFromID(mdia->mediaTrack->moov, mpod->trackIDs[ref->trackRef - 1]) == NULL) continue;
					//OK, get the esd
					e = GetESDForTime(mdia->mediaTrack->moov, mpod->trackIDs[ref->trackRef - 1], sample->DTS, &esd);
					if (!e) e = OD_AddDescToDesc((Descriptor *) od, (Descriptor *) esd);
					if (e) {
						OD_DeleteDescriptor((Descriptor **)&od);
						OD_DeleteCommand((ODCommand **)&odU2);
						OD_DeleteDescriptor((Descriptor **)&m4_od);
						OD_DeleteCommand((ODCommand **)&odU);
						goto err_exit;
					}

				}
				//delete our desc
				OD_DeleteDescriptor((Descriptor **)&m4_od);
				ChainAddEntry(odU2->objectDescriptors, od);
			}
			//clean a bit
			OD_DeleteCommand((ODCommand **)&odU);
			//if no desc in the command, remove it
			if (ChainGetCount(odU2->objectDescriptors)) {
				OD_AddCommand(ODencode, (ODCommand *)odU2);
			} else {
				OD_DeleteCommand((ODCommand **)&odU2);
			}
			break;

		case ESDUpdate_Tag:
			esdU = (ESDescriptorUpdate *) com;
			esdU2 = (ESDescriptorUpdate *) OD_NewCommand(ESDUpdate_Tag);
			esdU2->ODID = esdU->ODID;
			for (i = 0; i< ChainGetCount(esdU->ESDescriptors); i++) {
				ref = (ES_ID_Ref*)ChainGetEntry(esdU->ESDescriptors, i);
				//if the ref index is not valid, skip this desc...
				if (GetTrackFromID(mdia->mediaTrack->moov, mpod->trackIDs[ref->trackRef - 1]) == NULL) continue;
				//OK, get the esd
				e = GetESDForTime(mdia->mediaTrack->moov, mpod->trackIDs[ref->trackRef - 1], sample->DTS, &esd);
				if (e) goto err_exit;
				ChainAddEntry(esdU2->ESDescriptors, esd);
			}
			OD_DeleteCommand((ODCommand **)&esdU);
			//if our command is not empty, add it. Otherwise delete it...
			if (ChainGetCount(esdU2->ESDescriptors)) {
				OD_AddCommand(ODencode, (ODCommand *)esdU2);
			} else {
				OD_DeleteCommand((ODCommand **)&esdU2);
			}
			break;

		//brand new case: the ESRemove follows the same principle according to the spec...
		case ESDRemove_Ref_Tag:
			//both commands have the same structure, only the tags change
			esdR = (ESDescriptorRemove *) com;
			esdR2 = (ESDescriptorRemove *) OD_NewCommand(ESDRemove_Tag);
			esdR2->ODID = esdR->ODID;
			esdR2->NbESDs = esdR->NbESDs;
			//alloc our stuff
			esdR2->ES_ID = (unsigned short*)malloc(sizeof(u32) * esdR->NbESDs);
			if (!esdR2->ES_ID) {
				e = M4OutOfMem;
				goto err_exit;
			}
			skipped = 0;
			//get the ES_ID in the mpod indicated in the ES_ID[]
			for (i = 0; i < esdR->NbESDs; i++) {
				//if the ref index is not valid, remove this desc...
				if (GetTrackFromID(mdia->mediaTrack->moov, mpod->trackIDs[esdR->ES_ID[i] - 1]) == NULL) {
					skipped ++;
				} else {
					//the command in the file has the ref index of the trackID in the mpod
					esdR2->ES_ID[i - skipped] = mpod->trackIDs[esdR->ES_ID[i] - 1];
				}
			}
			//realloc...
			if (skipped && (skipped != esdR2->NbESDs) ) {
				esdR2->NbESDs -= skipped;
				esdR2->ES_ID = (unsigned short*)realloc(esdR2->ES_ID, sizeof(u32) * esdR2->NbESDs);
			}
			OD_DeleteCommand((ODCommand **)&esdR);
			//if empty, remove it otherwise add it
			if (skipped == esdR2->NbESDs) {
				OD_DeleteCommand((ODCommand **)&esdR2);
			} else {
				OD_AddCommand(ODencode, (ODCommand *)esdR2);
			}
			break;

		default:
			e = OD_AddCommand(ODencode, com);
			if (e) goto err_exit;
		}
	}
	//encode our new AU
	e = OD_EncodeAU(ODencode);
	if (e) goto err_exit;

	//and set the buffer in the sample
	free(sample->data);
	sample->data = NULL;
	sample->dataLength = 0;
	e = OD_GetEncodedAU(ODencode, &sample->data, &sample->dataLength);

err_exit:
	OD_DeleteCodec(ODdecode);
	OD_DeleteCodec(ODencode);
	return e;
}


// Update the dependancies when an OD AU is inserted in the file
M4Err Media_ParseODFrame(MediaAtom *mdia, M4Sample *sample)
{
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *mpod;
	M4Err e;
	ODCommand *com;
	LPODCODEC ODencode;
	LPODCODEC ODdecode;
	u32 i, j;

	//the commands we proceed
	ESDescriptorUpdate *esdU, *esdU2;
	ESDescriptorRemove *esdR, *esdR2;
	ObjectDescriptorUpdate *odU, *odU2;

	//the desc they contain
	ObjectDescriptor *od;
	M4F_ObjectDescriptor *m4_od;
	ESDescriptor *esd;
	ES_ID_Ref *ref;
	Descriptor *desc;
	M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex);
	M4Err trak_AddAtom(TrackAtom *ptr, Atom *a);
	M4Err tref_AddAtom(TrackReferenceAtom *ptr, Atom *a);

	if (!mdia || !sample || !sample->data || !sample->dataLength) return M4BadParam;

	//First find the references, and create them if none
	tref = mdia->mediaTrack->References;
	if (!tref) {
		tref = (TrackReferenceAtom *) CreateAtom(TrackReferenceAtomType);
		e = trak_AddAtom(mdia->mediaTrack, (Atom *) tref);
		if (e) return e;
	}
	//then find the OD reference, and create it if none
	e = Track_FindRef(mdia->mediaTrack, ODTrackReferenceAtomType, &mpod);
	if (e) return e;
	if (!mpod) {
		mpod = (TrackReferenceTypeAtom *) CreateAtom(ODTrackReferenceAtomType);
		e = tref_AddAtom(tref, (Atom *)mpod);
		if (e) return e;
	}

	//OK, create our codecs
	ODencode = OD_NewCodec(OD_WRITE);
	if (!ODencode) return M4OutOfMem;
	ODdecode = OD_NewCodec(OD_READ);
	if (!ODdecode) return M4OutOfMem;

	e = OD_SetBuffer(ODdecode, sample->data, sample->dataLength);
	if (e) goto err_exit;
	e = OD_DecodeAU(ODdecode);
	if (e) goto err_exit;

	while (1) {
		com = OD_GetCommand(ODdecode);
		if (!com) break;

		//check our commands
		switch (com->tag) {
		//Rewrite OD Update
		case ODUpdate_Tag:
			//duplicate our command
			odU = (ObjectDescriptorUpdate *) com;
			odU2 = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);

			for (i = 0; i < ChainGetCount(odU->objectDescriptors); i++) {
				desc = (Descriptor*)ChainGetEntry(odU->objectDescriptors, i);
				//both OD and IODs are accepted
				switch (desc->tag) {
				case ObjectDescriptor_Tag:
				case InitialObjectDescriptor_Tag:
					break;
				default:
					e = M4InvalidDescriptor;
					goto err_exit;
				}
				//get the esd
				e = OD_DuplicateDescriptor(desc, (Descriptor **)&od);
				if (e) goto err_exit;
				if (desc->tag == ObjectDescriptor_Tag) {
					m4_od = (M4F_ObjectDescriptor *) malloc(sizeof(M4F_ObjectDescriptor));
					m4_od->tag = MP4_OD_Tag;
				} else {
					m4_od = (M4F_ObjectDescriptor *) malloc(sizeof(M4F_InitialObjectDescriptor));
					m4_od->tag = MP4_IOD_Tag;
					//copy PL
					((M4F_InitialObjectDescriptor *)m4_od)->inlineProfileFlag = ((InitialObjectDescriptor *)od)->inlineProfileFlag;
					((M4F_InitialObjectDescriptor *)m4_od)->graphics_profileAndLevel = ((InitialObjectDescriptor *)od)->graphics_profileAndLevel;
					((M4F_InitialObjectDescriptor *)m4_od)->audio_profileAndLevel = ((InitialObjectDescriptor *)od)->audio_profileAndLevel;
					((M4F_InitialObjectDescriptor *)m4_od)->OD_profileAndLevel = ((InitialObjectDescriptor *)od)->OD_profileAndLevel;
					((M4F_InitialObjectDescriptor *)m4_od)->scene_profileAndLevel = ((InitialObjectDescriptor *)od)->scene_profileAndLevel;
					((M4F_InitialObjectDescriptor *)m4_od)->visual_profileAndLevel = ((InitialObjectDescriptor *)od)->visual_profileAndLevel;
				}
				//in OD stream only ref desc are accepted
				m4_od->ES_ID_RefDescriptors = NewChain();
				m4_od->ES_ID_IncDescriptors = NULL;
	
				//TO DO: check that a given sampleDescription exists
				m4_od->extensionDescriptors = od->extensionDescriptors;
				od->extensionDescriptors = NULL;
				m4_od->IPMPDescriptorPointers = od->IPMPDescriptorPointers;
				od->IPMPDescriptorPointers = NULL;
				m4_od->OCIDescriptors = od->OCIDescriptors;
				od->OCIDescriptors = NULL;
				m4_od->URLString = od->URLString;
				od->URLString = NULL;
				m4_od->objectDescriptorID = od->objectDescriptorID;

				for (j = 0; j < ChainGetCount(od->ESDescriptors); j++) {
					esd =(ESDescriptor*)ChainGetEntry(od->ESDescriptors, j);
					ref = (ES_ID_Ref *) OD_NewDescriptor(ES_ID_RefTag);
					//1 to 1 mapping trackID and ESID. Add this track to MPOD
					//if track does not exist, this will be remove while reading the OD stream
					e = reftype_AddRefTrack(mpod, esd->ESID, &ref->trackRef);
					e = OD_AddDescToDesc((Descriptor *)m4_od, (Descriptor *)ref);
					if (e) goto err_exit;
				}
				//delete our desc
				OD_DeleteDescriptor((Descriptor **)&od);
				//and add the new one to our command
				ChainAddEntry(odU2->objectDescriptors, m4_od);
			}
			//delete the command
			OD_DeleteCommand((ODCommand **)&odU);
			//and add the new one to the codec
			OD_AddCommand(ODencode, (ODCommand *)odU2);
			break;

		//Rewrite ESD Update
		case ESDUpdate_Tag:
			esdU = (ESDescriptorUpdate *) com;
			esdU2 = (ESDescriptorUpdate *) OD_NewCommand(ESDUpdate_Tag);
			esdU2->ODID = esdU->ODID;
			for (i = 0; i< ChainGetCount(esdU->ESDescriptors); i++) {
				esd = (ESDescriptor*)ChainGetEntry(esdU->ESDescriptors, i);
				ref = (ES_ID_Ref *) OD_NewDescriptor(ES_ID_RefTag);
				//1 to 1 mapping trackID and ESID
				e = reftype_AddRefTrack(mpod, esd->ESID, &ref->trackRef);
				e = ChainAddEntry(esdU2->ESDescriptors, ref);
				if (e) goto err_exit;
			}
			OD_DeleteCommand((ODCommand **)&esdU);
			OD_AddCommand(ODencode, (ODCommand *)esdU2);
			break;

		//Brand new case: the ESRemove has to be rewritten too according to the specs...
		case ESDRemove_Tag:
			esdR = (ESDescriptorRemove *) com;
			esdR2 = (ESDescriptorRemove *) OD_NewCommand(ESDRemove_Tag);
			//change the tag for the file format
			esdR2->tag = ESDRemove_Ref_Tag;
			esdR2->ODID = esdR->ODID;
			esdR2->NbESDs = esdR->NbESDs;
			//alloc our stuff
			esdR2->ES_ID = (unsigned short*)malloc(sizeof(u32) * esdR->NbESDs);
			if (!esdR2->ES_ID) {
				e = M4OutOfMem;
				goto err_exit;
			}
			for (i = 0; i < esdR->NbESDs; i++) {
				//1 to 1 mapping trackID and ESID
				e = reftype_AddRefTrack(mpod, esdR->ES_ID[i], &esdR2->ES_ID[i]);
				if (e) goto err_exit;
			}
			OD_DeleteCommand(&com);
			OD_AddCommand(ODencode, (ODCommand *)esdR2);
			break;

		//Add the command as is
		default:
			e = OD_AddCommand(ODencode, com);
			if (e) goto err_exit;
		}
	}

	//encode our new AU
	e = OD_EncodeAU(ODencode);
	if (e) goto err_exit;

	//and set the buffer in the sample
	free(sample->data);
	sample->data = NULL;
	sample->dataLength = 0;
	e = OD_GetEncodedAU(ODencode, &sample->data, &sample->dataLength);

err_exit:

	OD_DeleteCodec(ODencode);
	OD_DeleteCodec(ODdecode);
	return e;
}

