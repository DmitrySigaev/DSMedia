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


#ifndef M4_READ_ONLY

M4Err M4_TextDescriptionAddFont(M4File *the_file, u32 trackNumber, u32 descriptionIndex, u16 fontID, char *fontName)
{
	TrackAtom *trak;
	M4Err e;
	TextSampleEntryAtom *txt;

	if (!fontName || !descriptionIndex || !fontID) return M4BadParam;
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media) return M4BadParam;

	if (trak->Media->handler->handlerType != M4_TimedTextMediaType) return M4BadParam;
	txt = (TextSampleEntryAtom*)ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, descriptionIndex - 1);
	if (!txt) return M4BadParam;
	if (txt->type != TextSampleEntryAtomType) return M4BadParam;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	assert(txt->font_table);
	assert(txt->font_table->entry_count);

	txt->font_table->fonts = realloc(txt->font_table->fonts, sizeof(FontRecord) * (txt->font_table->entry_count+1) );
	if (!txt->font_table->fonts) return M4OutOfMem;
	txt->font_table->fonts[txt->font_table->entry_count].fontID = fontID;
	txt->font_table->fonts[txt->font_table->entry_count].fontName = strdup(fontName);
	txt->font_table->entry_count++;
	return M4OK;
}

M4Err M4_UpdateTextDescription(M4File *the_file, u32 trackNumber, u32 descriptionIndex, TextSampleDescriptor *desc)
{
	TrackAtom *trak;
	M4Err e;
	u32 i;
	TextSampleEntryAtom *txt;

	if (!descriptionIndex || !desc) return M4BadParam;
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !desc->font_count) return M4BadParam;

	if (trak->Media->handler->handlerType != M4_TimedTextMediaType) return M4BadParam;

	txt = (TextSampleEntryAtom*)ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, descriptionIndex - 1);
	if (!txt) return M4BadParam;
	if (txt->type != TextSampleEntryAtomType) return M4BadParam;

	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	txt->back_color = desc->back_color; 
	txt->default_box = desc->default_pos;
	txt->default_style = desc->default_style;
	txt->displayFlags = desc->displayFlags;
	txt->vertical_justification = desc->vert_justif;
	txt->horizontal_justification = desc->horiz_justif;
	if (txt->font_table) DelAtom((Atom*)txt->font_table);

	txt->font_table = (FontTableAtom *)CreateAtom(FontTableAtomType);
	txt->font_table->entry_count = desc->font_count;
	SAFEALLOC(txt->font_table->fonts, sizeof(FontRecord) * desc->font_count);
	for (i=0; i<desc->font_count; i++) {
		txt->font_table->fonts[i].fontID = desc->fonts[i].fontID;
		if (desc->fonts[i].fontName) txt->font_table->fonts[i].fontName = strdup(desc->fonts[i].fontName);
	}
	return e;
}

M4Err M4_NewTextDescription(M4File *the_file, u32 trackNumber, TextSampleDescriptor *desc, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	u32 dataRefIndex, i;
	TextSampleEntryAtom *txt;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !desc->font_count) return M4BadParam;

	if (trak->Media->handler->handlerType != M4_TimedTextMediaType) return M4BadParam;

	//get or create the data ref
	e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
	if (e) return e;
	if (!dataRefIndex) {
		e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
		if (e) return e;
	}
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	txt = (TextSampleEntryAtom *) CreateAtom(TextSampleEntryAtomType);
	txt->dataReferenceIndex = dataRefIndex;
	ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, txt);
	if (outDescriptionIndex) *outDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);

	txt->back_color = desc->back_color; 
	txt->default_box = desc->default_pos;
	txt->default_style = desc->default_style;
	txt->displayFlags = desc->displayFlags;
	txt->vertical_justification = desc->vert_justif;
	txt->horizontal_justification = desc->horiz_justif;
	txt->font_table = (FontTableAtom *)CreateAtom(FontTableAtomType);
	txt->font_table->entry_count = desc->font_count;

	SAFEALLOC(txt->font_table->fonts, sizeof(FontRecord) * desc->font_count);
	for (i=0; i<desc->font_count; i++) {
		txt->font_table->fonts[i].fontID = desc->fonts[i].fontID;
		if (desc->fonts[i].fontName) txt->font_table->fonts[i].fontName = strdup(desc->fonts[i].fontName);
	}
	return e;
}


/*blindly adds text - note we don't rely on terminaison characters to handle utf8 and utf16 data 
in the same way. It is the user responsability to signal UTF16*/
M4Err M4_TxtAddText(TextSample *samp, char *text_data, u32 text_len)
{
	if (!samp) return M4BadParam;
	if (!text_len) return M4OK;
	if (!samp->text) {
		samp->text = malloc(sizeof(char) * text_len);
		memcpy(samp->text, text_data, sizeof(char) * text_len);
		samp->len = text_len;
	} else {
		samp->text = realloc(samp->text, sizeof(char) * (samp->len + text_len) );
		memcpy(samp->text + samp->len, text_data, sizeof(char) * text_len);
		samp->len += text_len;
	}
	return M4OK;
}

M4Err M4_TxtSetUTF16Marker(TextSample *samp)
{
	/*we MUST have an empty sample*/
	if (!samp || samp->text) return M4BadParam;
	samp->text = malloc(sizeof(char) * 2);
	samp->text[0] = (char) 0xFE;
	samp->text[1] = (char) 0xFF;
	samp->len = 2;
	return M4OK;
}

M4Err M4_TxtAddStyle(TextSample *samp, StyleRecord *rec)
{
	if (!samp || !rec) return M4BadParam;

	if (!samp->styles) {
		samp->styles = (TextStyleAtom *) CreateAtom(TextStyleAtomType);
		if (!samp->styles) return M4OutOfMem;
	}
	if (!samp->styles->entry_count) {
		samp->styles->entry_count = 1;
		SAFEALLOC(samp->styles->styles, sizeof(StyleRecord));
		if (!samp->styles->styles) return M4OutOfMem;
		samp->styles->styles[0] = *rec;
	} else {
		samp->styles->styles = realloc(samp->styles->styles, sizeof(StyleRecord)*(samp->styles->entry_count+1));
		if (!samp->styles->styles) return M4OutOfMem;
		samp->styles->styles[samp->styles->entry_count] = *rec;
		samp->styles->entry_count++;
	}
	return M4OK;
}

M4Err M4_TxtAddHighlight(TextSample *samp, u16 start_char, u16 end_char)
{
	TextHighlightAtom *a;
	if (!samp) return M4BadParam;
	if (start_char == end_char) return M4BadParam;

	a = (TextHighlightAtom *) CreateAtom(TextHighlightAtomType);
	if (!a) return M4OutOfMem;
	a->startcharoffset = start_char;
	a->endcharoffset = end_char;
	return ChainAddEntry(samp->others, a);
}

M4Err M4_TxtSetHighlightColor(TextSample *samp, u8 r, u8 g, u8 b, u8 a)
{
	if (!samp) return M4BadParam;

	if (!samp->highlight_color) {
		samp->highlight_color = (TextHighlightColorAtom *) CreateAtom(TextHighlightColorAtomType);
		if (!samp->highlight_color) return M4OutOfMem;
	}
	samp->highlight_color->hil_color = a; samp->highlight_color->hil_color <<= 8;
	samp->highlight_color->hil_color = r; samp->highlight_color->hil_color <<= 8;
	samp->highlight_color->hil_color = g; samp->highlight_color->hil_color <<= 8;
	samp->highlight_color->hil_color = b;
	return M4OK;
}

M4Err M4_TxtSetHighlightColorARGB(TextSample *samp, u32 argb)
{
	if (!samp) return M4BadParam;

	if (!samp->highlight_color) {
		samp->highlight_color = (TextHighlightColorAtom *) CreateAtom(TextHighlightColorAtomType);
		if (!samp->highlight_color) return M4OutOfMem;
	}
	samp->highlight_color->hil_color = argb;
	return M4OK;
}

/*3GPP spec is quite obscur here*/
M4Err M4_TxtAddKaraoke(TextSample *samp, u32 start_time)
{
	if (!samp) return M4BadParam;
	samp->cur_karaoke = (TextKaraokeAtom *) CreateAtom(TextKaraokeAtomType);
	if (!samp->cur_karaoke) return M4OutOfMem;
	samp->cur_karaoke->highlight_starttime = start_time;
	return ChainAddEntry(samp->others, samp->cur_karaoke);
}

M4Err M4_TxtSetKaraokeSeg(TextSample *samp, u32 end_time, u16 start_char, u16 end_char)
{
	if (!samp || !samp->cur_karaoke) return M4BadParam;
	if (!samp->cur_karaoke->entrycount) {
		samp->cur_karaoke->entrycount = 1;
		SAFEALLOC(samp->cur_karaoke->records, sizeof(KaraokeRecord));
		if (!samp->cur_karaoke->records) return M4OutOfMem;
		samp->cur_karaoke->records[0].end_charoffset = end_char;
		samp->cur_karaoke->records[0].start_charoffset = start_char;
		samp->cur_karaoke->records[0].highlight_endtime = end_time;
	} else {
		samp->cur_karaoke->records = realloc(samp->cur_karaoke->records, sizeof(KaraokeRecord)*(samp->cur_karaoke->entrycount+1));
		if (!samp->cur_karaoke->records) return M4OutOfMem;
		samp->cur_karaoke->records[samp->cur_karaoke->entrycount].end_charoffset = end_char;
		samp->cur_karaoke->records[samp->cur_karaoke->entrycount].start_charoffset = start_char;
		samp->cur_karaoke->records[samp->cur_karaoke->entrycount].highlight_endtime = end_time;
		samp->cur_karaoke->entrycount++;
	}
	return M4OK;
}


M4Err M4_TxtSetScrollDelay(TextSample *samp, u32 scroll_delay)
{
	if (!samp) return M4BadParam;
	if (!samp->scroll_delay) {
		samp->scroll_delay = (TextScrollDelayAtom*) CreateAtom(TextScrollDelayAtomType);
		if (!samp->scroll_delay) return M4OutOfMem;
	}
	samp->scroll_delay->scroll_delay = scroll_delay;
	return M4OK;
}

M4Err M4_TxtAddHypertext(TextSample *samp, char *URL, char *altString, u16 start_char, u16 end_char)
{
	TextHyperTextAtom*a;
	if (!samp) return M4BadParam;
	a = (TextHyperTextAtom*) CreateAtom(TextHyperTextAtomType);
	if (!a) return M4OutOfMem;
	a->startcharoffset = start_char;
	a->endcharoffset = end_char;
	a->URL = URL ? strdup(URL) : NULL;
	a->URL_hint = altString ? strdup(altString) : NULL;
	return ChainAddEntry(samp->others, a);
}

M4Err M4_TxtSetBox(TextSample *samp, s16 top, s16 left, s16 bottom, s16 right)
{
	if (!samp) return M4BadParam;
	if (!samp->box) {
		samp->box = (TextBoxAtom*) CreateAtom(TextBoxAtomType);
		if (!samp->box) return M4OutOfMem;
	}
	samp->box->box.top = top;
	samp->box->box.left = left;
	samp->box->box.bottom = bottom;
	samp->box->box.right = right;
	return M4OK;
}

M4Err M4_TxtAddBlink(TextSample *samp, u16 start_char, u16 end_char)
{
	TextBlinkAtom *a;
	if (!samp) return M4BadParam;
	a = (TextBlinkAtom*) CreateAtom(TextBlinkAtomType);
	if (!a) return M4OutOfMem;
	a->startcharoffset = start_char;
	a->endcharoffset = end_char;
	return ChainAddEntry(samp->others, a);
}

M4Err M4_TxtSetWrap(TextSample *samp, u8 wrap_flags)
{
	if (!samp) return M4BadParam;
	if (!samp->wrap) {
		samp->wrap = (TextWrapAtom*) CreateAtom(TextWrapAtomType);
		if (!samp->wrap) return M4OutOfMem;
	}
	samp->wrap->wrap_flag = wrap_flags;
	return M4OK;
}

static M4INLINE M4Err gpp_write_modifier(BitStream *bs, Atom *a)
{
	M4Err e;
	if (!a) return M4OK;
	e = SizeAtom(a);
	if (!e) e = WriteAtom(a, bs);
	return e;
}

M4Sample *M4_TxtToSample(TextSample *samp)
{
	M4Err e;
	M4Sample *res;
	BitStream *bs;
	u32 i;
	if (!samp) return NULL;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, samp->len, 16);
	if (samp->len) BS_WriteData(bs, samp->text, samp->len);

	e = gpp_write_modifier(bs, (Atom *)samp->styles);
	if (!e) e = gpp_write_modifier(bs, (Atom *)samp->highlight_color);
	if (!e) e = gpp_write_modifier(bs, (Atom *)samp->scroll_delay);
	if (!e) e = gpp_write_modifier(bs, (Atom *)samp->box);
	if (!e) e = gpp_write_modifier(bs, (Atom *)samp->wrap);

	if (!e) {
		for (i=0; i<ChainGetCount(samp->others); i++) {
			Atom *a = ChainGetEntry(samp->others, i);
			e = gpp_write_modifier(bs, a);
			if (e) break;
		}
	}
	if (e) {
		DeleteBitStream(bs);
		return NULL;
	}
	res = M4_NewSample();
	if (!res) {
		DeleteBitStream(bs);
		return NULL;
	}
	BS_GetContent(bs, (unsigned char **) &res->data, &res->dataLength);
	DeleteBitStream(bs);
	res->IsRAP = 1;
	return res;
}

#endif

TextSample *M4_NewTextSample()
{
	TextSample *res;
	SAFEALLOC(res, sizeof(TextSample));
	if (!res) return NULL;
	res->others = NewChain();
	return res;
}

M4Err M4_TxtReset(TextSample *samp)
{
	if (!samp) return M4BadParam;
	if (samp->text) free(samp->text);
	samp->text = NULL;
	samp->len = 0;
	if (samp->box) DelAtom((Atom *)samp->box);
	samp->box = NULL;
	if (samp->highlight_color) DelAtom((Atom *)samp->highlight_color);
	samp->highlight_color = NULL;
	if (samp->scroll_delay) DelAtom((Atom *)samp->scroll_delay);
	samp->scroll_delay = NULL;
	if (samp->wrap) DelAtom((Atom *)samp->wrap);
	samp->wrap = NULL;
	if (samp->styles) DelAtom((Atom *)samp->styles);
	samp->styles = NULL;
	samp->cur_karaoke = NULL;
	while (ChainGetCount(samp->others)) {
		Atom *a = ChainGetEntry(samp->others, 0);
		ChainDeleteEntry(samp->others, 0);
		DelAtom(a);
	}
	return M4OK;
}

void M4_DeleteTextSample(M4TXTSAMPLE tx_samp)
{
	M4_TxtReset(tx_samp);
	DeleteChain(tx_samp->others);
	free(tx_samp);
}

TextSample *M4_ParseTextSample(BitStream *bs)
{
	TextSample *s = M4_NewTextSample();
	
	/*empty sample*/
	if (!bs || !BS_Available(bs)) return s;

	s->len = BS_ReadInt(bs, 16);
	if (s->len) {
		/*2 extra bytes for UTF-16 term char just in case (we don't know if a BOM marker is present or 
		not since this may be a sample carried over RTP*/
		SAFEALLOC(s->text, sizeof(char)*(s->len+2) );
		BS_ReadData(bs, s->text, s->len);
	}

	while (BS_Available(bs)) {
		Atom *a;
		u64 read;
		M4Err e = ParseAtom(&a, bs, &read);
		if (!e) {
			switch (a->type) {
			case TextStyleAtomType:
				if (s->styles) {
					TextStyleAtom *st2 = (TextStyleAtom *)a;
					if (!s->styles->entry_count) {
						DelAtom((Atom*)s->styles);
						s->styles = st2;
					} else {
						s->styles->styles = realloc(s->styles->styles, sizeof(StyleRecord) * (s->styles->entry_count + st2->entry_count));
						memcpy(&s->styles->styles[s->styles->entry_count], st2->styles, sizeof(StyleRecord) * st2->entry_count);
						s->styles->entry_count += st2->entry_count;
						DelAtom(a);
					}
				} else {
					s->styles = (TextStyleAtom*)a;
				}
				break;
			case TextKaraokeAtomType:
				s->cur_karaoke = (TextKaraokeAtom*)a;
			case TextHighlightAtomType:
			case TextHyperTextAtomType:
			case TextBlinkAtomType:
				ChainAddEntry(s->others, a);
				break;
			case TextHighlightColorAtomType:
				if (s->highlight_color) DelAtom(a);
				else s->highlight_color = (TextHighlightColorAtom *) a;
				break;
			case TextScrollDelayAtomType:
				if (s->scroll_delay) DelAtom(a);
				else s->scroll_delay= (TextScrollDelayAtom*) a;
				break;
			case TextBoxAtomType:
				if (s->box) DelAtom(a);
				else s->box= (TextBoxAtom *) a;
				break;
			case TextWrapAtomType:
				if (s->wrap) DelAtom(a);
				else s->wrap= (TextWrapAtom*) a;
				break;
			default:
				DelAtom(a);
				break;
			}
		}
	}
	return s;
}

TextSample *M4_ParseTextSampleFromData(char *data, u32 dataLength)
{
	TextSample *s;
	BitStream *bs;
	/*empty text sample*/
	if (!data || !dataLength) {
		return M4_NewTextSample();
	}
	
	bs = NewBitStream(data, dataLength, BS_READ);
	s = M4_ParseTextSample(bs);
	DeleteBitStream(bs);
	return s;
}


/*out-of-band sample desc (128 and 255 reserved in RFC)*/
#define SAMPLE_INDEX_OFFSET		129


static void M4_WriteTX3G(TextSampleEntryAtom *a, BitStream *bs, u32 sidx, u32 sidx_offset)
{
	u32 size, j;
	void gpp_write_rgba(BitStream *bs, u32 col);
	void gpp_write_box(BitStream *bs, BoxRecord *rec);
	void gpp_write_style(BitStream *bs, StyleRecord *rec);

	
	if (sidx_offset) BS_WriteInt(bs, sidx + sidx_offset, 8);

	/*SINCE WINCE HAS A READONLY VERSION OF MP4 WE MUST DO IT BY HAND*/
	size = 8 + 18 + 8 + 12;
	size += 8 + 2;
	for (j=0; j<a->font_table->entry_count; j++) {
		size += 3;
		if (a->font_table->fonts[j].fontName) size += strlen(a->font_table->fonts[j].fontName);
	}
	/*write TextSampleEntry atom*/
	BS_WriteInt(bs, size, 32);
	BS_WriteInt(bs, TextSampleEntryAtomType, 32);
	BS_WriteData(bs, (unsigned char*)a->reserved, 6);
	BS_WriteInt(bs, a->dataReferenceIndex, 16);
	BS_WriteInt(bs, a->displayFlags, 32);
	BS_WriteInt(bs, a->horizontal_justification, 8);
	BS_WriteInt(bs, a->vertical_justification, 8);
	gpp_write_rgba(bs, a->back_color);
	gpp_write_box(bs, &a->default_box);
	gpp_write_style(bs, &a->default_style);
	/*write font table atom*/
	size -= (8 + 18 + 8 + 12);
	BS_WriteInt(bs, size, 32);
	BS_WriteInt(bs, FontTableAtomType, 32);
	BS_WriteInt(bs, a->font_table->entry_count, 16);
	for (j=0; j<a->font_table->entry_count; j++) {
		BS_WriteInt(bs, a->font_table->fonts[j].fontID, 16);
		if (a->font_table->fonts[j].fontName) {
			u32 len = strlen(a->font_table->fonts[j].fontName);
			BS_WriteInt(bs, len, 8);
			BS_WriteData(bs, a->font_table->fonts[j].fontName, len);
		} else {
			BS_WriteInt(bs, 0, 8);
		}
	}
}

M4Err M4_GetStreamingTextESD(MediaAtom *mdia, ESDescriptor **out_esd)
{
	BitStream *bs;
	u32 count, i;
	Bool has_v_info;
	Chain *sampleDesc;
	ESDescriptor *esd;



	*out_esd = NULL;
	sampleDesc = mdia->information->sampleTable->SampleDescription->atomList;
	count = ChainGetCount(sampleDesc);
	if (!count) return M4InvalidMP4Media;
	
	esd = OD_NewESDescriptor(2);
	esd->decoderConfig->streamType = M4ST_TEXT;
	esd->decoderConfig->objectTypeIndication = 0x08;

	bs = NewBitStream(NULL, 0, BS_WRITE);


	/*Base3GPPFormat*/
	BS_WriteInt(bs, 0x10, 8);
	/*MPEGExtendedFormat*/
	BS_WriteInt(bs, 0x10, 8);
	/*profileLevel*/
	BS_WriteInt(bs, 0x10, 8);
	BS_WriteInt(bs, mdia->mediaHeader->timeScale, 24);
	BS_WriteInt(bs, 0, 1);	/*no alt formats*/
	BS_WriteInt(bs, 2, 2);	/*only out-of-band-band sample desc*/
	BS_WriteInt(bs, 1, 1);	/*we will write sample desc*/

	/*write v info if any visual track in this movie*/
	has_v_info = 0;
	for (i=0; i<ChainGetCount(mdia->mediaTrack->moov->trackList); i++) {
		TrackAtom *tk = ChainGetEntry(mdia->mediaTrack->moov->trackList, i);
		if (tk->Media->handler && (tk->Media->handler->handlerType == M4_VisualMediaType)) {
			has_v_info = 1;
		}
	}
	BS_WriteInt(bs, has_v_info, 1);

	BS_WriteInt(bs, 0, 3);	/*reserved, spec doesn't say the values*/
	BS_WriteInt(bs, mdia->mediaTrack->Header->layer, 8);
	BS_WriteInt(bs, mdia->mediaTrack->Header->width>>16, 16);
	BS_WriteInt(bs, mdia->mediaTrack->Header->height>>16, 16);

	/*write desc*/
	BS_WriteInt(bs, count, 8);
	for (i=0; i<count; i++) {
		TextSampleEntryAtom *a;
		a = (TextSampleEntryAtom *) ChainGetEntry(sampleDesc, i);
		if (a->type != TextSampleEntryAtomType) continue;
		M4_WriteTX3G(a, bs, i+1, SAMPLE_INDEX_OFFSET);
	}
	if (has_v_info) {
		u32 trans;
		/*which video shall we pick for MPEG-4, and how is the associations indicated in 3GP ???*/
		BS_WriteInt(bs, 0, 16);
		BS_WriteInt(bs, 0, 16);
		trans = mdia->mediaTrack->Header->matrix[6]; trans >>= 16;
		BS_WriteInt(bs, trans, 16);
		trans = mdia->mediaTrack->Header->matrix[7]; trans >>= 16;
		BS_WriteInt(bs, trans, 16);
	}

	BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(bs);
	*out_esd = esd;
	return M4OK;
}

M4Err M4_RewriteTextSample(M4Sample *samp, u32 sampleDescriptionIndex, u32 sample_dur)
{
	BitStream *bs;
	u32 pay_start, txt_size;
	Bool is_utf_16 = 0;
	if (!samp || !samp->data || !samp->dataLength) return M4OK;

	bs = NewBitStream(samp->data, samp->dataLength, BS_READ);
	txt_size = BS_ReadInt(bs, 16);
	DeleteBitStream(bs);

	/*remove BOM*/
	pay_start = 2;
	if (txt_size>2) {
		/*seems 3GP only accepts BE UTF-16 (no LE, no UTF32)*/
		if (((u8) samp->data[2]==(u8) 0xFE) && ((u8)samp->data[3]==(u8) 0xFF)) {
			is_utf_16 = 1;
			pay_start = 4;
			txt_size -= 2;
		}
	}

	/*rewrite as TTU(1)*/
	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, is_utf_16, 1);
	BS_WriteInt(bs, 0, 4);
	BS_WriteInt(bs, 1, 3);
	BS_WriteInt(bs, 8 + samp->dataLength - pay_start, 16);
	BS_WriteInt(bs, sampleDescriptionIndex + SAMPLE_INDEX_OFFSET, 8);
	BS_WriteInt(bs, sample_dur, 24);
	/*write text size*/
	BS_WriteInt(bs, txt_size, 16);
	if (txt_size) BS_WriteData(bs, samp->data + pay_start, samp->dataLength - pay_start);

	free(samp->data);
	samp->data = NULL;
	BS_GetContent(bs, (unsigned char **) &samp->data, &samp->dataLength);
	DeleteBitStream(bs);
	return M4OK;
}


M4Err M4_GetEncodedTX3G(M4File *file, u32 track, u32 sidx, u32 sidx_offset, char **tx3g, u32 *tx3g_size)
{
	BitStream *bs;
	TrackAtom *trak;
	TextSampleEntryAtom *a;
	
	trak = GetTrackFromFile(file, track);
	if (!trak) return M4BadParam;

	a = (TextSampleEntryAtom *) ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, sidx-1);
	if (!a || (a->type != TextSampleEntryAtomType)) return M4BadParam;
	
	bs = NewBitStream(NULL, 0, BS_WRITE);
	M4_WriteTX3G(a, bs, sidx, sidx_offset);
	*tx3g = NULL;
	*tx3g_size = 0;
	BS_GetContent(bs, (unsigned char **)tx3g, tx3g_size);
	DeleteBitStream(bs);
	return M4OK;
}

