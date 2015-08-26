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

void amr3_del(Atom *s)
{
	AMRSampleEntryAtom *ptr = (AMRSampleEntryAtom *)s;
	if (ptr == NULL) return;
	if (ptr->amr_info) DelAtom((Atom *)ptr->amr_info);

	free(ptr);
}


M4Err amr3_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sub_read;
	AMRSampleEntryAtom *ptr = (AMRSampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ReadAudioSampleEntry((AudioSampleEntryAtom*)s, bs, read);

	e = ParseAtom((Atom **)&ptr->amr_info, bs, &sub_read);
	if (e) return e;
	*read += sub_read;
	if (*read != s->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *amr3_New()
{
	AMRSampleEntryAtom *tmp = (AMRSampleEntryAtom *) malloc(sizeof(AMRSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(AMRSampleEntryAtom));
	InitAudioSampleEntry((AudioSampleEntryAtom*)tmp);
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err amr3_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	AMRSampleEntryAtom *ptr = (AMRSampleEntryAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	WriteAudioSampleEntry((AudioSampleEntryAtom*)s, bs);
	return WriteAtom((Atom *)ptr->amr_info, bs);
}

M4Err amr3_Size(Atom *s)
{
	M4Err e;
	AMRSampleEntryAtom *ptr = (AMRSampleEntryAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	SizeAudioSampleEntry((AudioSampleEntryAtom*)s);

	e = SizeAtom((Atom *)ptr->amr_info);
	if (e) return e;
	ptr->size += ptr->amr_info->size;
	return M4OK;
}

#endif //M4_READ_ONLY


void d263_del(Atom *s)
{
	H263ConfigAtom *ptr = (H263ConfigAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err d263_Read(Atom *s, BitStream *bs, u64 *read)
{
	H263ConfigAtom *ptr = (H263ConfigAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->vendor = BS_ReadInt(bs, 32);
	ptr->decoder_version = BS_ReadInt(bs, 8);
	ptr->Level = BS_ReadInt(bs, 8);
	ptr->Profile = BS_ReadInt(bs, 8);

	*read += 7;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *d263_New()
{
	H263ConfigAtom *tmp = (H263ConfigAtom *) malloc(sizeof(H263ConfigAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(H263ConfigAtom));
	tmp->type = H263ConfigAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err d263_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	H263ConfigAtom *ptr = (H263ConfigAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->vendor, 32);
	BS_WriteInt(bs, ptr->decoder_version, 8);
	BS_WriteInt(bs, ptr->Level, 8);
	BS_WriteInt(bs, ptr->Profile, 8);
	return M4OK;
}

M4Err d263_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 7;
	return M4OK;
}

#endif //M4_READ_ONLY


void damr_del(Atom *s)
{
	AMRConfigAtom *ptr = (AMRConfigAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err damr_Read(Atom *s, BitStream *bs, u64 *read)
{
	AMRConfigAtom *ptr = (AMRConfigAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->vendor = BS_ReadInt(bs, 32);
	ptr->decoder_version = BS_ReadInt(bs, 8);
	ptr->mode_set = BS_ReadInt(bs, 16);
	ptr->mode_change_period = BS_ReadInt(bs, 8);
	ptr->frames_per_sample = BS_ReadInt(bs, 8);
	*read += 9;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *damr_New()
{
	AMRConfigAtom *tmp = (AMRConfigAtom *) malloc(sizeof(AMRConfigAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(AMRConfigAtom));
	tmp->type = AMRConfigAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err damr_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	AMRConfigAtom *ptr = (AMRConfigAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->vendor, 32);
	BS_WriteInt(bs, ptr->decoder_version, 8);
	BS_WriteInt(bs, ptr->mode_set, 16);
	BS_WriteInt(bs, ptr->mode_change_period, 8);
	BS_WriteInt(bs, ptr->frames_per_sample, 8);
	return M4OK;
}

M4Err damr_Size(Atom *s)
{
	M4Err e;
	AMRConfigAtom *ptr = (AMRConfigAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += 9;
	return M4OK;
}

#endif //M4_READ_ONLY


void h263_del(Atom *s)
{
	H263SampleEntryAtom *ptr = (H263SampleEntryAtom *)s;
	if (ptr == NULL) return;

	if (ptr->h263_config) DelAtom((Atom *)ptr->h263_config);
	free(ptr);
}


M4Err h263_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sub_read;
	H263SampleEntryAtom *ptr = (H263SampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ReadVideoSampleEntry((VisualSampleEntryAtom *)ptr, bs, read);

	e = ParseAtom((Atom **)&ptr->h263_config, bs, &sub_read);
	if (e) return e;
	*read += sub_read;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *h263_New()
{
	H263SampleEntryAtom *tmp = (H263SampleEntryAtom *) malloc(sizeof(H263SampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(H263SampleEntryAtom));

	InitVideoSampleEntry((VisualSampleEntryAtom *)tmp);

	tmp->type = H263SampleEntryAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err h263_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	H263SampleEntryAtom *ptr = (H263SampleEntryAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	WriteVideoSampleEntry((VisualSampleEntryAtom *)s, bs);

	e = WriteAtom((Atom *)ptr->h263_config, bs);
	if (e) return e;
	return M4OK;
}

M4Err h263_Size(Atom *s)
{
	M4Err e;
	H263SampleEntryAtom *ptr = (H263SampleEntryAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	SizeVideoSampleEntry((VisualSampleEntryAtom *)s);

	e = SizeAtom((Atom *)ptr->h263_config);
	if (e) return e;
	ptr->size += ptr->h263_config->size;
	return M4OK;
}

#endif //M4_READ_ONLY



Atom *ftab_New()
{
	FontTableAtom *tmp;
	SAFEALLOC(tmp, sizeof(FontTableAtom));
	if (!tmp) return NULL;
	tmp->type = FontTableAtomType;
	return (Atom *) tmp;
}
void ftab_del(Atom *s)
{
	FontTableAtom *ptr = (FontTableAtom *)s;
	if (ptr->fonts) {
		u32 i;
		for (i=0; i<ptr->entry_count; i++) 
			if (ptr->fonts[i].fontName) free(ptr->fonts[i].fontName);
		free(ptr->fonts);
	}
	free(ptr);
}
M4Err ftab_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 i;
	FontTableAtom *ptr = (FontTableAtom *)s;
	ptr->entry_count = BS_ReadInt(bs, 16);
	*read += 2;
	SAFEALLOC(ptr->fonts, sizeof(FontRecord)*ptr->entry_count);
	for (i=0; i<ptr->entry_count; i++) {
		u32 len;
		ptr->fonts[i].fontID = BS_ReadInt(bs, 16);
		len = BS_ReadInt(bs, 8);
		*read += 3;
		if (len) {
			SAFEALLOC(ptr->fonts[i].fontName, sizeof(char)*(len+1));
			BS_ReadData(bs, ptr->fonts[i].fontName, len);
			*read += len;
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err ftab_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	FontTableAtom *ptr = (FontTableAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->entry_count, 16);
	for (i=0; i<ptr->entry_count; i++) {
		BS_WriteInt(bs, ptr->fonts[i].fontID, 16);
		if (ptr->fonts[i].fontName) {
			u32 len = strlen(ptr->fonts[i].fontName);
			BS_WriteInt(bs, len, 8);
			BS_WriteData(bs, ptr->fonts[i].fontName, len);
		} else {
			BS_WriteInt(bs, 0, 8);
		}
	}
	return M4OK;
}
M4Err ftab_Size(Atom *s)
{
	u32 i;
	FontTableAtom *ptr = (FontTableAtom *)s;
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 2;
	for (i=0; i<ptr->entry_count; i++) {
		s->size += 3;
		if (ptr->fonts[i].fontName) s->size += strlen(ptr->fonts[i].fontName);
	}
	return M4OK;
}

#endif




Atom *tx3g_New()
{
	TextSampleEntryAtom *tmp;
	SAFEALLOC(tmp, sizeof(TextSampleEntryAtom));
	if (!tmp) return NULL;
	tmp->type = TextSampleEntryAtomType;
	return (Atom *) tmp;
}

void tx3g_del(Atom *s)
{
	TextSampleEntryAtom *ptr = (TextSampleEntryAtom*)s;
	if (ptr->font_table) DelAtom((Atom *)ptr->font_table);
	free(ptr);
}

static u32 gpp_read_rgba(BitStream *bs)
{
	u8 r, g, b, a;
	u32 col;
	r = BS_ReadInt(bs, 8);
	g = BS_ReadInt(bs, 8);
	b = BS_ReadInt(bs, 8);
	a = BS_ReadInt(bs, 8);
	col = a; col<<=8; 
	col |= r; col<<=8; 
	col |= g; col<<=8; 
	col |= b;
	return col;
}

#define GPP_BOX_SIZE	8
static void gpp_read_box(BitStream *bs, BoxRecord *rec)
{
	rec->top = BS_ReadInt(bs, 16);
	rec->left = BS_ReadInt(bs, 16);
	rec->bottom = BS_ReadInt(bs, 16);
	rec->right = BS_ReadInt(bs, 16);
}

#define GPP_STYLE_SIZE	12
static void gpp_read_style(BitStream *bs, StyleRecord *rec)
{
	rec->startChar = BS_ReadInt(bs, 16);
	rec->endChar = BS_ReadInt(bs, 16);
	rec->fontID = BS_ReadInt(bs, 16);
	rec->style_flags = BS_ReadInt(bs, 8);
	rec->font_size = BS_ReadInt(bs, 8);
	rec->text_color = gpp_read_rgba(bs);
}

M4Err tx3g_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr;
	Atom *a;
	TextSampleEntryAtom *ptr = (TextSampleEntryAtom*)s;

	BS_ReadData(bs, (unsigned char*)ptr->reserved, 6);
	ptr->dataReferenceIndex = BS_ReadInt(bs, 16);
	ptr->displayFlags = BS_ReadInt(bs, 32);
	ptr->horizontal_justification = BS_ReadInt(bs, 8);
	ptr->vertical_justification = BS_ReadInt(bs, 8);
	ptr->back_color = gpp_read_rgba(bs);
	gpp_read_box(bs, &ptr->default_box);
	gpp_read_style(bs, &ptr->default_style);
	*read += 18 + GPP_BOX_SIZE + GPP_STYLE_SIZE;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		if (a->type==FontTableAtomType) {
			if (ptr->font_table) DelAtom((Atom *) ptr->font_table);
			ptr->font_table = (FontTableAtom *)a;
		} else {
			DelAtom(a);
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}


void gpp_write_rgba(BitStream *bs, u32 col)
{
	BS_WriteInt(bs, (col>>16) & 0xFF, 8);
	BS_WriteInt(bs, (col>>8) & 0xFF, 8);
	BS_WriteInt(bs, (col) & 0xFF, 8);
	BS_WriteInt(bs, (col>>24) & 0xFF, 8);
}

void gpp_write_box(BitStream *bs, BoxRecord *rec)
{
	BS_WriteInt(bs, rec->top, 16);
	BS_WriteInt(bs, rec->left, 16);
	BS_WriteInt(bs, rec->bottom, 16);
	BS_WriteInt(bs, rec->right, 16);
}

#define GPP_STYLE_SIZE	12
void gpp_write_style(BitStream *bs, StyleRecord *rec)
{
	BS_WriteInt(bs, rec->startChar, 16);
	BS_WriteInt(bs, rec->endChar, 16);
	BS_WriteInt(bs, rec->fontID, 16);
	BS_WriteInt(bs, rec->style_flags, 8);
	BS_WriteInt(bs, rec->font_size, 8);
	gpp_write_rgba(bs, rec->text_color);
}

#ifndef M4_READ_ONLY

M4Err tx3g_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextSampleEntryAtom *ptr = (TextSampleEntryAtom*)s;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteData(bs, (unsigned char*)ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);
	BS_WriteInt(bs, ptr->displayFlags, 32);
	BS_WriteInt(bs, ptr->horizontal_justification, 8);
	BS_WriteInt(bs, ptr->vertical_justification, 8);
	gpp_write_rgba(bs, ptr->back_color);
	gpp_write_box(bs, &ptr->default_box);
	gpp_write_style(bs, &ptr->default_style);
	return WriteAtom((Atom *) ptr->font_table, bs);
}

M4Err tx3g_Size(Atom *s)
{
	TextSampleEntryAtom *ptr = (TextSampleEntryAtom*)s;
	M4Err e = Atom_Size(s);
	if (e) return e;
	/*base + this  + box + style*/
	s->size += 18 + GPP_BOX_SIZE + GPP_STYLE_SIZE;
	if (ptr->font_table) {
		e = SizeAtom((Atom *) ptr->font_table);
		if (e) return e;
		s->size += ptr->font_table->size;
	}
	return M4OK;
}

#endif

Atom *styl_New()
{
	TextStyleAtom *tmp;
	SAFEALLOC(tmp, sizeof(TextStyleAtom));
	if (!tmp) return NULL;
	tmp->type = TextStyleAtomType;
	return (Atom *) tmp;
}

void styl_del(Atom *s)
{
	TextStyleAtom*ptr = (TextStyleAtom*)s;
	if (ptr->styles) free(ptr->styles);
	free(ptr);
}

M4Err styl_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 i;
	TextStyleAtom*ptr = (TextStyleAtom*)s;
	ptr->entry_count = BS_ReadInt(bs, 16);
	*read += 2;
	if (ptr->entry_count) {
		SAFEALLOC(ptr->styles, sizeof(StyleRecord)*ptr->entry_count);
		for (i=0; i<ptr->entry_count; i++) {
			gpp_read_style(bs, &ptr->styles[i]);
			*read += GPP_STYLE_SIZE;
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err styl_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	TextStyleAtom*ptr = (TextStyleAtom*)s;
	e = Atom_Write(s, bs);

	BS_WriteInt(bs, ptr->entry_count, 16);
	for (i=0; i<ptr->entry_count; i++) gpp_write_style(bs, &ptr->styles[i]);
	return M4OK;
}

M4Err styl_Size(Atom *s)
{
	TextStyleAtom*ptr = (TextStyleAtom*)s;
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 2 + ptr->entry_count * GPP_STYLE_SIZE;
	return M4OK;
}

#endif

Atom *hlit_New()
{
	TextHighlightAtom *tmp;
	SAFEALLOC(tmp, sizeof(TextHighlightAtom));
	if (!tmp) return NULL;
	tmp->type = TextHighlightAtomType;
	return (Atom *) tmp;
}

void hlit_del(Atom *s)
{
	free(s);
}

M4Err hlit_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextHighlightAtom *ptr = (TextHighlightAtom *)s;
	ptr->startcharoffset = BS_ReadInt(bs, 16);
	ptr->endcharoffset = BS_ReadInt(bs, 16);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err hlit_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextHighlightAtom *ptr = (TextHighlightAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->startcharoffset, 16);
	BS_WriteInt(bs, ptr->endcharoffset, 16);
	return M4OK;
}

M4Err hlit_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}

#endif

Atom *hclr_New()
{
	TextHighlightColorAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextHighlightColorAtom));
	if (!tmp) return NULL;
	tmp->type = TextHighlightColorAtomType;
	return (Atom *) tmp;
}

void hclr_del(Atom *s)
{
	free(s);
}

M4Err hclr_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextHighlightColorAtom*ptr = (TextHighlightColorAtom*)s;
	ptr->hil_color = gpp_read_rgba(bs);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err hclr_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextHighlightColorAtom*ptr = (TextHighlightColorAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	gpp_write_rgba(bs, ptr->hil_color);
	return M4OK;
}

M4Err hclr_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}

#endif

Atom *krok_New()
{
	TextKaraokeAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextKaraokeAtom));
	if (!tmp) return NULL;
	tmp->type = TextKaraokeAtomType;
	return (Atom *) tmp;
}

void krok_del(Atom *s)
{
	TextKaraokeAtom*ptr = (TextKaraokeAtom*)s;
	if (ptr->records) free(ptr->records);
	free(ptr);
}

M4Err krok_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextKaraokeAtom*ptr = (TextKaraokeAtom*)s;

	ptr->highlight_starttime = BS_ReadInt(bs, 32);
	ptr->entrycount = BS_ReadInt(bs, 16);
	*read += 6;
	if (ptr->entrycount) {
		u32 i;
		SAFEALLOC(ptr->records, sizeof(KaraokeRecord)*ptr->entrycount);
		for (i=0; i<ptr->entrycount; i++) {
			ptr->records[i].highlight_endtime = BS_ReadInt(bs, 32);
			ptr->records[i].start_charoffset = BS_ReadInt(bs, 16);
			ptr->records[i].end_charoffset = BS_ReadInt(bs, 16);
			*read += 8;
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err krok_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	TextKaraokeAtom*ptr = (TextKaraokeAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->highlight_starttime, 32);
	BS_WriteInt(bs, ptr->entrycount, 16);
	for (i=0; i<ptr->entrycount; i++) {
		BS_WriteInt(bs, ptr->records[i].highlight_endtime, 32);
		BS_WriteInt(bs, ptr->records[i].start_charoffset, 16);
		BS_WriteInt(bs, ptr->records[i].end_charoffset, 16);
	}
	return M4OK;
}

M4Err krok_Size(Atom *s)
{
	TextKaraokeAtom*ptr = (TextKaraokeAtom*)s;
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 6 * 8*ptr->entrycount;
	return M4OK;
}

#endif

Atom *dlay_New()
{
	TextScrollDelayAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextScrollDelayAtom));
	if (!tmp) return NULL;
	tmp->type = TextScrollDelayAtomType;
	return (Atom *) tmp;
}

void dlay_del(Atom *s)
{
	free(s);
}

M4Err dlay_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextScrollDelayAtom*ptr = (TextScrollDelayAtom*)s;
	ptr->scroll_delay = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err dlay_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextScrollDelayAtom*ptr = (TextScrollDelayAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->scroll_delay, 32);
	return M4OK;
}

M4Err dlay_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}

#endif

Atom *href_New()
{
	TextHyperTextAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextHyperTextAtom));
	if (!tmp) return NULL;
	tmp->type = TextHyperTextAtomType;
	return (Atom *) tmp;
}

void href_del(Atom *s)
{
	TextHyperTextAtom*ptr = (TextHyperTextAtom*)s;
	if (ptr->URL) free(ptr->URL);
	if (ptr->URL_hint) free(ptr->URL_hint);
	free(ptr);
}

M4Err href_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 len;
	TextHyperTextAtom*ptr = (TextHyperTextAtom*)s;
	ptr->startcharoffset = BS_ReadInt(bs, 16);
	ptr->endcharoffset = BS_ReadInt(bs, 16);
	*read += 6;
	len = BS_ReadInt(bs, 8);
	if (len) {
		SAFEALLOC(ptr->URL, sizeof(char) * (len+1));
		BS_ReadData(bs, ptr->URL, len);
		*read += len;
	}
	len = BS_ReadInt(bs, 8);
	if (len) {
		SAFEALLOC(ptr->URL_hint, sizeof(char) * (len+1));
		BS_ReadData(bs, ptr->URL_hint, len);
		*read += len;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err href_Write(Atom *s, BitStream *bs)
{
	u32 len;
	M4Err e;
	TextHyperTextAtom*ptr = (TextHyperTextAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->startcharoffset, 16);
	BS_WriteInt(bs, ptr->endcharoffset, 16);
	if (ptr->URL) {
		len = strlen(ptr->URL);
		BS_WriteInt(bs, len, 8);
		BS_WriteData(bs, ptr->URL, len);
	} else {
		BS_WriteInt(bs, 0, 8);
	}
	if (ptr->URL_hint) {
		len = strlen(ptr->URL_hint);
		BS_WriteInt(bs, len, 8);
		BS_WriteData(bs, ptr->URL_hint, len);
	} else {
		BS_WriteInt(bs, 0, 8);
	}
	return M4OK;
}

M4Err href_Size(Atom *s)
{
	TextHyperTextAtom*ptr = (TextHyperTextAtom*)s;
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 6;
	if (ptr->URL) s->size += strlen(ptr->URL);
	if (ptr->URL_hint) s->size += strlen(ptr->URL_hint);
	return M4OK;
}

#endif


Atom *tbox_New()
{
	TextBoxAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextBoxAtom));
	if (!tmp) return NULL;
	tmp->type = TextBoxAtomType;
	return (Atom *) tmp;
}

void tbox_del(Atom *s)
{
	free(s);
}

M4Err tbox_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextBoxAtom*ptr = (TextBoxAtom*)s;
	gpp_read_box(bs, &ptr->box);
	*read += GPP_BOX_SIZE;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err tbox_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextBoxAtom*ptr = (TextBoxAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	gpp_write_box(bs, &ptr->box);
	return M4OK;
}

M4Err tbox_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}

#endif


Atom *blnk_New()
{
	TextBlinkAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextBlinkAtom));
	if (!tmp) return NULL;
	tmp->type = TextBlinkAtomType;
	return (Atom *) tmp;
}

void blnk_del(Atom *s)
{
	free(s);
}

M4Err blnk_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextBlinkAtom*ptr = (TextBlinkAtom*)s;
	ptr->startcharoffset = BS_ReadInt(bs, 16);
	ptr->endcharoffset = BS_ReadInt(bs, 16);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err blnk_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextBlinkAtom*ptr = (TextBlinkAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->startcharoffset, 16);
	BS_WriteInt(bs, ptr->endcharoffset, 16);
	return M4OK;
}

M4Err blnk_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}

#endif

Atom *twrp_New()
{
	TextWrapAtom*tmp;
	SAFEALLOC(tmp, sizeof(TextWrapAtom));
	if (!tmp) return NULL;
	tmp->type = TextWrapAtomType;
	return (Atom *) tmp;
}

void twrp_del(Atom *s)
{
	free(s);
}

M4Err twrp_Read(Atom *s, BitStream *bs, u64 *read)
{
	TextWrapAtom*ptr = (TextWrapAtom*)s;
	ptr->wrap_flag = BS_ReadInt(bs, 8);
	*read += 1;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err twrp_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TextWrapAtom*ptr = (TextWrapAtom*)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->wrap_flag, 8);
	return M4OK;
}
M4Err twrp_Size(Atom *s)
{
	M4Err e = Atom_Size(s);
	if (e) return e;
	s->size += 1;
	return M4OK;
}

#endif

