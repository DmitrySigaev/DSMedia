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

#include <gpac/intern/m4_isomedia_dev.h>

#ifdef GCCLONGINT
#define LLD "%lld"
#else
#define LLD "%I64d"
#endif


#define MP4_MAX_TREE	40

//to switch to "Box"
#define ATOM_NAME	"Box"

M4Err DumpAtom(Atom *a, FILE * trace)
{
	char name[5];

	if (a->size > 0xFFFFFFFF) {
		fprintf(trace, "<%sInfo LargeSize=\""LLD"\" ", ATOM_NAME, a->size);
	} else {
		fprintf(trace, "<%sInfo Size=\"%d\" ", ATOM_NAME, (u32) a->size);
	}
	if (a->type == ExtendedAtomType ) {
		fprintf(trace, "ExtendedType=\"%s\"/>\n", a->uuid);
	} else {
		MP4TypeToString(a->type, name);
		fprintf(trace, "Type=\"%s\"/>\n", name);
	}
	return M4OK;
}


void NullAtomErr(FILE * trace)
{
	fprintf(trace, "<!--ERROR: NULL %s Found-->\n", ATOM_NAME);
}

void BadTopAtomErr(Atom *a, FILE * trace)
{
	char name[5];
	MP4TypeToString(a->type, name);	
	fprintf(trace, "<!--ERROR: Invalid Top-level %s Found (\"%s\")-->\n", ATOM_NAME, name);
}

static void DumpData(FILE *trace, char *data, u32 dataLength)
{
	u32 i;
	fprintf(trace, "data:application/octet-string,");
	for (i=0; i<dataLength; i++) {
		fprintf(trace, "%%");
		fprintf(trace, "%02X", (unsigned char) data[i]);
	}
}

M4Err AtomDump(void *ptr, FILE * trace)
{
	Atom *a = (Atom *) ptr;

	if (!a) {
		NullAtomErr(trace);
		return M4OK;
	}

	switch (a->type) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		return reftype_dump(a, trace);
	case FreeSpaceAtomType:
	case SkipAtomType:
		return free_dump(a, trace);
	case MediaDataAtomType: return mdat_dump(a, trace);
	case MovieAtomType: return moov_dump(a, trace);
	case MovieHeaderAtomType: return mvhd_dump(a, trace);
	case MediaHeaderAtomType: return mdhd_dump(a, trace);
	case VideoMediaHeaderAtomType: return vmhd_dump(a, trace);
	case SoundMediaHeaderAtomType: return smhd_dump(a, trace);
	case HintMediaHeaderAtomType: return hmhd_dump(a, trace);
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		return nmhd_dump(a, trace);
	case SampleTableAtomType: return stbl_dump(a, trace);
	case DataInformationAtomType: return dinf_dump(a, trace);
	case DataEntryURLAtomType: return url_dump(a, trace);
	case DataEntryURNAtomType: return urn_dump(a, trace);
	case CopyrightAtomType: return cprt_dump(a, trace);
	case HandlerAtomType: return hdlr_dump(a, trace);
	case ObjectDescriptorAtomType: return iods_dump(a, trace);
	case TrackAtomType: return trak_dump(a, trace);
	case MPEGSampleEntryAtomType: return mp4s_dump(a, trace);
	case MPEGVisualSampleEntryAtomType: return mp4v_dump(a, trace);
	case MPEGAudioSampleEntryAtomType: return mp4a_dump(a, trace);
	case GenericMediaSampleEntryAtomType: return gnrm_dump(a, trace);
	case GenericVisualSampleEntryAtomType: return gnrv_dump(a, trace);
	case GenericAudioSampleEntryAtomType: return gnra_dump(a, trace);
	case EditAtomType: return edts_dump(a, trace);
	case UserDataAtomType: return udta_dump(a, trace);
	case DataReferenceAtomType: return dref_dump(a, trace);
	case SampleDescriptionAtomType: return stsd_dump(a, trace);
	case TimeToSampleAtomType: return stts_dump(a, trace);
	case CompositionOffsetAtomType: return ctts_dump(a, trace);
	case ShadowSyncAtomType: return stsh_dump(a, trace);
	case EditListAtomType: return elst_dump(a, trace);
	case SampleToChunkAtomType: return stsc_dump(a, trace);
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		return stsz_dump(a, trace);
	case ChunkOffsetAtomType: return stco_dump(a, trace);
	case SyncSampleAtomType: return stss_dump(a, trace);
	case DegradationPriorityAtomType: return stdp_dump(a, trace);
	case ChunkLargeOffsetAtomType: return co64_dump(a, trace);
	case ESDAtomType: return esds_dump(a, trace);
	case MediaInformationAtomType: return minf_dump(a, trace);
	case TrackHeaderAtomType: return tkhd_dump(a, trace);
	case TrackReferenceAtomType: return tref_dump(a, trace);
	case MediaAtomType: return mdia_dump(a, trace);

	//Hinting
	case RTPHintSampleEntryAtomType: return ghnt_dump(a, trace);
	case rtpoAtomType: return rtpo_dump(a, trace);
	case HintTrackInfoAtomType: return hnti_dump(a, trace);
	case SDPAtomType: return sdp_dump(a, trace);
	case HintInfoAtomType: return hinf_dump(a, trace);
	case relyHintEntryType: return rely_dump(a, trace);
	case timsHintEntryType: return tims_dump(a, trace);
	case tsroHintEntryType: return tsro_dump(a, trace);
	case snroHintEntryType: return snro_dump(a, trace);
	case trpyAtomType: return trpy_dump(a, trace);
	case numpAtomType: return nump_dump(a, trace);
	case totlAtomType: return totl_dump(a, trace);
	case npckAtomType: return npck_dump(a, trace);
	case tpylAtomType: return tpyl_dump(a, trace);
	case tpayAtomType: return tpay_dump(a, trace);
	case maxrAtomType: return maxr_dump(a, trace);
	case dmedAtomType: return dmed_dump(a, trace);
	case dimmAtomType: return dimm_dump(a, trace);
	case drepAtomType: return drep_dump(a, trace);
	case tminAtomType: return tmin_dump(a, trace);
	case tmaxAtomType: return tmax_dump(a, trace);
	case pmaxAtomType: return pmax_dump(a, trace);
	case dmaxAtomType: return dmax_dump(a, trace);
	case paytAtomType: return payt_dump(a, trace);
	case nameAtomType: return name_dump(a, trace);
	
	//V2 Atoms
	case FileTypeAtomType: return ftyp_dump(a, trace);
	case PaddingBitsAtomType: return padb_dump(a, trace);

#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: return mvex_dump(a, trace);
	case TrackExtendsAtomType: return trex_dump(a, trace);
	case MovieFragmentAtomType: return moof_dump(a, trace);
	case MovieFragmentHeaderAtomType: return mfhd_dump(a, trace);
	case TrackFragmentAtomType: return traf_dump(a, trace);
	case TrackFragmentHeaderAtomType: return tfhd_dump(a, trace);
	case TrackFragmentRunAtomType: return trun_dump(a, trace);
#endif
	
	case VoidAtomType: return void_dump(a, trace);
	case SampleFragmentAtomType: return stsf_dump(a, trace);

	//3GPP atoms
	case AMRSampleEntryAtomType: return amr3_dump(a, trace);
	case AMRConfigAtomType: return damr_dump(a, trace);
	case H263SampleEntryAtomType: return h263_dump(a, trace);
	case H263ConfigAtomType: return d263_dump(a, trace);

	case AVCConfigurationAtomType: return avcc_dump(a, trace);
	case MPEG4BitRateAtomType: return btrt_dump(a, trace);
	case MPEG4ExtensionDescriptorsAtomType: return m4ds_dump(a, trace);
	case AVCSampleEntryAtomType: return avc1_dump(a, trace);

	case FontTableAtomType: return ftab_dump(a, trace);
	case TextSampleEntryAtomType: return tx3g_dump(a, trace);
	case TextStyleAtomType: return styl_dump(a, trace);
	case TextHighlightAtomType: return hlit_dump(a, trace);
	case TextHighlightColorAtomType: return hclr_dump(a, trace);
	case TextKaraokeAtomType: return krok_dump(a, trace);
	case TextScrollDelayAtomType: return dlay_dump(a, trace);
	case TextHyperTextAtomType: return href_dump(a, trace);
	case TextBoxAtomType: return tbox_dump(a, trace);
	case TextBlinkAtomType: return blnk_dump(a, trace);
	case TextWrapAtomType: return twrp_dump(a, trace);

	default: return defa_dump(a, trace);
	}
}


M4Err AtomListDump(Chain *list, FILE * trace)
{
	u32 i;
	Atom *a;
	if (!list) return M4OK;
	for (i=0; i<ChainGetCount(list); i++) {
		a = ChainGetEntry(list, i);
		AtomDump(a, trace);
	}
	return M4OK;
}



M4Err M4_FileDump(M4File *file, FILE * trace)
{
	M4Err AtomDump(void *ptr, FILE * trace);
	void BadTopAtomErr(Atom *a, FILE * trace);

	u32 i;
	Atom *atom;
	M4Movie *mov = (M4Movie *)file;
	if (!mov || !trace) return M4BadParam;

	fprintf(trace, "<!--MP4Box dump trace-->\n");
	fprintf(trace, "<MP4File Name=\"%s\">\n", file->fileName);

	if (mov->brand) AtomDump(mov->brand, trace);

	for (i=0; i<ChainGetCount(mov->TopAtoms); i++) {
		atom = ChainGetEntry(mov->TopAtoms, i);
		switch (atom->type) {
		case FileTypeAtomType:
		case MovieAtomType:
		case MediaDataAtomType:
		case FreeSpaceAtomType:
		case SkipAtomType:
			break;
		default:
			BadTopAtomErr(atom, trace);
			break;
		}
		AtomDump(atom, trace);
	}
#ifndef	M4_ISO_NO_FRAGMENTS
	for (i=0; i<ChainGetCount(mov->MovieFragments); i++) {
		atom = ChainGetEntry(mov->MovieFragments, i);
		AtomDump(atom, trace);
	}
#endif

	fprintf(trace, "</MP4File>\n");
	return M4OK;
}



M4Err FullAtom_dump(Atom *a, FILE * trace)
{
	FullAtom *p;
	p = (FullAtom *)a;
	fprintf(trace, "<Full%sInfo Version=\"%d\" Flags=\"%d\"/>\n", ATOM_NAME, p->version, p->flags);
	return M4OK;
}


M4Err reftype_dump(Atom *a, FILE * trace)
{
	char *s;
	u32 i;
	TrackReferenceTypeAtom *p;

	p = (TrackReferenceTypeAtom *)a;

	switch (a->type) {
	case HintTrackReferenceAtomType:
		s = "Hint";
		break;
	case StreamDependenceAtomType:
		s = "Stream";
		break;
	case ODTrackReferenceAtomType:
		s = "OD";
		break;
	case OCRReferenceAtomType:
		s = "Sync";
		break;
	default:
		s = "Unknown";
		break;
	}
	fprintf(trace, "<%sTrackReference%s Tracks=\"", s, ATOM_NAME);
	for (i=0; i<p->trackIDCount; i++) fprintf(trace, " %d", p->trackIDs[i]);
	fprintf(trace, "\">\n");
	DumpAtom(a, trace);
	fprintf(trace, "</%sTrackReference%s>\n", s, ATOM_NAME);
	return M4OK;
}

M4Err free_dump(Atom *a, FILE * trace)
{
	FreeSpaceAtom *p;

	p = (FreeSpaceAtom *)a;
	fprintf(trace, "<FreeSpace%s size=\"%d\">\n", ATOM_NAME, p->dataSize);
	DumpAtom(a, trace);
	fprintf(trace, "</FreeSpace%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err mdat_dump(Atom *a, FILE * trace)
{
	MediaDataAtom *p;

	p = (MediaDataAtom *)a;
	fprintf(trace, "<MediaData%s dataSize=\""LLD"\">\n", ATOM_NAME, p->dataSize);
	DumpAtom(a, trace);
	fprintf(trace, "</MediaData%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err moov_dump(Atom *a, FILE * trace)
{
	MovieAtom *p;
	p = (MovieAtom *)a;
	fprintf(trace, "<Movie%s>\n", ATOM_NAME);
	DumpAtom(a, trace);

	if (p->iods) {
		AtomDump(p->iods, trace);
	} else {
		fprintf(trace, "<!--No Root Object Descriptor found-->\n");
	}
	AtomDump(p->mvhd, trace);
#ifndef	M4_ISO_NO_FRAGMENTS
	if (p->mvex) AtomDump(p->mvex, trace);
#endif

	AtomListDump(p->trackList, trace);
	if (p->udta) AtomDump(p->udta, trace);
	fprintf(trace, "</Movie%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err mvhd_dump(Atom *a, FILE * trace)
{
	MovieHeaderAtom *p;

	p = (MovieHeaderAtom *) a;

	fprintf(trace, "<MovieHeader%s ", ATOM_NAME);
	fprintf(trace, "CreationTime=\""LLD"\" ", p->creationTime);
	fprintf(trace, "ModificationTime=\""LLD"\" ", p->modificationTime);
	fprintf(trace, "TimeScale=\"%d\" ", p->timeScale);
	fprintf(trace, "Duration=\""LLD"\" ", p->duration);
	fprintf(trace, "NextTrackID=\"%d\">\n", p->nextTrackID);
	
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	fprintf(trace, "</MovieHeader%s>", ATOM_NAME);
	return M4OK;
}

M4Err mdhd_dump(Atom *a, FILE * trace)
{
	MediaHeaderAtom *p;
	
	p = (MediaHeaderAtom *)a;
	fprintf(trace, "<MediaHeader%s ", ATOM_NAME);

	fprintf(trace, "CreationTime=\""LLD"\" ", p->creationTime);
	fprintf(trace, "ModificationTime=\""LLD"\" ", p->modificationTime);
	fprintf(trace, "TimeScale=\"%d\" ", p->timeScale);
	fprintf(trace, "Duration=\""LLD"\" ", p->duration);
	fprintf(trace, "LanguageCode=\"%c%c%c\">\n", p->packedLanguage[0], p->packedLanguage[1], p->packedLanguage[2]);

	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</MediaHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err vmhd_dump(Atom *a, FILE * trace)
{
	fprintf(trace, "<VideoMediaHeader%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</VideoMediaHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err smhd_dump(Atom *a, FILE * trace)
{
	fprintf(trace, "<SoundMediaHeader%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</SoundMediaHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err hmhd_dump(Atom *a, FILE * trace)
{
	HintMediaHeaderAtom *p;

	p = (HintMediaHeaderAtom *)a;

	fprintf(trace, "<HintMediaHeader%s ", ATOM_NAME);

	fprintf(trace, "MaximumPDUSize=\"%d\" ", p->maxPDUSize);
	fprintf(trace, "AveragePDUSize=\"%d\" ", p->avgPDUSize);
	fprintf(trace, "MaxBitRate=\"%d\" ", p->maxBitrate);
	fprintf(trace, "AverageBitRate=\"%d\">\n", p->avgBitrate);
	
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</HintMediaHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err nmhd_dump(Atom *a, FILE * trace)
{
	fprintf(trace, "<MPEGMediaHeader%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</MPEGMediaHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stbl_dump(Atom *a, FILE * trace)
{
	SampleTableAtom *p;
	p = (SampleTableAtom *)a;
	fprintf(trace, "<SampleTable%s>\n", ATOM_NAME);
	DumpAtom(a, trace);

	AtomDump(p->SampleDescription, trace);
	AtomDump(p->TimeToSample, trace);
	if (p->CompositionOffset) AtomDump(p->CompositionOffset, trace);
	if (p->SyncSample) AtomDump(p->SyncSample, trace);
	if (p->ShadowSync) AtomDump(p->ShadowSync, trace);
	AtomDump(p->SampleToChunk, trace);
	AtomDump(p->SampleSize, trace);
	AtomDump(p->ChunkOffset, trace);
	if (p->DegradationPriority) AtomDump(p->DegradationPriority, trace);
	if (p->PaddingBits) AtomDump(p->PaddingBits, trace);
	if (p->Fragments) AtomDump(p->Fragments, trace);

	fprintf(trace, "</SampleTable%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err dinf_dump(Atom *a, FILE * trace)
{
	DataInformationAtom *p;
	p = (DataInformationAtom *)a;
	fprintf(trace, "<DataInformation%s>", ATOM_NAME);
	DumpAtom(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</DataInformation%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err url_dump(Atom *a, FILE * trace)
{
	DataEntryURLAtom *p;

	p = (DataEntryURLAtom *)a;
	fprintf(trace, "<URLDataEntry%s", ATOM_NAME);
	if (p->location) {
		fprintf(trace, " URL=\"%s\">\n", p->location);
	} else {
		fprintf(trace, ">\n");
		if (! (p->flags & 1) ) {
			fprintf(trace, "<!--ERROR: No location indicated-->\n");
		} else {
			fprintf(trace, "<!--Data is contained in the movie file-->\n");
		}
	}	
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</URLDataEntry%s>", ATOM_NAME);
	return M4OK;
}

M4Err urn_dump(Atom *a, FILE * trace)
{
	DataEntryURNAtom *p;

	p = (DataEntryURNAtom *)a;
	fprintf(trace, "<URNDataEntry%s", ATOM_NAME);
	if (p->nameURN) fprintf(trace, " URN=\"%s\"", p->nameURN);
	if (p->location) fprintf(trace, " URL=\"%s\"", p->location);
	fprintf(trace, ">\n");

	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</URNDataEntry%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err cprt_dump(Atom *a, FILE * trace)
{
	CopyrightAtom *p;

	p = (CopyrightAtom *)a;
	fprintf(trace, "<Copyright%s LanguageCode=\"%s\" CopyrightNotice=\"%s\">\n", ATOM_NAME, p->packedLanguageCode, p->notice);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</Copyright%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err hdlr_dump(Atom *a, FILE * trace)
{
	char name[5];
	u32 len;
	char *str;
	short uniLine[5000];
	HandlerAtom *p;

	p = (HandlerAtom *)a;
	MP4TypeToString(p->handlerType, name);

	fprintf(trace, "<Handler%s Type=\"%s\" Name=\"", ATOM_NAME, name);
	len = strlen(p->nameUTF8);
	str = p->nameUTF8;
	len = utf8_mbstowcs(uniLine, 5000, (const char **) &str);
	if (len != (size_t) -1) {
		fwprintf(trace, L"%s", uniLine);
	} else {
		fprintf(trace, "Unavailable (Badly formated)");
	}
	fprintf(trace, "\">\n");
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</Handler%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err iods_dump(Atom *a, FILE * trace)
{
	ObjectDescriptorAtom *p;

	p = (ObjectDescriptorAtom *)a;
	fprintf(trace, "<ObjectDescriptor%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (p->descriptor) {
		OD_DumpDescriptor(p->descriptor, trace, 1, 1);
	} else {
		fprintf(trace, "<!--WARNING: Object Descriptor not present-->\n");
	}
	fprintf(trace, "</ObjectDescriptor%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err trak_dump(Atom *a, FILE * trace)
{
	TrackAtom *p;

	p = (TrackAtom *)a;
	fprintf(trace, "<Track%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	if (p->Header) {
		AtomDump(p->Header, trace);
	} else {
		fprintf(trace, "<!--INVALID FILE: Missing Track Header-->\n");
	}
	if (p->References) AtomDump(p->References, trace);
	if (p->EditAtom) AtomDump(p->EditAtom, trace);
	AtomListDump(p->atomList, trace);
	if (p->udta) AtomDump(p->udta, trace);	
	fprintf(trace, "</Track%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err mp4s_dump(Atom *a, FILE * trace)
{
	MPEGSampleEntryAtom *p;

	p = (MPEGSampleEntryAtom *)a;
	fprintf(trace, "<MPEGSystemsSampleDescription%s DataReferenceIndex=\"%d\">\n", ATOM_NAME, p->dataReferenceIndex);
	DumpAtom(a, trace);
	if (p->esd) {
		AtomDump(p->esd, trace);
	} else {
		fprintf(trace, "<!--INVALID MP4 FILE: ESD_%s not present in MPEG Sample Description or corrupted-->\n", ATOM_NAME);
	}
	fprintf(trace, "</MPEGSystemsSampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}


void base_visual_entry_dump(VisualSampleEntryAtom *p, FILE * trace)
{
	fprintf(trace, " DataReferenceIndex=\"%d\" Width=\"%d\" Height=\"%d\"", p->dataReferenceIndex, p->Width, p->Height);

	//dump reserved info
	fprintf(trace, " XDPI=\"%d\" YDPI=\"%d\" BitDepth=\"%d\"", p->horiz_res, p->vert_res, p->bit_depth);
	if (strlen(p->compressor_name) )
		fprintf(trace, " CompressorName=\"%s\"\n", p->compressor_name);

}

M4Err mp4v_dump(Atom *a, FILE * trace)
{
	MPEGVisualSampleEntryAtom *p;

	p = (MPEGVisualSampleEntryAtom *)a;
	fprintf(trace, "<MPEGVisualSampleDescription%s", ATOM_NAME);
	base_visual_entry_dump((VisualSampleEntryAtom *)p, trace);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);

	
	if (p->esd) {
		AtomDump(p->esd, trace);
	} else {
		fprintf(trace, "<!--INVALID MP4 FILE: ESD_%s not present in MPEG Sample Description or corrupted-->\n", ATOM_NAME);
	}
	fprintf(trace, "</MPEGVisualSampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}


void base_audio_entry_dump(AudioSampleEntryAtom *p, FILE * trace)
{
	fprintf(trace, " DataReferenceIndex=\"%d\" SampleRate=\"%d\"", p->dataReferenceIndex, p->samplerate_hi);
	fprintf(trace, " Channels=\"%d\" BitsPerSample=\"%d\"", p->channel_count, p->bitspersample);
}

M4Err mp4a_dump(Atom *a, FILE * trace)
{
	MPEGAudioSampleEntryAtom *p;

	p = (MPEGAudioSampleEntryAtom *)a;
	fprintf(trace, "<MPEGAudioSampleDescription%s", ATOM_NAME);
	base_audio_entry_dump((AudioSampleEntryAtom *)p, trace);
	fprintf(trace, ">\n");

	DumpAtom(a, trace);
	if (p->esd) {
		AtomDump(p->esd, trace);
	} else {
		fprintf(trace, "<!--INVALID MP4 FILE: ESD_%s not present in MPEG Sample Description or corrupted-->\n", ATOM_NAME);
	}
	fprintf(trace, "</MPEGAudioSampleDescription%s>", ATOM_NAME);
	return M4OK;
}

M4Err gnrm_dump(Atom *a, FILE * trace)
{
	GenericMediaSampleEntryAtom *p = (GenericMediaSampleEntryAtom *)a;
	fprintf(trace, "<SampleDescription%s DataReferenceIndex=\"%d\" ExtensionDataSize=\"%d\">\n", ATOM_NAME, p->dataReferenceIndex, p->data_size);
	a->type = p->EntryType;
	DumpAtom(a, trace);
	a->type = GenericMediaSampleEntryAtomType;
	fprintf(trace, "</SampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err gnrv_dump(Atom *a, FILE * trace)
{
	GenericVisualSampleEntryAtom *p = (GenericVisualSampleEntryAtom *)a;
	fprintf(trace, "<VisualSampleDescription%s DataReferenceIndex=\"%d\" Version=\"%d\" Revision=\"%d\" Vendor=\"%d\" TemporalQuality=\"%d\" SpacialQuality=\"%d\" Width=\"%d\" Height=\"%d\" HorizontalResolution=\"%d\" VerticalResolution=\"%d\" CompressorName=\"%s\" BitDepth=\"%d\">\n", 
		ATOM_NAME, p->dataReferenceIndex, p->version, p->revision, p->vendor, p->temporal_quality, p->spacial_quality, p->Width, p->Height, p->horiz_res, p->vert_res, p->compressor_name, p->bit_depth);
	a->type = p->EntryType;
	DumpAtom(a, trace);
	a->type = GenericVisualSampleEntryAtomType;
	fprintf(trace, "</VisualSampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err gnra_dump(Atom *a, FILE * trace)
{
	GenericAudioSampleEntryAtom *p = (GenericAudioSampleEntryAtom *)a;
	fprintf(trace, "<AudioSampleDescription%s DataReferenceIndex=\"%d\" Version=\"%d\" Revision=\"%d\" Vendor=\"%d\" ChannelCount=\"%d\" BitsPerSample=\"%d\" Samplerate=\"%d\">\n", 
		ATOM_NAME, p->dataReferenceIndex, p->version, p->revision, p->vendor, p->channel_count, p->bitspersample, p->samplerate_hi);
	a->type = p->EntryType;
	DumpAtom(a, trace);
	a->type = GenericAudioSampleEntryAtomType;
	fprintf(trace, "</AudioSampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err edts_dump(Atom *a, FILE * trace)
{
	EditAtom *p;

	p = (EditAtom *)a;
	fprintf(trace, "<Edit%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</Edit%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err udta_dump(Atom *a, FILE * trace)
{
	UserDataAtom *p;
	UserDataMap *map;
	u32 i;
	char name[5];

	p = (UserDataAtom *)a;
	fprintf(trace, "<UserData%s>\n", ATOM_NAME);
	DumpAtom(a, trace);

	for (i = 0; i < ChainGetCount(p->recordList); i++) {
		map = (UserDataMap*) ChainGetEntry(p->recordList, i);
		MP4TypeToString(map->atomType, name);
		fprintf(trace, "<UDTARecord Type=\"%s\">\n", name);
		AtomListDump(map->atomList, trace);
		fprintf(trace, "</UDTARecord>\n");
	}
	fprintf(trace, "</UserData%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err dref_dump(Atom *a, FILE * trace)
{
	DataReferenceAtom *p;

	p = (DataReferenceAtom *)a;
	fprintf(trace, "<DataReference%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</DataReference%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stsd_dump(Atom *a, FILE * trace)
{
	SampleDescriptionAtom *p;
	p = (SampleDescriptionAtom *)a;
	fprintf(trace, "<SampleDescription%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</SampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stts_dump(Atom *a, FILE * trace)
{
	TimeToSampleAtom *p;
	u32 i;
	sttsEntry *t;

	p = (TimeToSampleAtom *)a;
	fprintf(trace, "<TimeToSample%s EntryCount=\"%d\">\n", ATOM_NAME, ChainGetCount(p->entryList));
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	for (i=0; i<ChainGetCount(p->entryList); i++) {
		t = ChainGetEntry(p->entryList, i);
		fprintf(trace, "<TimeToSampleEntry SampleDelta=\"%d\" SampleCount=\"%d\"/>\n", t->sampleDelta, t->sampleCount);
	}
	fprintf(trace, "</TimeToSample%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err ctts_dump(Atom *a, FILE * trace)
{
	CompositionOffsetAtom *p;
	u32 i;
	dttsEntry *t;

	p = (CompositionOffsetAtom *)a;
	fprintf(trace, "<CompositionOffset%s EntryCount=\"%d\">\n", ATOM_NAME, ChainGetCount(p->entryList));
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	for (i=0; i<ChainGetCount(p->entryList); i++) {
		t = ChainGetEntry(p->entryList, i);
		fprintf(trace, "<CompositionOffsetEntry CompositionOffset=\"%d\" SampleCount=\"%d\"/>\n", t->decodingOffset, t->sampleCount);
	}
	fprintf(trace, "</CompositionOffset%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stsh_dump(Atom *a, FILE * trace)
{
	ShadowSyncAtom *p;
	u32 i;
	stshEntry *t;

	p = (ShadowSyncAtom *)a;
	fprintf(trace, "<SyncShadow%s EntryCount=\"%d\">\n", ATOM_NAME, ChainGetCount(p->entries));
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	for (i=0; i<ChainGetCount(p->entries); i++) {
		t = ChainGetEntry(p->entries, i);
		fprintf(trace, "<SyncShadowEntry ShadowedSample=\"%d\" SyncSample=\"%d\"/>\n", t->shadowedSampleNumber, t->syncSampleNumber);
	}
	fprintf(trace, "</SyncShadow%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err elst_dump(Atom *a, FILE * trace)
{
	EditListAtom *p;
	u32 i;
	edtsEntry *t;

	p = (EditListAtom *)a;
	fprintf(trace, "<EditList%s EntryCount=\"%d\">\n", ATOM_NAME, ChainGetCount(p->entryList));
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	for (i=0; i<ChainGetCount(p->entryList); i++) {
		t = ChainGetEntry(p->entryList, i);
		fprintf(trace, "<EditListEntry Duration=\""LLD"\" MediaTime=\""LLD"\" MediaRate=\"%d\"/>\n", t->segmentDuration, t->mediaTime, t->mediaRate);
	}
	fprintf(trace, "</EditList%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stsc_dump(Atom *a, FILE * trace)
{
	SampleToChunkAtom *p;
	u32 i;
	stscEntry *t;

	p = (SampleToChunkAtom *)a;
	fprintf(trace, "<SampleToChunk%s EntryCount=\"%d\">\n", ATOM_NAME, ChainGetCount(p->entryList));
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	for (i=0; i<ChainGetCount(p->entryList); i++) {
		t = ChainGetEntry(p->entryList, i);
		fprintf(trace, "<SampleToChunkEntry FirstChunk=\"%d\" SamplesPerChunk=\"%d\" SampleDescriptionIndex=\"%d\"/>\n", t->firstChunk, t->samplesPerChunk, t->sampleDescriptionIndex);
	}
	fprintf(trace, "</SampleToChunk%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stsz_dump(Atom *a, FILE * trace)
{
	SampleSizeAtom *p;
	u32 i;
	p = (SampleSizeAtom *)a;

	fprintf(trace, "<%s%s SampleCount=\"%d\"", (a->type == SampleSizeAtomType) ? "SampleSize" : "CompactSampleSize", ATOM_NAME, p->sampleCount);
	if (a->type == SampleSizeAtomType) {
		if (p->sampleSize) {
			fprintf(trace, " ConstantSampleSize=\"%d\"", p->sampleSize);
		}
	} else {
		fprintf(trace, " SampleSizeBits=\"%d\"", p->sampleSize);
	}
	fprintf(trace, ">\n");

	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if ((a->type != SampleSizeAtomType) || !p->sampleSize) {
		if (!p->sizes) {
			fprintf(trace, "<!--WARNING: No Sample Size indications-->\n");
		} else {
			for (i=0; i<p->sampleCount; i++) {
				fprintf(trace, "<SampleSizeEntry Size=\"%d\"/>\n", p->sizes[i]);
			}
		}
	}
	fprintf(trace, "</%s%s>\n", (a->type == SampleSizeAtomType) ? "SampleSize" : "CompactSampleSize", ATOM_NAME);
	return M4OK;
}

M4Err stco_dump(Atom *a, FILE * trace)
{
	ChunkOffsetAtom *p;
	u32 i;

	p = (ChunkOffsetAtom *)a;
	fprintf(trace, "<ChunkOffset%s EntryCount=\"%d\">\n", ATOM_NAME, p->entryCount);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (!p->offsets) {
		fprintf(trace, "<!--Warning: No Chunk Offsets indications-->\n");
	} else {
		for (i=0; i<p->entryCount; i++) {
			fprintf(trace, "<ChunkEntry offset=\"%d\"/>\n", p->offsets[i]);
		}
	}
	fprintf(trace, "</ChunkOffset%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stss_dump(Atom *a, FILE * trace)
{
	SyncSampleAtom *p;
	u32 i;

	p = (SyncSampleAtom *)a;
	fprintf(trace, "<SyncSample%s EntryCount=\"%d\">\n", ATOM_NAME, p->entryCount);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (!p->sampleNumbers) {
		fprintf(trace, "<!--Warning: No Key Frames indications-->\n");
	} else {
		for (i=0; i<p->entryCount; i++) {
			fprintf(trace, "<SyncSampleEntry sampleNumber=\"%d\"/>\n", p->sampleNumbers[i]);
		}
	}
	fprintf(trace, "</SyncSample%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stdp_dump(Atom *a, FILE * trace)
{
	DegradationPriorityAtom *p;
	u32 i;

	p = (DegradationPriorityAtom *)a;
	fprintf(trace, "<DegradationPriority%s EntryCount=\"%d\">\n", ATOM_NAME, p->entryCount);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (!p->priorities) {
		fprintf(trace, "<!--Warning: No Degradation Priority indications-->\n");
	} else {
		for (i=0; i<p->entryCount; i++) {
			fprintf(trace, "<DegradationPriorityEntry DegradationPriority=\"%d\"/>\n", p->priorities[i]);
		}
	}
	fprintf(trace, "</DegradationPriority%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err co64_dump(Atom *a, FILE * trace)
{
	ChunkLargeOffsetAtom *p;
	u32 i;

	p = (ChunkLargeOffsetAtom *)a;
	fprintf(trace, "<ChunkLargeOffset%s EntryCount=\"%d\"\n", ATOM_NAME, p->entryCount);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (!p->offsets) {
		fprintf(trace, "<Warning: No Chunk Offsets indications/>\n");
	} else {
		for (i=0; i<p->entryCount; i++) {
			fprintf(trace, "<ChunkOffsetEntry offset=\""LLD"\"/>\n", p->offsets[i]);
		}
	}
	fprintf(trace, "</ChunkLargeOffset%s>n", ATOM_NAME);
	return M4OK;
}

M4Err esds_dump(Atom *a, FILE * trace)
{
	ESDAtom *p;

	p = (ESDAtom *)a;
	fprintf(trace, "<MPEG4ESDescriptor%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	if (p->desc) {
		OD_DumpDescriptor(p->desc, trace, 1, 1);
	} else {
		fprintf(trace, "<!--INVALID MP4 FILE: ESDescriptor not present in MPEG Sample Description or corrupted-->\n");
	}
	fprintf(trace, "</MPEG4ESDescriptor%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err minf_dump(Atom *a, FILE * trace)
{
	MediaInformationAtom *p;

	p = (MediaInformationAtom *)a;
	fprintf(trace, "<MediaInformation%s>\n", ATOM_NAME);
	DumpAtom(a, trace);

	AtomDump(p->InfoHeader, trace);	
	AtomDump(p->dataInformation, trace);	
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</MediaInformation%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tkhd_dump(Atom *a, FILE * trace)
{
	TrackHeaderAtom *p;
	p = (TrackHeaderAtom *)a;
	fprintf(trace, "<TrackHeader%s ", ATOM_NAME);

	fprintf(trace, "CreationTime=\""LLD"\" ModificationTime=\""LLD"\" TrackID=\"%d\" Duration=\""LLD"\"",
		p->creationTime, p->modificationTime, p->trackID, p->duration);
	
	if (p->volume) {
		fprintf(trace, " Volume=\"%.2f\"", (Float)p->volume / 256);
	} else if (p->width || p->height) {
		fprintf(trace, " Width=\"%.2f\" Height=\"%.2f\"", (Float)p->width / 65536, (Float)p->height / 65536);
		if (p->layer) fprintf(trace, " Layer=\"%d\"", p->layer);
		if (p->alternate_group) fprintf(trace, " AlternateGroup=\"%d\"", p->alternate_group);
	}
	fprintf(trace, ">\n");
	if (p->width || p->height) {
		fprintf(trace, "<Matrix m11=\"0x%.8x\" m12=\"0x%.8x\" m13=\"0x%.8x\" \
								m21=\"0x%.8x\" m22=\"0x%.8x\" m23=\"0x%.8x\" \
								m31=\"0x%.8x\" m32=\"0x%.8x\" m33=\"0x%.8x\"/>",
								p->matrix[0], p->matrix[1], p->matrix[2],
								p->matrix[3], p->matrix[4], p->matrix[5],
								p->matrix[6], p->matrix[7], p->matrix[8]);
	}
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	fprintf(trace, "</TrackHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tref_dump(Atom *a, FILE * trace)
{
	TrackReferenceAtom *p;

	p = (TrackReferenceAtom *)a;
	fprintf(trace, "<TrackReference%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</TrackReference%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err mdia_dump(Atom *a, FILE * trace)
{
	MediaAtom *p;

	p = (MediaAtom *)a;
	fprintf(trace, "<Media%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	AtomDump(p->mediaHeader, trace);
	AtomDump(p->handler, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</Media%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err defa_dump(Atom *a, FILE * trace)
{
	char name[5];
	UnknownAtom *p;

	p = (UnknownAtom *)a;
	MP4TypeToString(a->type, name);
	fprintf(trace, "<Unknown%s Type=\"%s\" Size=\"%d\"/>\n", ATOM_NAME, name, p->dataSize);
	return M4OK;
}

M4Err void_dump(Atom *a, FILE * trace)
{
	fprintf(trace, "<Void %s/>\n", ATOM_NAME);
	return M4OK;
}

M4Err ftyp_dump(Atom *a, FILE * trace)
{
	FileTypeAtom *p;
	char name[5];
	u32 i;

	p = (FileTypeAtom *)a;
	MP4TypeToString(p->majorBrand, name);
	fprintf(trace, "<FileType%s MajorBrand=\"%s\" MinorVersion=\"%d\">\n", ATOM_NAME, name, p->minorVersion);
	DumpAtom(a, trace);

	for (i=0; i<p->altCount; i++) {
		MP4TypeToString(p->altBrand[i], name);
		fprintf(trace, "<BrandEntry AlternateBrand=\"%s\"/>\n", name);
	}
	fprintf(trace, "</FileType%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err padb_dump(Atom *a, FILE * trace)
{
	PaddingBitsAtom *p;
	u32 i;
	
	p = (PaddingBitsAtom *)a;
	fprintf(trace, "<PaddingBits%s EntryCount=\"%d\">\n", ATOM_NAME, p->SampleCount);
	DumpAtom(a, trace);
	for (i=0; i<p->SampleCount; i+=1) {
		fprintf(trace, "<PaddingBitsEntry PaddingBits=\"%d\"/>\n", p->padbits[i]);
	}
	fprintf(trace, "</PaddingBits%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err stsf_dump(Atom *a, FILE * trace)
{
	SampleFragmentAtom *p;
	stsfEntry *ent;
	u32 i, j, count;

	
	p = (SampleFragmentAtom *)a;
	count = ChainGetCount(p->entryList);
	fprintf(trace, "<SampleFragment%s EntryCount=\"%d\">\n", ATOM_NAME, count);
	DumpAtom(a, trace);

	for (i=0; i<count; i++) {
		ent = ChainGetEntry(p->entryList, i);
		fprintf(trace, "<SampleFragmentEntry SampleNumber=\"%d\" FragmentCount=\"%d\">\n", ent->SampleNumber, ent->fragmentCount);
		for (j=0;j<ent->fragmentCount;j++) fprintf(trace, "<FragmentSizeEntry size=\"%d\"/>\n", ent->fragmentSizes[j]);
		fprintf(trace, "</SampleFragmentEntry>\n");
	}

	fprintf(trace, "</SampleFragment%s>\n", ATOM_NAME);
	return M4OK;
}


//3GPP atoms
M4Err amr3_dump(Atom *a, FILE * trace)
{
	AMRSampleEntryAtom *p;

	p = (AMRSampleEntryAtom *)a;
	fprintf(trace, "<AMRSampleDescription%s", ATOM_NAME);
	base_audio_entry_dump((AudioSampleEntryAtom *)p, trace);
	fprintf(trace, " AMRMode=\"%s\">\n", p->type==AMRSampleEntryAtomType ? "Normal" : "Wide-Band");

	DumpAtom(a, trace);

	if (p->amr_info) {
		AtomDump(p->amr_info, trace);
	} else {
		fprintf(trace, "<!--INVALID 3GPP FILE: AMR Config not present in AMR Sample Description-->\n");
	}
	fprintf(trace, "</AMRSampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err damr_dump(Atom *a, FILE * trace)
{
	char name[5];
	AMRConfigAtom *p;

	p = (AMRConfigAtom *)a;
	name[4] = 0;
	MP4TypeToString(p->vendor, name);
	fprintf(trace, "<AMRConfiguration%s Vendor=\"%s\" Version=\"%d\"", ATOM_NAME, name, p->decoder_version);
	fprintf(trace, " FramesPerSample=\"%d\" SupportedModes=\"%x\" ModeRotating=\"%d\"", p->frames_per_sample, p->mode_set, p->mode_change_period);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);
	fprintf(trace, "</AMRConfiguration%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err h263_dump(Atom *a, FILE * trace)
{
	H263SampleEntryAtom *p;

	p = (H263SampleEntryAtom *)a;
	fprintf(trace, "<H263SampleDescription%s", ATOM_NAME);
	base_visual_entry_dump((VisualSampleEntryAtom *)p, trace);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);

	if (p->h263_config) {
		AtomDump(p->h263_config, trace);
	} else {
		fprintf(trace, "<!--INVALID 3GPP FILE: H263 Config not present in H263 Sample Description-->\n");
	}
	fprintf(trace, "</H263SampleDescription%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err d263_dump(Atom *a, FILE * trace)
{
	char name[5];
	H263ConfigAtom *p;

	p = (H263ConfigAtom *)a;
	name[4] = 0;
	MP4TypeToString(p->vendor, name);
	fprintf(trace, "<H263Configuration%s Vendor=\"%s\" Version=\"%d\"", ATOM_NAME, name, p->decoder_version);
	fprintf(trace, " Profile=\"%d\" Level=\"%d\"", p->Profile, p->Level);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);
	fprintf(trace, "</H263Configuration%s>\n", ATOM_NAME);
	return M4OK;
}


M4Err avcc_dump(Atom *a, FILE * trace)
{
	u32 i;
	AVCConfigurationAtom *p = (AVCConfigurationAtom *) a;
	fprintf(trace, "<AVCConfiguration%s>\n", ATOM_NAME);

	fprintf(trace, "<AVCDecoderConfigurationRecord configurationVersion=\"%d\" AVCProfileIndication=\"%d\" profile_compatibility=\"%d\" AVCLevelIndication=\"%d\" nal_unit_size=\"%d\">\n", 
					p->config->configurationVersion, p->config->AVCProfileIndication, p->config->profile_compatibility, p->config->AVCLevelIndication, p->config->nal_unit_size);

	for (i=0; i<p->config->numSequenceParameterSets; i++) {
		AVCConfigSlot *c = &p->config->sequenceParameterSets[i];
		fprintf(trace, "<sequenceParameterSets size=\"%d\" content=\"", c->size);
		DumpData(trace, c->data, c->size);
		fprintf(trace, "\"/>\n");
	}
	for (i=0; i<p->config->numPictureParameterSets; i++) {
		AVCConfigSlot *c = &p->config->pictureParameterSets[i];
		fprintf(trace, "<pictureParameterSets size=\"%d\" content=\"", c->size);
		DumpData(trace, c->data, c->size);
		fprintf(trace, "\"/>\n");
	}
	fprintf(trace, "</AVCDecoderConfigurationRecord>\n");

	DumpAtom(a, trace);
	fprintf(trace, "</AVCConfiguration%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err m4ds_dump(Atom *a, FILE * trace)
{
	u32 i;
	MPEG4ExtensionDescriptorsAtom *p = (MPEG4ExtensionDescriptorsAtom *) a;
	fprintf(trace, "<MPEG4ExtensionDescriptors%s>\n", ATOM_NAME);

	for (i=0; i<ChainGetCount(p->descriptors); i++) {
		Descriptor *desc = ChainGetEntry(p->descriptors, i);
		OD_DumpDescriptor(desc, trace, 1, 1);
	}
	DumpAtom(a, trace);
	fprintf(trace, "</MPEG4ExtensionDescriptors%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err btrt_dump(Atom *a, FILE * trace)
{
	MPEG4BitRateAtom *p = (MPEG4BitRateAtom*)a;
	fprintf(trace, "<MPEG4BitRate%s BufferSizeDB=\"%d\" avgBitRate=\"%d\" maxBitRate=\"%d\">\n", ATOM_NAME, p->bufferSizeDB, p->avgBitrate, p->maxBitrate);
	DumpAtom(a, trace);
	fprintf(trace, "</MPEG4BitRate%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err avc1_dump(Atom *a, FILE * trace)
{
	AVCSampleEntryAtom *p = (AVCSampleEntryAtom *)a;

	fprintf(trace, "<AVCSampleEntry%s>\n", ATOM_NAME);
	if (p->avc_config) AtomDump(p->avc_config, trace);
	if (p->descr) AtomDump(p->descr, trace);
	if (p->bitrate) AtomDump(p->bitrate, trace);
	DumpAtom(a, trace);
	fprintf(trace, "</AVCSampleEntry%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err ftab_dump(Atom *a, FILE * trace)
{
	u32 i;
	FontTableAtom *p = (FontTableAtom *)a;
	fprintf(trace, "<FontTable%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	for (i=0; i<p->entry_count; i++) {
		fprintf(trace, "<FontRecord ID=\"%d\" name=\"%s\"/>\n", p->fonts[i].fontID, p->fonts[i].fontName ? p->fonts[i].fontName : "NULL");
	}
	fprintf(trace, "</FontTable%s>\n", ATOM_NAME);
	return M4OK;
}

static void gpp_dump_rgba(FILE * trace, char *name, u32 col)
{
	fprintf(trace, "%s=\"%x %x %x %x\"", name, (col>>16)&0xFF, (col>>8)&0xFF, (col)&0xFF, (col>>24)&0xFF);
}
static void gpp_dump_box(FILE * trace, BoxRecord *rec)
{
	fprintf(trace, "<BoxRecord top=\"%d\" left=\"%d\" bottom=\"%d\" right=\"%d\"/>\n", rec->top, rec->left, rec->bottom, rec->right);
}
static void gpp_dump_style(FILE * trace, StyleRecord *rec)
{
	fprintf(trace, "<StyleRecord startChar=\"%d\" endChar=\"%d\" fontID=\"%d\" styles=\"", rec->startChar, rec->endChar, rec->fontID);
	if (!rec->style_flags) {
		fprintf(trace, "Normal");
	} else {
		if (rec->style_flags & 1) fprintf(trace, "Bold ");
		if (rec->style_flags & 2) fprintf(trace, "Italic ");
		if (rec->style_flags & 4) fprintf(trace, "Underlined ");
	}
	fprintf(trace, "\" fontSize=\"%d\" ", rec->font_size);
	gpp_dump_rgba(trace, "text-color", rec->text_color);
	fprintf(trace, "/>\n");
}

M4Err tx3g_dump(Atom *a, FILE * trace)
{
	TextSampleEntryAtom *p = (TextSampleEntryAtom *)a;
	fprintf(trace, "<TextSampleEntry%s dataReferenceIndex=\"%d\" displayFlags=\"%x\" horizontal-justification=\"%d\" vertical-justification=\"%d\" ", ATOM_NAME,
			p->dataReferenceIndex, p->displayFlags, p->horizontal_justification, p->vertical_justification);

	gpp_dump_rgba(trace, "background-color", p->back_color);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);

	fprintf(trace, "<DefaultBox>\n");
	gpp_dump_box(trace, &p->default_box);
	fprintf(trace, "</DefaultBox>\n");
	fprintf(trace, "<DefaultStyle>\n");
	gpp_dump_style(trace, &p->default_style);
	fprintf(trace, "</DefaultStyle>\n");
	AtomDump(p->font_table, trace);
	fprintf(trace, "</TextSampleEntry%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err styl_dump(Atom *a, FILE * trace)
{
	u32 i;
	TextStyleAtom*p = (TextStyleAtom*)a;
	fprintf(trace, "<TextStyle%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	for (i=0; i<p->entry_count; i++) gpp_dump_style(trace, &p->styles[i]);
	fprintf(trace, "</TextStyle%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err hlit_dump(Atom *a, FILE * trace)
{
	TextHighlightAtom*p = (TextHighlightAtom*)a;
	fprintf(trace, "<TextHighlight%s startcharoffset=\"%d\" endcharoffset=\"%d\">\n", ATOM_NAME, p->startcharoffset, p->endcharoffset);
	DumpAtom(a, trace);
	fprintf(trace, "</TextHighlight%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err hclr_dump(Atom *a, FILE * trace)
{
	TextHighlightColorAtom*p = (TextHighlightColorAtom*)a;
	fprintf(trace, "<TextHighlight%s ", ATOM_NAME);
	gpp_dump_rgba(trace, "highlight_color", p->hil_color);
	fprintf(trace, ">\n");
	DumpAtom(a, trace);
	fprintf(trace, "</TextHighlight%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err krok_dump(Atom *a, FILE * trace)
{
	u32 i;
	TextKaraokeAtom*p = (TextKaraokeAtom*)a;
	fprintf(trace, "<TextKaraoke%s highlight_starttime=\"%d\">\n", ATOM_NAME, p->highlight_starttime);
	DumpAtom(a, trace);
	for (i=0; i<p->entrycount; i++) {
		fprintf(trace, "<KaraokeRecord highlight_endtime=\"%d\" start_charoffset=\"%d\" end_charoffset=\"%d\"/>\n", p->records[i].highlight_endtime, p->records[i].start_charoffset, p->records[i].end_charoffset);
	}
	fprintf(trace, "</TextKaraoke%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err dlay_dump(Atom *a, FILE * trace)
{
	TextScrollDelayAtom*p = (TextScrollDelayAtom*)a;
	fprintf(trace, "<TextScrollDelay%s scroll_delay=\"%d\">\n", ATOM_NAME, p->scroll_delay);
	DumpAtom(a, trace);
	fprintf(trace, "</TextScrollDelay%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err href_dump(Atom *a, FILE * trace)
{
	TextHyperTextAtom*p = (TextHyperTextAtom*)a;
	fprintf(trace, "<TextHyperText%s startcharoffset=\"%d\" startcharoffset=\"%d\" URL=\"%s\" altString=\"%s\">\n", ATOM_NAME, p->startcharoffset, p->endcharoffset, p->URL ? p->URL : "NULL", p->URL_hint ? p->URL_hint : "NULL");
	DumpAtom(a, trace);
	fprintf(trace, "</TextHyperText%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err tbox_dump(Atom *a, FILE * trace)
{
	TextBoxAtom*p = (TextBoxAtom*)a;
	fprintf(trace, "<TextBox%s>\n", ATOM_NAME);
	gpp_dump_box(trace, &p->box);
	DumpAtom(a, trace);
	fprintf(trace, "</TextBox%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err blnk_dump(Atom *a, FILE * trace)
{
	TextBlinkAtom*p = (TextBlinkAtom*)a;
	fprintf(trace, "<TextBlink%s start_charoffset=\"%d\" end_charoffset=\"%d\">\n", ATOM_NAME, p->startcharoffset, p->endcharoffset);
	DumpAtom(a, trace);
	fprintf(trace, "</TextBlink%s>\n", ATOM_NAME);
	return M4OK;
}
M4Err twrp_dump(Atom *a, FILE * trace)
{
	TextWrapAtom*p = (TextWrapAtom*)a;
	fprintf(trace, "<TextWrap%s wrap_flag=\"%s\">\n", ATOM_NAME, p->wrap_flag ? ( (p->wrap_flag>1) ? "Reserved" : "Automatic" ) : "No Wrap");
	DumpAtom(a, trace);
	fprintf(trace, "</TextWrap%s>\n", ATOM_NAME);
	return M4OK;
}


M4Err hinf_dump(Atom *a, FILE * trace)
{
	HintInfoAtom *p;

	p = (HintInfoAtom *)a;
	fprintf(trace, "<HintInfo%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	AtomListDump(p->atomList, trace);
	fprintf(trace, "</HintInfo%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err trpy_dump(Atom *a, FILE * trace)
{
	TrpyAtom *p;

	p = (TrpyAtom *)a;
	fprintf(trace, "<LargeTotalRTPBytes%s RTPBytesSent=\""LLD"\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</LargeTotalRTPBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err totl_dump(Atom *a, FILE * trace)
{
	TotlAtom *p;

	p = (TotlAtom *)a;
	fprintf(trace, "<TotalRTPBytes%s RTPBytesSent=\"%d\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</TotalRTPBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err nump_dump(Atom *a, FILE * trace)
{
	NumpAtom *p;

	p = (NumpAtom *)a;
	fprintf(trace, "<LargeTotalPacket%s PacketsSent=\""LLD"\">\n", ATOM_NAME, p->nbPackets);
	DumpAtom(a, trace);
	fprintf(trace, "</LargeTotalPacket%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err npck_dump(Atom *a, FILE * trace)
{
	NpckAtom *p;

	p = (NpckAtom *)a;
	fprintf(trace, "<TotalPacket%s packetsSent=\"%d\">\n", ATOM_NAME, p->nbPackets);
	DumpAtom(a, trace);
	fprintf(trace, "</TotalPacket%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tpyl_dump(Atom *a, FILE * trace)
{
	TpylAtom *p;

	p = (TpylAtom *)a;
	fprintf(trace, "<LargeTotalMediaBytes%s BytesSent=\""LLD"\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</LargeTotalMediaBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tpay_dump(Atom *a, FILE * trace)
{
	TpayAtom *p;

	p = (TpayAtom *)a;
	fprintf(trace, "<TotalMediaBytes%s BytesSent=\"%d\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</TotalMediaBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err maxr_dump(Atom *a, FILE * trace)
{
	MaxrAtom *p;
	p = (MaxrAtom *)a;
	fprintf(trace, "<MaxDataRate%s MaxDataRate=\"%d\" Granularity=\"%d\">\n", ATOM_NAME, p->maxDataRate, p->granularity);
	DumpAtom(a, trace);
	fprintf(trace, "</MaxDataRate%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err dmed_dump(Atom *a, FILE * trace)
{
	DmedAtom *p;

	p = (DmedAtom *)a;
	fprintf(trace, "<BytesFromMediaTrack%s BytesSent=\""LLD"\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</BytesFromMediaTrack%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err dimm_dump(Atom *a, FILE * trace)
{
	DimmAtom *p;

	p = (DimmAtom *)a;
	fprintf(trace, "<ImmediateDataBytes%s BytesSent=\""LLD"\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</ImmediateDataBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err drep_dump(Atom *a, FILE * trace)
{
	DrepAtom *p;

	p = (DrepAtom *)a;
	fprintf(trace, "<RepeatedDataBytes%s RepeatedBytes=\""LLD"\">\n", ATOM_NAME, p->nbBytes);
	DumpAtom(a, trace);
	fprintf(trace, "</RepeatedDataBytes%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tmin_dump(Atom *a, FILE * trace)
{
	TminAtom *p;

	p = (TminAtom *)a;
	fprintf(trace, "<MinTransmissionTime%s MinimumTransmitTime=\"%d\">\n", ATOM_NAME, p->minTime);
	DumpAtom(a, trace);
	fprintf(trace, "</MinTransmissionTime%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tmax_dump(Atom *a, FILE * trace)
{
	TmaxAtom *p;

	p = (TmaxAtom *)a;
	fprintf(trace, "<MaxTransmissionTime%s MaximumTransmitTime=\"%d\">\n", ATOM_NAME, p->maxTime);
	DumpAtom(a, trace);
	fprintf(trace, "</MaxTransmissionTime%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err pmax_dump(Atom *a, FILE * trace)
{
	PmaxAtom *p;

	p = (PmaxAtom *)a;
	fprintf(trace, "<MaxPacketSize%s MaximumSize=\"%d\">\n", ATOM_NAME, p->maxSize);
	DumpAtom(a, trace);
	fprintf(trace, "</MaxPacketSize%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err dmax_dump(Atom *a, FILE * trace)
{
	DmaxAtom *p;

	p = (DmaxAtom *)a;
	fprintf(trace, "<MaxPacketDuration%s MaximumDuration=\"%d\">\n", ATOM_NAME, p->maxDur);
	DumpAtom(a, trace);
	fprintf(trace, "</MaxPacketDuration%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err payt_dump(Atom *a, FILE * trace)
{
	PaytAtom *p;

	p = (PaytAtom *)a;
	fprintf(trace, "<PayloadType%s PayloadID=\"%d\" PayloadString=\"%s\">\n", ATOM_NAME, p->payloadCode, p->payloadString);
	DumpAtom(a, trace);
	fprintf(trace, "</PayloadType%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err name_dump(Atom *a, FILE * trace)
{
	NameAtom *p;

	p = (NameAtom *)a;
	fprintf(trace, "<Name%s Name=\"%s\">\n", ATOM_NAME, p->string);
	DumpAtom(a, trace);
	fprintf(trace, "</Name%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err rely_dump(Atom *a, FILE * trace)
{
	RelyHintEntry *p;

	p = (RelyHintEntry *)a;
	fprintf(trace, "<RelyTransmission%s Prefered=\"%d\" required=\"%d\">\n", ATOM_NAME, p->prefered, p->required);
	DumpAtom(a, trace);
	fprintf(trace, "</RelyTransmission%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err snro_dump(Atom *a, FILE * trace)
{
	SeqOffHintEntry *p;

	p = (SeqOffHintEntry *)a;
	fprintf(trace, "<PacketSequenceOffset%s SeqNumOffset=\"%d\">\n", ATOM_NAME, p->SeqOffset);
	DumpAtom(a, trace);
	fprintf(trace, "</PacketSequenceOffset%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tims_dump(Atom *a, FILE * trace)
{
	TSHintEntry *p;

	p = (TSHintEntry *)a;
	fprintf(trace, "<RTPTimeScale%s TimeScale=\"%d\">\n", ATOM_NAME, p->timeScale);
	DumpAtom(a, trace);
	fprintf(trace, "</RTPTimeScale%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tsro_dump(Atom *a, FILE * trace)
{
	TimeOffHintEntry *p;

	p = (TimeOffHintEntry *)a;
	fprintf(trace, "<TimeStampOffset%s TimeStampOffset=\"%d\">\n", ATOM_NAME, p->TimeOffset);
	DumpAtom(a, trace);
	fprintf(trace, "</TimeStampOffset%s>\n", ATOM_NAME);
	return M4OK;
}


M4Err ghnt_dump(Atom *a, FILE * trace)
{
	char name[5];
	HintSampleEntryAtom *p;

	p = (HintSampleEntryAtom *)a;
	MP4TypeToString(p->type, name);
	fprintf(trace, "<GenericHintSampleEntry%s EntrySubType=\"%s\" DataReferenceIndex=\"%d\" HintTrackVersion=\"%d\" LastCompatibleVersion=\"%d\" MaxPacketSize=\"%d\">\n", 
		ATOM_NAME, name, p->dataReferenceIndex, p->HintTrackVersion, p->LastCompatibleVersion, p->MaxPacketSize);	
	
	DumpAtom(a, trace);
	AtomListDump(p->HintDataTable, trace);
	fprintf(trace, "</GenericHintSampleEntry%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err hnti_dump(Atom *a, FILE * trace)
{
	HintTrackInfoAtom *p;
	Atom *ptr;
	RTPAtom *rtp;
	char name[5];
	u32 i;

	p = (HintTrackInfoAtom *)a;
	fprintf(trace, "<HintTrackInfo%s>\n", ATOM_NAME);
	DumpAtom(a, trace);

	for (i=0 ;i<ChainGetCount(p->atomList); i++) {
		ptr = ChainGetEntry(p->atomList, i);
		if (ptr->type !=RTPAtomType) {
			AtomDump(ptr, trace);
		} else {
			rtp = (RTPAtom *)ptr;
			MP4TypeToString(rtp->subType, name);
			fprintf(trace, "<RTPInfo%s subType=\"%s\">\n", ATOM_NAME, name);
			fprintf(trace, "<!-- sdp text: %s -->\n", rtp->sdpText);
			fprintf(trace, "</RTPInfo%s>\n", ATOM_NAME);
		}
	}
	fprintf(trace, "</HintTrackInfo%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err sdp_dump(Atom *a, FILE * trace)
{
	SDPAtom *p;

	p = (SDPAtom *)a;
	fprintf(trace, "<SDP%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	fprintf(trace, "<!-- sdp text: %s -->\n", p->sdpText);
	fprintf(trace, "</SDP%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err rtpo_dump(Atom *a, FILE * trace)
{
	RtpoAtom *p;

	p = (RtpoAtom *)a;
	fprintf(trace, "<RTPTimeOffset%s PacketTimeOffset=\"%d\">\n", ATOM_NAME, p->timeOffset);
	DumpAtom(a, trace);
	fprintf(trace, "</RTPTimeOffset%s>\n", ATOM_NAME);
	return M4OK;
}



#ifndef	M4_ISO_NO_FRAGMENTS

M4Err mvex_dump(Atom *a, FILE * trace)
{
	MovieExtendsAtom *p;

	p = (MovieExtendsAtom *)a;
	fprintf(trace, "<MovieExtends%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	AtomListDump(p->TrackExList, trace);
	fprintf(trace, "</MovieExtends%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err trex_dump(Atom *a, FILE * trace)
{
	TrackExtendsAtom *p;

	p = (TrackExtendsAtom *)a;
	fprintf(trace, "<TrackExtends%s TrackID=\"%d\"", ATOM_NAME, p->trackID);

	fprintf(trace, " SampleDescriptionIndex=\"%d\" SampleDuration=\"%d\" SampleSize=\"%d\"", p->def_sample_desc_index, p->def_sample_duration, p->def_sample_size);
	fprintf(trace, " SamplePadding=\"%d\" SampleSync=\"%d\" SampleDegradationPriority=\"%d\"", 
		GET_FRAG_PAD(p->def_sample_flags), GET_FRAG_SYNC(p->def_sample_flags), GET_FRAG_DEG(p->def_sample_flags));

	fprintf(trace, ">\n");
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</TrackExtends%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err moof_dump(Atom *a, FILE * trace)
{
	MovieFragmentAtom *p;

	p = (MovieFragmentAtom *)a;
	fprintf(trace, "<MovieFragment%s TrackFragments=\"%d\">\n", ATOM_NAME, ChainGetCount(p->TrackList));
	DumpAtom(a, trace);

	if (p->mfhd) AtomDump(p->mfhd, trace);
	AtomListDump(p->TrackList, trace);
	fprintf(trace, "</MovieFragment%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err mfhd_dump(Atom *a, FILE * trace)
{
	MovieFragmentHeaderAtom *p;

	p = (MovieFragmentHeaderAtom *)a;
	fprintf(trace, "<MovieFragmentHeader%s FragmentSequenceNumber=\"%d\">\n", ATOM_NAME, p->sequence_number);
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</MovieFragmentHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err traf_dump(Atom *a, FILE * trace)
{
	TrackFragmentAtom *p;

	p = (TrackFragmentAtom *)a;
	fprintf(trace, "<TrackFragment%s>\n", ATOM_NAME);
	DumpAtom(a, trace);
	if (p->tfhd) AtomDump(p->tfhd, trace);
	AtomListDump(p->TrackRuns, trace);
	fprintf(trace, "</TrackFragment%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err tfhd_dump(Atom *a, FILE * trace)
{
	TrackFragmentHeaderAtom *p;

	p = (TrackFragmentHeaderAtom *)a;
	fprintf(trace, "<TrackFragmentHeader%s TrackID=\"%d\"", ATOM_NAME, p->trackID);

	if (p->flags & TF_BASE_OFFSET) {
		fprintf(trace, " BaseDataOffset=\""LLD"\"", p->base_data_offset);
	}
	if (p->flags & TF_SAMPLE_DESC) 
		fprintf(trace, "SampleDescriptionIndex=\"%d\"", p->sample_desc_index);
	if (p->flags & TF_SAMPLE_DUR) 
		fprintf(trace, " SampleDuration=\"%d\"", p->def_sample_duration);
	if (p->flags & TF_SAMPLE_SIZE) 
		fprintf(trace, " SampleSize=\"%d\"", p->def_sample_size);
	if (p->flags & TF_SAMPLE_FLAGS) {
		fprintf(trace, " SamplePadding=\"%d\"", GET_FRAG_PAD(p->def_sample_flags));
		fprintf(trace, " SampleSync=\"%d\"", GET_FRAG_SYNC(p->def_sample_flags));
		fprintf(trace, " SampleDegradationPriority=\"%d\"", GET_FRAG_DEG(p->def_sample_flags));
	}	
	fprintf(trace, ">\n");

	DumpAtom(a, trace);
	FullAtom_dump(a, trace);
	fprintf(trace, "</TrackFragmentHeader%s>\n", ATOM_NAME);
	return M4OK;
}

M4Err trun_dump(Atom *a, FILE * trace)
{
	u32 i;
	TrunEntry *ent;
	TrackFragmentRunAtom *p;

	p = (TrackFragmentRunAtom *)a;
	fprintf(trace, "<TrackRun%s SampleCount=\"%d\"", ATOM_NAME, p->sample_count);

	if (p->flags & TR_DATA_OFFSET) 
		fprintf(trace, " DataOffset=\"%d\"", p->data_offset);
	if (p->flags & TR_FIRST_FLAG) {
		fprintf(trace, " FirstSamplePadding=\"%d\" FirstSampleSync=\"%d\" FirstSampleDegradationPriority=\"%d\"", GET_FRAG_PAD(p->first_sample_flags), GET_FRAG_SYNC(p->first_sample_flags), GET_FRAG_DEG(p->first_sample_flags));
	}
	fprintf(trace, ">\n");
	DumpAtom(a, trace);
	FullAtom_dump(a, trace);

	for (i=0;i<ChainGetCount(p->entries); i++) {
		ent = ChainGetEntry(p->entries, i);

		fprintf(trace, "<TrackRunEntry");

		if (p->flags & TR_DURATION)
			fprintf(trace, " Duration=\"%d\"", ent->Duration);
		if (p->flags & TR_SIZE) 
			fprintf(trace, " Size=\"%d\"", ent->size);		
		if (p->flags & TR_CTS_OFFSET)
			fprintf(trace, " CTSOffset=\"%d\"", ent->CTS_Offset);

		if (p->flags & TR_FLAGS) {
			fprintf(trace, " SamplePadding=\"%d\" Sync=\"%d\" DegradationPriority=\"%d\"", 
				GET_FRAG_PAD(ent->flags), GET_FRAG_SYNC(ent->flags), GET_FRAG_DEG(ent->flags));
		}	
		fprintf(trace, "/>\n");
	}
	fprintf(trace, "</TrackRun%s>", ATOM_NAME);
	return M4OK;
}


#endif

M4Err DTE_Dump(Chain *dte, FILE * trace)
{
	GenericDTE *p;
	ImmediateDTE *i_p;
	SampleDTE *s_p;
	StreamDescDTE *sd_p;
	u32 i, count;

	count = ChainGetCount(dte);
	for (i=0; i<count; i++) {
		p = ChainGetEntry(dte, i);
		switch (p->source) {
		case 0:
			fprintf(trace, "<EmptyDataEntry/>\n");
			break;
		case 1:
			i_p = (ImmediateDTE *) p;
			fprintf(trace, "<ImmediateDataEntry DataSize=\"%d\"/>\n", i_p->dataLength);
			break;
		case 2:
			s_p = (SampleDTE *) p;
			fprintf(trace, "<SampleDataEntry DataSize=\"%d\" SampleOffset=\"%d\" SampleNumber=\"%d\" TrackReference=\"%d\"/>\n", 
				s_p->dataLength, s_p->byteOffset, s_p->sampleNumber, s_p->trackRefIndex);
			break;
		case 3:
			sd_p = (StreamDescDTE *) p;
			fprintf(trace, "<SampleDescriptionEntry DataSize=\"%d\" DescriptionOffset=\"%d\" StreamDescriptionindex=\"%d\" TrackReference=\"%d\"/>\n", 
				sd_p->dataLength, sd_p->byteOffset, sd_p->streamDescIndex, sd_p->trackRefIndex);
			break;
		default:
			fprintf(trace, "<UnknownTableEntry/>\n");
			break;
		}
	}
	return M4OK;
}


M4Err M4H_DumpHintSample(M4File *the_file, u32 trackNumber, u32 SampleNum, FILE * trace)
{
	M4Sample *tmp;
	HintSampleEntryAtom *entry;
	u32 descIndex, count, count2, i;
	M4Err e;
	BitStream *bs;
	HintSample *s;
	TrackAtom *trak;
	RTPPacket *pck;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;
	
	tmp = M4_GetSample(the_file, trackNumber, SampleNum, &descIndex);
	if (!tmp) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, descIndex, (SampleEntryAtom **) &entry, &count);
	if (e) return e;

	bs = NewBitStream(tmp->data, tmp->dataLength, BS_READ);

	//check we can read the sample
	switch (entry->type) {
	case RTPHintSampleEntryAtomType:
		break;
	default:
		return M4NotSupported;
	}

	s = New_HintSample(entry->type);
	Read_HintSample(s, bs, tmp->dataLength);
	DeleteBitStream(bs);

	count = ChainGetCount(s->packetTable);

	fprintf(trace, "<RTPHintSample SampleNumber=\"%d\" DecodingTime=\"%d\" CompositionTime=\"%d\" RandomAccessPoint=\"%d\" PacketCount=\"%d\">\n", SampleNum, tmp->DTS, tmp->DTS+tmp->CTS_Offset, tmp->IsRAP, count);

	for (i=0; i<count; i++) {
		pck = ChainGetEntry(s->packetTable, i);

		fprintf(trace, "<RTPHintPacket PacketNumber=\"%d\" P=\"%d\" X=\"%d\" M=\"%d\" PayloadType=\"%d\"", 
			i+1,  pck->P_bit, pck->X_bit, pck->M_bit, pck->payloadType);

		fprintf(trace, " SequenceNumber=\"%d\" RepeatedPacket=\"%d\" DropablePacket=\"%d\" RelativeTransmissionTime=\"%d\" FullPacketSize=\"%d\">\n", 
			pck->SequenceNumber, pck->R_bit, pck->B_bit, pck->relativeTransTime, Length_RTPPacket(pck));

		
		//TLV is made of Atoms
		count2 = ChainGetCount(pck->TLV);
		if (count2) {
			fprintf(trace, "<PrivateExtensionTable EntryCount=\"%d\">\n", count2);
			AtomListDump(pck->TLV, trace);
			fprintf(trace, "</PrivateExtensionTable>\n");
		}
		//DTE is made of NON atoms
		count2 = ChainGetCount(pck->DataTable);
		if (count2) {
			fprintf(trace, "<PacketDataTable EntryCount=\"%d\">\n", count2);
			DTE_Dump(pck->DataTable, trace);
			fprintf(trace, "</PacketDataTable>\n");
		}
		fprintf(trace, "</RTPHintPacket>\n");
	}

	fprintf(trace, "</RTPHintSample>\n");
	M4_DeleteSample(&tmp);
	Del_HintSample(s);
	return M4OK;
}


static void gpp_dump_box_noatom(FILE * trace, BoxRecord *rec)
{
	fprintf(trace, "<TextBox top=\"%d\" left=\"%d\" bottom=\"%d\" right=\"%d\"/>\n", rec->top, rec->left, rec->bottom, rec->right);
}
static void gpp_dump_style_noatom(FILE * trace, StyleRecord *rec)
{
	if (rec->startChar || rec->endChar) {
		fprintf(trace, "<Style fromChar=\"%d\" toChar=\"%d\" fontID=\"%d\" styles=\"", rec->startChar, rec->endChar - 1, rec->fontID);
	} else {
		fprintf(trace, "<Style fontID=\"%d\" styles=\"", rec->fontID);
	}
	if (!rec->style_flags) {
		fprintf(trace, "Normal");
	} else {
		if (rec->style_flags & 1) fprintf(trace, "Bold ");
		if (rec->style_flags & 2) fprintf(trace, "Italic ");
		if (rec->style_flags & 4) fprintf(trace, "Underlined ");
	}
	fprintf(trace, "\" fontSize=\"%d\" ", rec->font_size);
	gpp_dump_rgba(trace, "color", rec->text_color);
	fprintf(trace, "/>\n");
}

static char *ttd_format_time(u64 ts, u32 timescale, char *szDur)
{
	u32 h, m, s, ms;
	ts = (u32) (( ((Double) (s64) ts)/timescale)*1000.0);
	h = (u32) (ts / 3600000);
	m = (u32) (ts/ 60000) - h*60;
	s = (u32) (ts/1000) - h*3600 - m*60;
	ms = (u32) (ts) - h*3600000 - m*60000 - s*1000;
	sprintf(szDur, "%02d:%02d:%02d.%03d", h, m, s, ms);
	return szDur;
}

M4Err M4_DumpTextTrack(M4File *the_file, u32 track, FILE *dump)
{
	u32 i, j, count, di, len;
	u32 tx, ty, nb_descs;
	Bool has_scroll;
	char szDur[100];

	TrackAtom *trak = GetTrackFromFile(the_file, track);
	if (!trak || (trak->Media->handler->handlerType != M4_TimedTextMediaType)) return M4BadParam;

	fprintf(dump, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
	fprintf(dump, "<!-- GPAC 3GPP Text Stream -->\n");

	tx = trak->Header->matrix[6]; tx >>= 16;
	ty = trak->Header->matrix[7]; ty >>= 16;
	fprintf(dump, "<TextStream version=\"1.0\">\n");
	fprintf(dump, "<TextStreamHeader width=\"%d\" height=\"%d\" layer=\"%d\" translation_x=\"%d\" translation_y=\"%d\">\n", trak->Header->width / 65536, trak->Header->height / 65536, trak->Header->layer, tx, ty);

	nb_descs = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
	for (i=0; i<nb_descs; i++) {
		TextSampleEntryAtom *txt = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, i);

		fprintf(dump, "<TextSampleDescription horizontalJustification=\"");
		switch (txt->horizontal_justification) {
		case 1: fprintf(dump, "center"); break;
		case -1: fprintf(dump, "right"); break;
		default: fprintf(dump, "left"); break;
		}
		fprintf(dump, "\" verticalJustification=\"");
		switch (txt->vertical_justification) {
		case 1: fprintf(dump, "center"); break;
		case -1: fprintf(dump, "bottom"); break;
		default: fprintf(dump, "top"); break;
		}
		fprintf(dump, "\" ");
		gpp_dump_rgba(dump, "backColor", txt->back_color);
		fprintf(dump, " verticalText=\"%s\"", (txt->displayFlags & STF_VerticalText) ? "yes" : "no");
		fprintf(dump, " fillTextRegion=\"%s\"", (txt->displayFlags & STF_FillRegion) ? "yes" : "no");
		fprintf(dump, " continuousKaraoke=\"%s\"", (txt->displayFlags & STF_Karaoke) ? "yes" : "no");
		has_scroll = 0;
		if (txt->displayFlags & STF_ScrollIn) {
			has_scroll = 1;
			if (txt->displayFlags & STF_ScrollOut) fprintf(dump, " scroll=\"InOut\"");
			else fprintf(dump, " scroll=\"In\"");
		} else if (txt->displayFlags & STF_ScrollOut) {
			has_scroll = 1;
			fprintf(dump, " scroll=\"Out\"");
		} else {
			fprintf(dump, " scroll=\"None\"");
		}
		if (has_scroll) {
			u32 mode = (txt->displayFlags & STF_ScrollDirection)>>7;
			switch (mode) {
			case STF_ScrollCredits: fprintf(dump, " scrollType=\"Credits\""); break;
			case STF_ScrollMarquee: fprintf(dump, " scrollType=\"Marquee\""); break;
			case STF_ScrollDown: fprintf(dump, " scrollType=\"Down\""); break;
			case STF_ScrollRight: fprintf(dump, " scrollType=\"Right\""); break;
			default: fprintf(dump, " scrollType=\"Unknown\""); break;
			}
		}
		fprintf(dump, ">\n");
		fprintf(dump, "<FontTable>\n");
		if (txt->font_table) {
			for (j=0; j<txt->font_table->entry_count; j++) {
				fprintf(dump, "<FontTableEntry fontName=\"%s\" fontID=\"%d\"/>\n", txt->font_table->fonts[j].fontName, txt->font_table->fonts[j].fontID);
			
			}
		}
		fprintf(dump, "</FontTable>\n");
		if ((txt->default_box.bottom == txt->default_box.top) || (txt->default_box.right == txt->default_box.left)) {
			txt->default_box.top = txt->default_box.left = 0;
			txt->default_box.right = trak->Header->width / 65536;
			txt->default_box.bottom = trak->Header->height / 65536;
		}
		gpp_dump_box_noatom(dump, &txt->default_box);
		gpp_dump_style_noatom(dump, &txt->default_style);
		fprintf(dump, "</TextSampleDescription>\n");
	}
	fprintf(dump, "</TextStreamHeader>\n");

	count = M4_GetSampleCount(the_file, track);
	for (i=0; i<count; i++) {
		BitStream *bs;
		TextSample *txt;
		M4Sample *s = M4_GetSample(the_file, track, i+1, &di);
		if (!s) continue;

		fprintf(dump, "<TextSample sampleTime=\"%s\"", ttd_format_time(s->DTS, trak->Media->mediaHeader->timeScale, szDur));
		if (nb_descs>1) fprintf(dump, " sampleDescriptionIndex=\"%d\"", di);

		bs = NewBitStream(s->data, s->dataLength, BS_READ);
		txt = M4_ParseTextSample(bs);
		DeleteBitStream(bs);

		if (!txt->len) {
			fprintf(dump, " text=\"\"");
		}else {
			s16 uniLine[10000];
			/*UTF16*/
			if ((txt->len>2) && ((unsigned char) txt->text[0] == (unsigned char) 0xFE) && ((unsigned char) txt->text[1] == (unsigned char) 0xFF)) {
				/*copy 2 more chars because the lib always add 2 '0' at the end for UTF16 end of string*/
				memcpy((char *) uniLine, txt->text+2, sizeof(char) * (txt->len));
				len = utf8_wcslen(uniLine);
			} else {
				char *str;
				str = txt->text;
				len = utf8_mbstowcs(uniLine, 10000, (const char **) &str);
			}
			if (len>=0) {
				uniLine[len] = 0;
				fprintf(dump, " text=\"\'");
				for (j=0; j<len; j++) {
					if ((uniLine[j]=='\n') || (uniLine[j]=='\r') || (uniLine[j]==0x85) || (uniLine[j]==0x2028) || (uniLine[j]==0x2029) ) {
						fprintf(dump, "\'\'");
						if ((uniLine[j]=='\r') && (uniLine[j+1]=='\n')) j++;
					} 
					else {
						switch (uniLine[j]) {
						case '\'': fprintf(dump, "&apos;"); break;
						case '\"': fprintf(dump, "&quot;"); break;
						case '&': fprintf(dump, "&amp;"); break;
						case '>': fprintf(dump, "&gt;"); break;
						case '<': fprintf(dump, "&lt;"); break;
						default:
							if (uniLine[j] < 128) {
								fprintf(dump, "%c", (u8) uniLine[j]);
							} else {
								fprintf(dump, "&#%d;", uniLine[j]);
							}
							break;
						}
					}
				}
				fprintf(dump, "\'\"");
			} else {
				fprintf(dump, "text=\"UNKNOWN UTF ENCODING\"");
			}
		}
		if (txt->highlight_color) {
			fprintf(dump, " ");
			gpp_dump_rgba(dump, "highlightColor", txt->highlight_color->hil_color);
		}
		if (txt->scroll_delay) {
			Double delay = txt->scroll_delay->scroll_delay;
			delay /= trak->Media->mediaHeader->timeScale;
			fprintf(dump, " scrollDelay=\"%g\"", delay);
		}
		if (txt->wrap) fprintf(dump, " wrap=\"%s\"", (txt->wrap->wrap_flag==0x01) ? "Automatic" : "None");

		fprintf(dump, ">\n");

		if (txt->box) gpp_dump_box_noatom(dump, &txt->box->box);
		if (txt->styles) {
			for (j=0; j<txt->styles->entry_count; j++) {
				gpp_dump_style_noatom(dump, &txt->styles->styles[j]);
			}
		}
		for (j=0; j<ChainGetCount(txt->others); j++) {
			Atom *a = ChainGetEntry(txt->others, j);
			switch (a->type) {
			case TextHighlightAtomType:
				fprintf(dump, "<Highlight fromChar=\"%d\" toChar=\"%d\"/>\n", ((TextHighlightAtom *)a)->startcharoffset, ((TextHighlightAtom *)a)->endcharoffset - 1);
				break;
			case TextHyperTextAtomType:
			{
				TextHyperTextAtom *ht = (TextHyperTextAtom *)a;
				fprintf(dump, "<HyperLink fromChar=\"%d\" toChar=\"%d\" URL=\"%s\" URLToolTip=\"%s\"/>\n", 
					ht->startcharoffset, ht->endcharoffset - 1, ht->URL ? ht->URL : "", ht->URL_hint ? ht->URL_hint : "");
			}
				break;
			case TextBlinkAtomType:
				fprintf(dump, "<Blinking fromChar=\"%d\" toChar=\"%d\"/>\n", ((TextBlinkAtom *)a)->startcharoffset, ((TextBlinkAtom *)a)->endcharoffset - 1);
				break;
			case TextKaraokeAtomType:
			{	
				u32 k;
				Double t;
				TextKaraokeAtom *krok = (TextKaraokeAtom *)a;
				t = krok->highlight_starttime;
				t /= trak->Media->mediaHeader->timeScale;
				fprintf(dump, "<Karaoke startTime=\"%g\">\n", t);
				for (k=0; k<krok->entrycount; k++) {
					t = krok->records[k].highlight_endtime;
					t /= trak->Media->mediaHeader->timeScale;
					fprintf(dump, "<KaraokeRange fromChar=\"%d\" toChar=\"%d\" endTime=\"%g\"/>\n", krok->records[k].start_charoffset, krok->records[k].end_charoffset-1, t);
				}
				fprintf(dump, "</Karaoke>\n");
			}
				break;
			}
		}

		fprintf(dump, "</TextSample>\n");
		M4_DeleteSample(&s);
		M4_DeleteTextSample(txt);
	}
	fprintf(dump, "</TextStream>\n");
	return M4OK;
}
