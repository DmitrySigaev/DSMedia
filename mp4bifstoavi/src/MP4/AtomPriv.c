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

Atom *CreateAtom(u32 atomType)
{
	Atom *a;
	switch (atomType) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		a = reftype_New();
		if (a) a->type = atomType;
		return a;
	case FreeSpaceAtomType: return free_New();
	case SkipAtomType:
		a = free_New();
		if (a) a->type = SkipAtomType;
		return a;
	case MediaDataAtomType: return mdat_New();
	case MovieAtomType: return moov_New();
	case MovieHeaderAtomType: return mvhd_New();
	case MediaHeaderAtomType: return mdhd_New();
	case VideoMediaHeaderAtomType: return vmhd_New();
	case SoundMediaHeaderAtomType: return smhd_New();
	case HintMediaHeaderAtomType: return hmhd_New();
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		a = nmhd_New();
		if (a) a->type = atomType;
		return a;
	case SampleTableAtomType: return stbl_New();
	case DataInformationAtomType: return dinf_New();
	case DataEntryURLAtomType: return url_New();
	case DataEntryURNAtomType: return urn_New();
	case CopyrightAtomType: return cprt_New();
	case HandlerAtomType: return hdlr_New();
	case ObjectDescriptorAtomType: return iods_New();
	case TrackAtomType: return trak_New();
	case MPEGSampleEntryAtomType: return mp4s_New();
	case MPEGVisualSampleEntryAtomType: return mp4v_New();
	case MPEGAudioSampleEntryAtomType: return mp4a_New();
	case GenericMediaSampleEntryAtomType: return gnrm_New();
	case GenericVisualSampleEntryAtomType: return gnrv_New();
	case GenericAudioSampleEntryAtomType: return gnra_New();
	case EditAtomType: return edts_New();
	case UserDataAtomType: return udta_New();
	case DataReferenceAtomType: return dref_New();
	case SampleDescriptionAtomType: return stsd_New();
	case TimeToSampleAtomType: return stts_New();
	case CompositionOffsetAtomType: return ctts_New();
	case ShadowSyncAtomType: return stsh_New();
	case EditListAtomType: return elst_New();
	case SampleToChunkAtomType: return stsc_New();
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		a = stsz_New();
		if (a) a->type = atomType;
		return a;
	case ChunkOffsetAtomType: return stco_New();
	case SyncSampleAtomType: return stss_New();
	case DegradationPriorityAtomType: return stdp_New();
	case ChunkLargeOffsetAtomType: return co64_New();
	case ESDAtomType: return esds_New();
	case MediaInformationAtomType: return minf_New();
	case TrackHeaderAtomType: return tkhd_New();
	case TrackReferenceAtomType: return tref_New();
	case MediaAtomType: return mdia_New();
	
	//Hinting
	case RTPHintSampleEntryAtomType:
		a = ghnt_New();
		if (a) a->type = atomType;
		return a;

	case rtpoAtomType: return rtpo_New();
	case HintTrackInfoAtomType: return hnti_New();
	case SDPAtomType: return sdp_New();
	case HintInfoAtomType: return hinf_New();
	case relyHintEntryType: return rely_New();
	case timsHintEntryType: return tims_New();
	case tsroHintEntryType: return tsro_New();
	case snroHintEntryType: return snro_New();
	case trpyAtomType: return trpy_New();
	case numpAtomType: return nump_New();
	case totlAtomType: return totl_New();
	case npckAtomType: return npck_New();
	case tpylAtomType: return tpyl_New();
	case tpayAtomType: return tpay_New();
	case maxrAtomType: return maxr_New();
	case dmedAtomType: return dmed_New();
	case dimmAtomType: return dimm_New();
	case drepAtomType: return drep_New();
	case tminAtomType: return tmin_New();
	case tmaxAtomType: return tmax_New();
	case pmaxAtomType: return pmax_New();
	case dmaxAtomType: return dmax_New();
	case paytAtomType: return payt_New();
	case nameAtomType: return name_New();

	case FileTypeAtomType: return ftyp_New();
	case PaddingBitsAtomType: return padb_New();
	
#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: return mvex_New();
	case TrackExtendsAtomType: return trex_New();
	case MovieFragmentAtomType: return moof_New();
	case MovieFragmentHeaderAtomType: return mfhd_New();
	case TrackFragmentAtomType: return traf_New();
	case TrackFragmentHeaderAtomType: return tfhd_New();
	case TrackFragmentRunAtomType: return trun_New();
#endif 

	case VoidAtomType: return void_New();
	case SampleFragmentAtomType: return stsf_New();

	//3GPP atoms
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		a = amr3_New();
		if (a) a->type = atomType;
		return a;
	case AMRConfigAtomType: return damr_New();
	case H263SampleEntryAtomType: return h263_New();
	case H263ConfigAtomType: return d263_New();

	/*AVC atoms*/
	case AVCConfigurationAtomType: return avcc_New();
	case MPEG4BitRateAtomType: return btrt_New();
	case MPEG4ExtensionDescriptorsAtomType: return m4ds_New();
	case AVCSampleEntryAtomType: return avc1_New();

	/*3GPP streaming text*/
	case FontTableAtomType: return ftab_New();
	case TextSampleEntryAtomType: return tx3g_New();
	case TextStyleAtomType: return styl_New();
	case TextHighlightAtomType: return hlit_New();
	case TextHighlightColorAtomType: return hclr_New();
	case TextKaraokeAtomType: return krok_New();
	case TextScrollDelayAtomType: return dlay_New();
	case TextHyperTextAtomType: return href_New();
	case TextBoxAtomType: return tbox_New();
	case TextBlinkAtomType: return blnk_New();
	case TextWrapAtomType: return twrp_New();

	case ExtendedAtomType:
		a = defa_New();
		if (a) a->type = ExtendedAtomType;
		return a;

	default:
		a = defa_New();
		if (a) {
			a->type = atomType;
			//to identify a default atom regardless of its type
			//we format the UUID (not used ;)
			memset(a->uuid, 1, 16);
		}
		return a;
	}
}


void DelAtom(Atom *a)
{
	switch (a->type) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		reftype_del(a);
		return;
	case FreeSpaceAtomType:
	case SkipAtomType:
		free_del(a);
		return;
	case MediaDataAtomType: mdat_del(a); return;
	case MovieAtomType: moov_del(a); return;
	case MovieHeaderAtomType: mvhd_del(a); return;
	case MediaHeaderAtomType: mdhd_del(a); return;
	case VideoMediaHeaderAtomType: vmhd_del(a); return;
	case SoundMediaHeaderAtomType: smhd_del(a); return;
	case HintMediaHeaderAtomType: hmhd_del(a); return;
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		nmhd_del(a);
		return;
	case SampleTableAtomType: stbl_del(a); return;
	case DataInformationAtomType: dinf_del(a); return;
	case DataEntryURLAtomType: url_del(a); return;
	case DataEntryURNAtomType: urn_del(a); return;
	case CopyrightAtomType: cprt_del(a); return;
	case HandlerAtomType: hdlr_del(a); return;
	case ObjectDescriptorAtomType: iods_del(a); return;
	case TrackAtomType: trak_del(a); return;
	case MPEGSampleEntryAtomType: mp4s_del(a); return;
	case MPEGVisualSampleEntryAtomType: mp4v_del(a); return;
	case MPEGAudioSampleEntryAtomType: mp4a_del(a); return;
	case GenericMediaSampleEntryAtomType: gnrm_del(a); return;
	case GenericVisualSampleEntryAtomType: gnrv_del(a); return;
	case GenericAudioSampleEntryAtomType: gnra_del(a); return;
	case EditAtomType: edts_del(a); return;
	case UserDataAtomType: udta_del(a); return;
	case DataReferenceAtomType: dref_del(a); return;
	case SampleDescriptionAtomType: stsd_del(a); return;
	case TimeToSampleAtomType: stts_del(a); return;
	case CompositionOffsetAtomType: ctts_del(a); return;
	case ShadowSyncAtomType: stsh_del(a); return;
	case EditListAtomType: elst_del(a); return;
	case SampleToChunkAtomType: stsc_del(a); return;
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		stsz_del(a);
		return;
	case ChunkOffsetAtomType: stco_del(a); return;
	case SyncSampleAtomType: stss_del(a); return;
	case DegradationPriorityAtomType: stdp_del(a); return;
	case ChunkLargeOffsetAtomType: co64_del(a); return;
	case ESDAtomType: esds_del(a); return;
	case MediaInformationAtomType: minf_del(a); return;
	case TrackHeaderAtomType: tkhd_del(a); return;
	case TrackReferenceAtomType: tref_del(a); return;
	case MediaAtomType: mdia_del(a); return;

	//Hinting
	case RTPHintSampleEntryAtomType: ghnt_del(a); return;
	case rtpoAtomType: rtpo_del(a); return;
	case HintTrackInfoAtomType: hnti_del(a); return;
	case SDPAtomType: sdp_del(a); return;
	case HintInfoAtomType: hinf_del(a); return;
	case relyHintEntryType: rely_del(a); return;
	case timsHintEntryType: tims_del(a); return;
	case tsroHintEntryType: tsro_del(a); return;
	case snroHintEntryType: snro_del(a); return;
	case trpyAtomType: trpy_del(a); return;
	case numpAtomType: nump_del(a); return;
	case totlAtomType: totl_del(a); return;
	case npckAtomType: npck_del(a); return;
	case tpylAtomType: tpyl_del(a); return;
	case tpayAtomType: tpay_del(a); return;
	case maxrAtomType: maxr_del(a); return;
	case dmedAtomType: dmed_del(a); return;
	case dimmAtomType: dimm_del(a); return;
	case drepAtomType: drep_del(a); return;
	case tminAtomType: tmin_del(a); return;
	case tmaxAtomType: tmax_del(a); return;
	case pmaxAtomType: pmax_del(a); return;
	case dmaxAtomType: dmax_del(a); return;
	case paytAtomType: payt_del(a); return;
	case nameAtomType: name_del(a); return;

	case FileTypeAtomType: ftyp_del(a); return;
	case PaddingBitsAtomType: padb_del(a); return;

#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: mvex_del(a); return;
	case TrackExtendsAtomType: trex_del(a); return;
	case MovieFragmentAtomType: moof_del(a); return;
	case MovieFragmentHeaderAtomType: mfhd_del(a); return;
	case TrackFragmentAtomType: traf_del(a); return;
	case TrackFragmentHeaderAtomType: tfhd_del(a); return;
	case TrackFragmentRunAtomType: trun_del(a); return;
#endif

	case VoidAtomType: void_del(a); return;
	case SampleFragmentAtomType: stsf_del(a); return;

	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		amr3_del(a);
		return;
	case AMRConfigAtomType: damr_del(a); return;
	case H263SampleEntryAtomType: h263_del(a); return;
	case H263ConfigAtomType: d263_del(a); return;

	/*AVC atoms*/
	case AVCConfigurationAtomType: avcc_del(a); return;
	case MPEG4BitRateAtomType: btrt_del(a); return;
	case MPEG4ExtensionDescriptorsAtomType: m4ds_del(a); return;
	case AVCSampleEntryAtomType: avc1_del(a); return;

	/*3GPP streaming text*/
	case FontTableAtomType: ftab_del(a); return;
	case TextSampleEntryAtomType: tx3g_del(a); return;
	case TextStyleAtomType: styl_del(a); return;
	case TextHighlightAtomType: hlit_del(a); return;
	case TextHighlightColorAtomType: hclr_del(a); return;
	case TextKaraokeAtomType: krok_del(a); return;
	case TextScrollDelayAtomType: dlay_del(a); return;
	case TextHyperTextAtomType: href_del(a); return;
	case TextBoxAtomType: tbox_del(a); return;
	case TextBlinkAtomType: blnk_del(a); return;
	case TextWrapAtomType: twrp_del(a); return;

	default:
		//((UnknownAtom *)a)->data = NULL;
		defa_del(a);
		return;
	}
}




M4Err ReadAtom(Atom *a, BitStream *bs, u64 *read)
{
	switch (a->type) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		return reftype_Read(a, bs, read);
	case FreeSpaceAtomType:
	case SkipAtomType:
		return free_Read(a, bs, read);
	case MediaDataAtomType: return mdat_Read(a, bs, read);
	case MovieAtomType: return moov_Read(a, bs, read);
	case MovieHeaderAtomType: return mvhd_Read(a, bs, read);
	case MediaHeaderAtomType: return mdhd_Read(a, bs, read);
	case VideoMediaHeaderAtomType: return vmhd_Read(a, bs, read);
	case SoundMediaHeaderAtomType: return smhd_Read(a, bs, read);
	case HintMediaHeaderAtomType: return hmhd_Read(a, bs, read);
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		return nmhd_Read(a, bs, read);
	case SampleTableAtomType: return stbl_Read(a, bs, read);
	case DataInformationAtomType: return dinf_Read(a, bs, read);
	case DataEntryURLAtomType: return url_Read(a, bs, read);
	case DataEntryURNAtomType: return urn_Read(a, bs, read);
	case CopyrightAtomType: return cprt_Read(a, bs, read);
	case HandlerAtomType: return hdlr_Read(a, bs, read);
	case ObjectDescriptorAtomType: return iods_Read(a, bs, read);
	case TrackAtomType: return trak_Read(a, bs, read);
	case MPEGSampleEntryAtomType: return mp4s_Read(a, bs, read);
	case MPEGVisualSampleEntryAtomType: return mp4v_Read(a, bs, read);
	case MPEGAudioSampleEntryAtomType: return mp4a_Read(a, bs, read);
	case EditAtomType: return edts_Read(a, bs, read);
	case UserDataAtomType: return udta_Read(a, bs, read);
	case DataReferenceAtomType: return dref_Read(a, bs, read);
	case SampleDescriptionAtomType: return stsd_Read(a, bs, read);
	case TimeToSampleAtomType: return stts_Read(a, bs, read);
	case CompositionOffsetAtomType: return ctts_Read(a, bs, read);
	case ShadowSyncAtomType: return stsh_Read(a, bs, read);
	case EditListAtomType: return elst_Read(a, bs, read);
	case SampleToChunkAtomType: return stsc_Read(a, bs, read);
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		return stsz_Read(a, bs, read);
	case ChunkOffsetAtomType: return stco_Read(a, bs, read);
	case SyncSampleAtomType: return stss_Read(a, bs, read);
	case DegradationPriorityAtomType: return stdp_Read(a, bs, read);
	case ChunkLargeOffsetAtomType: return co64_Read(a, bs, read);
	case ESDAtomType: return esds_Read(a, bs, read);
	case MediaInformationAtomType: return minf_Read(a, bs, read);
	case TrackHeaderAtomType: return tkhd_Read(a, bs, read);
	case TrackReferenceAtomType: return tref_Read(a, bs, read);
	case MediaAtomType: return mdia_Read(a, bs, read);
	
	//Hinting
	case RTPHintSampleEntryAtomType: return ghnt_Read(a, bs, read);
	case rtpoAtomType: return rtpo_Read(a, bs, read);
	case HintTrackInfoAtomType: return hnti_Read(a, bs, read);
	case SDPAtomType: return sdp_Read(a, bs, read);
	case HintInfoAtomType: return hinf_Read(a, bs, read);
	case relyHintEntryType: return rely_Read(a, bs, read);
	case timsHintEntryType: return tims_Read(a, bs, read);
	case tsroHintEntryType: return tsro_Read(a, bs, read);
	case snroHintEntryType: return snro_Read(a, bs, read);
	case trpyAtomType: return trpy_Read(a, bs, read);
	case numpAtomType: return nump_Read(a, bs, read);
	case totlAtomType: return totl_Read(a, bs, read);
	case npckAtomType: return npck_Read(a, bs, read);
	case tpylAtomType: return tpyl_Read(a, bs, read);
	case tpayAtomType: return tpay_Read(a, bs, read);
	case maxrAtomType: return maxr_Read(a, bs, read);
	case dmedAtomType: return dmed_Read(a, bs, read);
	case dimmAtomType: return dimm_Read(a, bs, read);
	case drepAtomType: return drep_Read(a, bs, read);
	case tminAtomType: return tmin_Read(a, bs, read);
	case tmaxAtomType: return tmax_Read(a, bs, read);
	case pmaxAtomType: return pmax_Read(a, bs, read);
	case dmaxAtomType: return dmax_Read(a, bs, read);
	case paytAtomType: return payt_Read(a, bs, read);
	case nameAtomType: return name_Read(a, bs, read);
	
	//V2 Atoms
	case FileTypeAtomType: return ftyp_Read(a, bs, read);
	case PaddingBitsAtomType: return padb_Read(a, bs, read);

#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: return mvex_Read(a, bs, read);
	case TrackExtendsAtomType: return trex_Read(a, bs, read);
	case MovieFragmentAtomType: return moof_Read(a, bs, read);
	case MovieFragmentHeaderAtomType: return mfhd_Read(a, bs, read);
	case TrackFragmentAtomType: return traf_Read(a, bs, read);
	case TrackFragmentHeaderAtomType: return tfhd_Read(a, bs, read);
	case TrackFragmentRunAtomType: return trun_Read(a, bs, read);
#endif
	
	case VoidAtomType: return void_Read(a, bs, read);
	case SampleFragmentAtomType: return stsf_Read(a, bs, read);

	//3GPP atoms
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		return amr3_Read(a, bs, read);
	case AMRConfigAtomType: return damr_Read(a, bs, read);
	case H263SampleEntryAtomType: return h263_Read(a, bs, read);
	case H263ConfigAtomType: return d263_Read(a, bs, read);

	case AVCConfigurationAtomType: return avcc_Read(a, bs, read);
	case MPEG4BitRateAtomType: return btrt_Read(a, bs, read);
	case MPEG4ExtensionDescriptorsAtomType: return m4ds_Read(a, bs, read);
	case AVCSampleEntryAtomType: return avc1_Read(a, bs, read);

	/*3GPP streaming text*/
	case FontTableAtomType: return ftab_Read(a, bs, read);
	case TextSampleEntryAtomType: return tx3g_Read(a, bs, read);
	case TextStyleAtomType: return styl_Read(a, bs, read);
	case TextHighlightAtomType: return hlit_Read(a, bs, read);
	case TextHighlightColorAtomType: return hclr_Read(a, bs, read);
	case TextKaraokeAtomType: return krok_Read(a, bs, read);
	case TextScrollDelayAtomType: return dlay_Read(a, bs, read);
	case TextHyperTextAtomType: return href_Read(a, bs, read);
	case TextBoxAtomType: return tbox_Read(a, bs, read);
	case TextBlinkAtomType: return blnk_Read(a, bs, read);
	case TextWrapAtomType: return twrp_Read(a, bs, read);

	default:
		return defa_Read(a, bs, read);
	}
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err WriteAtom(Atom *a, BitStream *bs)
{
	switch (a->type) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		return reftype_Write(a, bs);
	case FreeSpaceAtomType:
	case SkipAtomType:
		return free_Write(a, bs);
	case MediaDataAtomType: return mdat_Write(a, bs);
	case MovieAtomType: return moov_Write(a, bs);
	case MovieHeaderAtomType: return mvhd_Write(a, bs);
	case MediaHeaderAtomType: return mdhd_Write(a, bs);
	case VideoMediaHeaderAtomType: return vmhd_Write(a, bs);
	case SoundMediaHeaderAtomType: return smhd_Write(a, bs);
	case HintMediaHeaderAtomType: return hmhd_Write(a, bs);
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		return nmhd_Write(a, bs);
	case SampleTableAtomType: return stbl_Write(a, bs);
	case DataInformationAtomType: return dinf_Write(a, bs);
	case DataEntryURLAtomType: return url_Write(a, bs);
	case DataEntryURNAtomType: return urn_Write(a, bs);
	case CopyrightAtomType: return cprt_Write(a, bs);
	case HandlerAtomType: return hdlr_Write(a, bs);
	case ObjectDescriptorAtomType: return iods_Write(a, bs);
	case TrackAtomType: return trak_Write(a, bs);
	case MPEGSampleEntryAtomType: return mp4s_Write(a, bs);
	case MPEGVisualSampleEntryAtomType: return mp4v_Write(a, bs);
	case MPEGAudioSampleEntryAtomType: return mp4a_Write(a, bs);
	case GenericMediaSampleEntryAtomType: return gnrm_Write(a, bs);
	case GenericVisualSampleEntryAtomType: return gnrv_Write(a, bs);
	case GenericAudioSampleEntryAtomType: return gnra_Write(a, bs);
	case EditAtomType: return edts_Write(a, bs);
	case UserDataAtomType: return udta_Write(a, bs);
	case DataReferenceAtomType: return dref_Write(a, bs);
	case SampleDescriptionAtomType: return stsd_Write(a, bs);
	case TimeToSampleAtomType: return stts_Write(a, bs);
	case CompositionOffsetAtomType: return ctts_Write(a, bs);
	case ShadowSyncAtomType: return stsh_Write(a, bs);
	case EditListAtomType: return elst_Write(a, bs);
	case SampleToChunkAtomType: return stsc_Write(a, bs);
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		return stsz_Write(a, bs);
	case ChunkOffsetAtomType: return stco_Write(a, bs);
	case SyncSampleAtomType: return stss_Write(a, bs);
	case DegradationPriorityAtomType: return stdp_Write(a, bs);
	case ChunkLargeOffsetAtomType: return co64_Write(a, bs);
	case ESDAtomType: return esds_Write(a, bs);
	case MediaInformationAtomType: return minf_Write(a, bs);
	case TrackHeaderAtomType: return tkhd_Write(a, bs);
	case TrackReferenceAtomType: return tref_Write(a, bs);
	case MediaAtomType: return mdia_Write(a, bs);

	//Hinting
	case RTPHintSampleEntryAtomType: return ghnt_Write(a, bs);
	case rtpoAtomType: return rtpo_Write(a, bs);
	case HintTrackInfoAtomType: return hnti_Write(a, bs);
	case SDPAtomType: return sdp_Write(a, bs);
	case HintInfoAtomType: return hinf_Write(a, bs);
	case relyHintEntryType: return rely_Write(a, bs);
	case timsHintEntryType: return tims_Write(a, bs);
	case tsroHintEntryType: return tsro_Write(a, bs);
	case snroHintEntryType: return snro_Write(a, bs);
	case trpyAtomType: return trpy_Write(a, bs);
	case numpAtomType: return nump_Write(a, bs);
	case totlAtomType: return totl_Write(a, bs);
	case npckAtomType: return npck_Write(a, bs);
	case tpylAtomType: return tpyl_Write(a, bs);
	case tpayAtomType: return tpay_Write(a, bs);
	case maxrAtomType: return maxr_Write(a, bs);
	case dmedAtomType: return dmed_Write(a, bs);
	case dimmAtomType: return dimm_Write(a, bs);
	case drepAtomType: return drep_Write(a, bs);
	case tminAtomType: return tmin_Write(a, bs);
	case tmaxAtomType: return tmax_Write(a, bs);
	case pmaxAtomType: return pmax_Write(a, bs);
	case dmaxAtomType: return dmax_Write(a, bs);
	case paytAtomType: return payt_Write(a, bs);
	case nameAtomType: return name_Write(a, bs);
	
	case FileTypeAtomType: return ftyp_Write(a, bs);
	case PaddingBitsAtomType: return padb_Write(a, bs);

#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: return mvex_Write(a, bs);
	case TrackExtendsAtomType: return trex_Write(a, bs);
	case MovieFragmentAtomType: return moof_Write(a, bs);
	case MovieFragmentHeaderAtomType: return mfhd_Write(a, bs);
	case TrackFragmentAtomType: return traf_Write(a, bs);
	case TrackFragmentHeaderAtomType: return tfhd_Write(a, bs);
	case TrackFragmentRunAtomType: return trun_Write(a, bs);
#endif 

	case VoidAtomType: return void_Write(a, bs);
	case SampleFragmentAtomType: return stsf_Write(a, bs);

	//3GPP atoms
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		return amr3_Write(a, bs);
	case AMRConfigAtomType: return damr_Write(a, bs);
	case H263SampleEntryAtomType: return h263_Write(a, bs);
	case H263ConfigAtomType: return d263_Write(a, bs);

	case AVCConfigurationAtomType: return avcc_Write(a, bs);
	case MPEG4BitRateAtomType: return btrt_Write(a, bs);
	case MPEG4ExtensionDescriptorsAtomType: return m4ds_Write(a, bs);
	case AVCSampleEntryAtomType: return avc1_Write(a, bs);

	/*3GPP streaming text*/
	case FontTableAtomType: return ftab_Write(a, bs);
	case TextSampleEntryAtomType: return tx3g_Write(a, bs);
	case TextStyleAtomType: return styl_Write(a, bs);
	case TextHighlightAtomType: return hlit_Write(a, bs);
	case TextHighlightColorAtomType: return hclr_Write(a, bs);
	case TextKaraokeAtomType: return krok_Write(a, bs);
	case TextScrollDelayAtomType: return dlay_Write(a, bs);
	case TextHyperTextAtomType: return href_Write(a, bs);
	case TextBoxAtomType: return tbox_Write(a, bs);
	case TextBlinkAtomType: return blnk_Write(a, bs);
	case TextWrapAtomType: return twrp_Write(a, bs);
	
	default:
		return defa_Write(a, bs);
	}
}


M4Err SizeAtom(Atom *a)
{
	switch (a->type) {
	case HintTrackReferenceAtomType:
	case StreamDependenceAtomType:
	case ODTrackReferenceAtomType:
	case OCRReferenceAtomType:
	case IPIReferenceAtomType:
		return reftype_Size(a);
	case FreeSpaceAtomType:
	case SkipAtomType:
		return free_Size(a);
	case MediaDataAtomType: return mdat_Size(a);
	case MovieAtomType: return moov_Size(a);
	case MovieHeaderAtomType: return mvhd_Size(a);
	case MediaHeaderAtomType: return mdhd_Size(a);
	case VideoMediaHeaderAtomType: return vmhd_Size(a);
	case SoundMediaHeaderAtomType: return smhd_Size(a);
	case HintMediaHeaderAtomType: return hmhd_Size(a);
	//the same atom is used for all MPEG4 systems streams
	case ObjectDescriptorMediaHeaderAtomType:
	case ClockReferenceMediaHeaderAtomType:
	case SceneDescriptionMediaHeaderAtomType:
	case MPEGMediaHeaderAtomType:
		return nmhd_Size(a);
	case SampleTableAtomType: return stbl_Size(a);
	case DataInformationAtomType: return dinf_Size(a);
	case DataEntryURLAtomType: return url_Size(a);
	case DataEntryURNAtomType: return urn_Size(a);
	case CopyrightAtomType: return cprt_Size(a);
	case HandlerAtomType: return hdlr_Size(a);
	case ObjectDescriptorAtomType: return iods_Size(a);
	case TrackAtomType: return trak_Size(a);
	case MPEGSampleEntryAtomType: return mp4s_Size(a);
	case MPEGVisualSampleEntryAtomType: return mp4v_Size(a);
	case MPEGAudioSampleEntryAtomType: return mp4a_Size(a);
	case GenericMediaSampleEntryAtomType: return gnrm_Size(a);
	case GenericVisualSampleEntryAtomType: return gnrv_Size(a);
	case GenericAudioSampleEntryAtomType: return gnra_Size(a);
	case EditAtomType: return edts_Size(a);
	case UserDataAtomType: return udta_Size(a);
	case DataReferenceAtomType: return dref_Size(a);
	case SampleDescriptionAtomType: return stsd_Size(a);
	case TimeToSampleAtomType: return stts_Size(a);
	case CompositionOffsetAtomType: return ctts_Size(a);
	case ShadowSyncAtomType: return stsh_Size(a);
	case EditListAtomType: return elst_Size(a);
	case SampleToChunkAtomType: return stsc_Size(a);
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		return stsz_Size(a);
	case ChunkOffsetAtomType: return stco_Size(a);
	case SyncSampleAtomType: return stss_Size(a);
	case DegradationPriorityAtomType: return stdp_Size(a);
	case ChunkLargeOffsetAtomType: return co64_Size(a);
	case ESDAtomType: return esds_Size(a);
	case MediaInformationAtomType: return minf_Size(a);
	case TrackHeaderAtomType: return tkhd_Size(a);
	case TrackReferenceAtomType: return tref_Size(a);
	case MediaAtomType: return mdia_Size(a);
	
	//Hinting
	case RTPHintSampleEntryAtomType: return ghnt_Size(a);
	case rtpoAtomType: return rtpo_Size(a);
	case HintTrackInfoAtomType: return hnti_Size(a);
	case SDPAtomType: return sdp_Size(a);
	case HintInfoAtomType: return hinf_Size(a);
	case relyHintEntryType: return rely_Size(a);
	case timsHintEntryType: return tims_Size(a);
	case tsroHintEntryType: return tsro_Size(a);
	case snroHintEntryType: return snro_Size(a);
	case trpyAtomType: return trpy_Size(a);
	case numpAtomType: return nump_Size(a);
	case totlAtomType: return totl_Size(a);
	case npckAtomType: return npck_Size(a);
	case tpylAtomType: return tpyl_Size(a);
	case tpayAtomType: return tpay_Size(a);
	case maxrAtomType: return maxr_Size(a);
	case dmedAtomType: return dmed_Size(a);
	case dimmAtomType: return dimm_Size(a);
	case drepAtomType: return drep_Size(a);
	case tminAtomType: return tmin_Size(a);
	case tmaxAtomType: return tmax_Size(a);
	case pmaxAtomType: return pmax_Size(a);
	case dmaxAtomType: return dmax_Size(a);
	case paytAtomType: return payt_Size(a);
	case nameAtomType: return name_Size(a);
	
	//V2 Atoms
	case FileTypeAtomType: return ftyp_Size(a);
	case PaddingBitsAtomType: return padb_Size(a);
	
#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType: return mvex_Size(a);
	case TrackExtendsAtomType: return trex_Size(a);
	case MovieFragmentAtomType: return moof_Size(a);
	case MovieFragmentHeaderAtomType: return mfhd_Size(a);
	case TrackFragmentAtomType: return traf_Size(a);
	case TrackFragmentHeaderAtomType: return tfhd_Size(a);
	case TrackFragmentRunAtomType: return trun_Size(a);
#endif
	
	case VoidAtomType: return void_Size(a);
	case SampleFragmentAtomType: return stsf_Size(a);

	//3GPP atoms
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		return amr3_Size(a);
	case AMRConfigAtomType: return damr_Size(a);
	case H263SampleEntryAtomType: return h263_Size(a);
	case H263ConfigAtomType: return d263_Size(a);

	case AVCConfigurationAtomType: return avcc_Size(a);
	case MPEG4BitRateAtomType: return btrt_Size(a);
	case MPEG4ExtensionDescriptorsAtomType: return m4ds_Size(a);
	case AVCSampleEntryAtomType: return avc1_Size(a);

	/*3GPP streaming text*/
	case FontTableAtomType: return ftab_Size(a);
	case TextSampleEntryAtomType: return tx3g_Size(a);
	case TextStyleAtomType: return styl_Size(a);
	case TextHighlightAtomType: return hlit_Size(a);
	case TextHighlightColorAtomType: return hclr_Size(a);
	case TextKaraokeAtomType: return krok_Size(a);
	case TextScrollDelayAtomType: return dlay_Size(a);
	case TextHyperTextAtomType: return href_Size(a);
	case TextBoxAtomType: return tbox_Size(a);
	case TextBlinkAtomType: return blnk_Size(a);
	case TextWrapAtomType: return twrp_Size(a);

	default: return defa_Size(a);
	}
}

#endif //M4_READ_ONLY
