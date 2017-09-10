// 10 september 2017
package main

// note: this is an ad-hoc tool I wrote to test two different rips of the same CD to make sure my CD drive wasn't broken after a different damaged CD didn't damage the drive somehow
// the rips were slightly different; I wanted to see how
// I might make more ad-hoc modifications later (maybe in a different file) later, but still
// resources:
// - https://stackoverflow.com/questions/29565068/mp4-file-format-specification
// - http://l.web.umkc.edu/lizhu/teaching/2016sp.video-communication/ref/mp4.pdf
// - http://jchblog.u.qiniudn.com/doc/ISO_IEC_14496-14_2003-11-15.pdf

import (
	"fmt"
	"os"
	"io"
	"io/ioutil"
	"bytes"
	"encoding/binary"
	"reflect"
	"encoding/hex"
)

type FourCC uint32

func (f FourCC) Bytes() []byte {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, uint32(f))
	return b
}

func (f FourCC) EqualsString(expected string) bool {
	expected = expected[:4]		// compiler intrinsic bounds check
	return bytes.Equal(f.Bytes(), []byte(expected))
}

func (f FourCC) String() string {
	return fmt.Sprintf("%q", string(f.Bytes()))
}

type Box struct {
	Size			uint32
	Type			FourCC
	LargeSize		uint64
	UserType		[16]byte

	Contents		[]byte
}

func ReadBox(r io.Reader) (*Box, error) {
	b := new(Box)
	err := binary.Read(r, binary.BigEndian, &(b.Size))
	if err != nil {
		return nil, err
	}
	err = binary.Read(r, binary.BigEndian, &(b.Type))
	if err != nil {
		return nil, err
	}
	size := uint64(b.Size)
	// size includes the first two fields
	sizeToRemove := uint64(8)
	if size == 1 {
		err = binary.Read(r, binary.BigEndian, &(b.LargeSize))
		if err != nil {
			return nil, err
		}
		size = b.LargeSize
		// and size includes this field too
		sizeToRemove += 8
	}
	if size == 0 {
		b.Contents, err = ioutil.ReadAll(r)
		if err != nil {
			return nil, err
		}
	} else {
		b.Contents = make([]byte, size - sizeToRemove)
		_, err = io.ReadFull(r, b.Contents)
		if err != nil {
			return nil, err
		}
	}
	if b.Type.EqualsString("uuid") {
		copy(b.UserType[:], b.Contents[:16])
		b.Contents = b.Contents[16:]
	}
	return b, nil
}

type MovieHeaderBox struct {
	// FullBox fields
	Version		uint8
	Flags		[3]uint8		// none used here

	// MovieHeaderBox fields
	CreationTime		uint64
	ModificationTime	uint64
	Timescale			uint32
	Duration			uint64
	Rate				int32
	Volume			int16
	Reserved			uint16
	Reserved2		[2]uint32
	Matrix			[9]int32
	Predefined		[6]uint32
	NextTrackID		uint32

	Remainder		[]byte		// unread portion of box
}

func NewMovieHeaderBox(b *Box) (*MovieHeaderBox, error) {
	var u32s [4]uint32
	var err error

	m := new(MovieHeaderBox)
	m.Version = b.Contents[0]
	copy(m.Flags[:], b.Contents[1:4])

	r := bytes.NewReader(b.Contents[4:])
	readif := func(x interface{}) {
		if err == nil {
			err = binary.Read(r, binary.BigEndian, x)
		}
	}
	switch m.Version {
	case 0:
		readif(&u32s)
		m.CreationTime = uint64(u32s[0])
		m.ModificationTime = uint64(u32s[1])
		m.Timescale = u32s[2]
		m.Duration = uint64(u32s[3])
	case 1:
		readif(&(m.CreationTime))
		readif(&(m.ModificationTime))
		readif(&(m.Timescale))
		readif(&(m.Duration))
	default:
		return nil, fmt.Errorf("unknown version %d\n", m.Version)
	}
	readif(&(m.Rate))
	readif(&(m.Volume))
	readif(&(m.Reserved))
	readif(&(m.Reserved2))
	readif(&(m.Matrix))
	readif(&(m.Predefined))
	readif(&(m.NextTrackID))
	if err != nil {
		return nil, err
	}
	m.Remainder = b.Contents[4:]
	m.Remainder = m.Remainder[len(m.Remainder) - r.Len():]
	return m, nil
}

type TrackHeaderBox struct {
	// FullBox fields
	Version		uint8
	Flags		[3]uint8

	// TrackHeaderBox fields
	CreationTime		uint64
	ModificationTime	uint64
	TrackID			uint32
	Reserved			uint32
	Duration			uint64
	Reserved2		[2]uint32
	Layer			int16
	AlternateGroup		int16
	Volume			int16
	Reserved3		uint16
	Matrix			[9]int32
	Width			uint32
	Height			uint32

	Remainder		[]byte		// unread portion of box
}

func NewTrackHeaderBox(b *Box) (*TrackHeaderBox, error) {
	var u32s [5]uint32
	var err error

	t := new(TrackHeaderBox)
	t.Version = b.Contents[0]
	copy(t.Flags[:], b.Contents[1:4])

	r := bytes.NewReader(b.Contents[4:])
	readif := func(x interface{}) {
		if err == nil {
			err = binary.Read(r, binary.BigEndian, x)
		}
	}
	switch t.Version {
	case 0:
		readif(&u32s)
		t.CreationTime = uint64(u32s[0])
		t.ModificationTime = uint64(u32s[1])
		t.TrackID = u32s[2]
		t.Reserved = u32s[3]
		t.Duration = uint64(u32s[4])
	case 1:
		readif(&(t.CreationTime))
		readif(&(t.ModificationTime))
		readif(&(t.TrackID))
		readif(&(t.Reserved))
		readif(&(t.Duration))
	default:
		return nil, fmt.Errorf("unknown version %d\n", t.Version)
	}
	readif(&(t.Reserved2))
	readif(&(t.Layer))
	readif(&(t.AlternateGroup))
	readif(&(t.Volume))
	readif(&(t.Reserved3))
	readif(&(t.Matrix))
	readif(&(t.Width))
	readif(&(t.Height))
	if err != nil {
		return nil, err
	}
	t.Remainder = b.Contents[4:]
	t.Remainder = t.Remainder[len(t.Remainder) - r.Len():]
	return t, nil
}

type MediaHeaderBox struct {
	// FullBox fields
	Version		uint8
	Flags		[3]uint8		// none used here

	// MediaHeaderBox fields
	CreationTime		uint64
	ModificationTime	uint64
	Timescale			uint32
	Duration			uint64
	Language			uint16
	Predefined		uint16

	Remainder		[]byte		// unread portion of box
}

func NewMediaHeaderBox(b *Box) (*MediaHeaderBox, error) {
	var u32s [4]uint32
	var err error

	m := new(MediaHeaderBox)
	m.Version = b.Contents[0]
	copy(m.Flags[:], b.Contents[1:4])

	r := bytes.NewReader(b.Contents[4:])
	readif := func(x interface{}) {
		if err == nil {
			err = binary.Read(r, binary.BigEndian, x)
		}
	}
	switch m.Version {
	case 0:
		readif(&u32s)
		m.CreationTime = uint64(u32s[0])
		m.ModificationTime = uint64(u32s[1])
		m.Timescale = u32s[2]
		m.Duration = uint64(u32s[3])
	case 1:
		readif(&(m.CreationTime))
		readif(&(m.ModificationTime))
		readif(&(m.Timescale))
		readif(&(m.Duration))
	default:
		return nil, fmt.Errorf("unknown version %d\n", m.Version)
	}
	readif(&(m.Language))
	readif(&(m.Predefined))
	if err != nil {
		return nil, err
	}
	m.Remainder = b.Contents[4:]
	m.Remainder = m.Remainder[len(m.Remainder) - r.Len():]
	return m, nil
}

type HandlerBox struct {
	// FullBox fields
	Version		uint8
	Flags		[3]uint8		// none used here

	// HandlerBox fields
	Predefined		uint32
	HandlerType		FourCC
	Reserved			[3]uint32
	Name			string

	Remainder		[]byte		// unread portion of box
}

func NewHandlerBox(b *Box) (*HandlerBox, error) {
	var err error

	h := new(HandlerBox)
	h.Version = b.Contents[0]
	copy(h.Flags[:], b.Contents[1:4])

	r := bytes.NewReader(b.Contents[4:])
	readif := func(x interface{}) {
		if err == nil {
			err = binary.Read(r, binary.BigEndian, x)
		}
	}
	readif(&(h.Predefined))
	readif(&(h.HandlerType))
	readif(&(h.Reserved))
	if err != nil {
		return nil, err
	}
	h.Remainder = b.Contents[4:]
	h.Remainder = h.Remainder[len(h.Remainder) - r.Len():]

	// now populate h.Name
	zeroTerminator := bytes.IndexByte(h.Remainder, 0)
	if zeroTerminator == -1 {
		return nil, fmt.Errorf("string not null-terminated")
	}
	h.Name = string(h.Remainder[:zeroTerminator])
	h.Remainder = h.Remainder[zeroTerminator + 1:]

	return h, nil
}

func main() {
	fa, err := os.Open(os.Args[1])
	if err != nil {
		panic(err)
	}
	ba, err := ioutil.ReadAll(fa)
	if err != nil {
		panic(err)
	}
	fa.Close()
	fb, err := os.Open(os.Args[2])
	if err != nil {
		panic(err)
	}
	bb, err := ioutil.ReadAll(fb)
	if err != nil {
		panic(err)
	}
	fb.Close()

	if len(ba) != len(bb) {
		fmt.Println("files differ in size")
		os.Exit(1)
	}

	ra := bytes.NewReader(ba)
	rb := bytes.NewReader(bb)
	i := 1
	bad := false
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// remove this to just dump top-level boxes
		// in the files I need(ed) to test, only moov differed
		if a.Type.EqualsString("moov") {
			ra = bytes.NewReader(a.Contents)
			rb = bytes.NewReader(b.Contents)
			i = 1
			break
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("box %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// we found moov; go through its boxes
	// all three boxes (mvhd, trak, and udta) differ
	var mvhda, mvhdb *Box
	var traka, trakb *Box
	var udtaa, udtab *Box
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// likewise, remove all this to dump moov's contents
		if a.Type.EqualsString("mvhd") {
			mvhda = a
			mvhdb = b
		} else if a.Type.EqualsString("trak") {
			traka = a
			trakb = b
		} else if a.Type.EqualsString("udta") {
			udtaa = a
			udtab = b
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("block %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// now process the mvhds
	// only the creation time and modification time differ; this is perfectly fine
	ma, err := NewMovieHeaderBox(mvhda)
	if err != nil {
		panic(err)
	}
	mb, err := NewMovieHeaderBox(mvhdb)
	if err != nil {
		panic(err)
	}
	diff := func(a interface{}, b interface{}, name string, mark bool) {
		// remove this line to unsilence ctime and mtime
		if !mark { return }
		if !reflect.DeepEqual(a, b) {
			fmt.Printf("%s differs (%v vs %v)\n", name, a, b)
			if mark {
				bad = true
			}
		}
	}
	{
		a := ma
		b := mb
		diff(a.Version, b.Version, "* mvhd version", true)
		diff(a.Flags, b.Flags, "* mvhd flags", true)
		diff(a.CreationTime, b.CreationTime, "* mvhd creation time", false)
		diff(a.ModificationTime, b.ModificationTime, "* mvhd modification time", false)
		diff(a.Timescale, b.Timescale, "* mvhd timescale", true)
		diff(a.Duration, b.Duration, "* mvhd duration", true)
		diff(a.Rate, b.Rate, "* mvhd rate", true)
		diff(a.Volume, b.Volume, "* mvhd volume", true)
		diff(a.Reserved, b.Reserved, "* mvhd reserved", true)
		diff(a.Reserved2, b.Reserved2, "* mvhd reserved 2", true)
		diff(a.Matrix, b.Matrix, "* mvhd matrix", true)
		diff(a.Predefined, b.Predefined, "* mvhd predefined", true)
		diff(a.NextTrackID, b.NextTrackID, "* mvhd next track ID", true)
		diff(bytes.Equal(a.Remainder, b.Remainder), true,
			"* mvhd extra data", true)
	}

	// now process trak; this is also a recursive box
	// both boxes (tkhd and mdia) differ
	var tkhda, tkhdb *Box
	var mdiaa, mdiab *Box
	ra = bytes.NewReader(traka.Contents)
	rb = bytes.NewReader(trakb.Contents)
	i = 1
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// likewise, remove all this to dump trak's contents
		if a.Type.EqualsString("tkhd") {
			tkhda = a
			tkhdb = b
		} else if a.Type.EqualsString("mdia") {
			mdiaa = a
			mdiab = b
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("block %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// now process the tkhds
	// as above, only the creation time and modification time differ; this is perfectly fine
	ta, err := NewTrackHeaderBox(tkhda)
	if err != nil {
		panic(err)
	}
	tb, err := NewTrackHeaderBox(tkhdb)
	if err != nil {
		panic(err)
	}
	diff = func(a interface{}, b interface{}, name string, mark bool) {
		// remove this line to unsilence ctime and mtime
		if !mark { return }
		if !reflect.DeepEqual(a, b) {
			fmt.Printf("%s differs (%v vs %v)\n", name, a, b)
			if mark {
				bad = true
			}
		}
	}
	{
		a := ta
		b := tb
		diff(a.Version, b.Version, "* tkhd version", true)
		diff(a.Flags, b.Flags, "* tkhd flags", true)
		diff(a.CreationTime, b.CreationTime, "* tkhd creation time", false)
		diff(a.ModificationTime, b.ModificationTime, "* tkhd modification time", false)
		diff(a.TrackID, b.TrackID, "* tkhd track ID", true)
		diff(a.Reserved, b.Reserved, "* tkhd reserved", true)
		diff(a.Duration, b.Duration, "* tkhd duration", true)
		diff(a.Reserved2, b.Reserved2, "* tkhd reserved 2", true)
		diff(a.Layer, b.Layer, "* tkhd layer", true)
		diff(a.AlternateGroup, b.AlternateGroup, "* tkhd alternate group", true)
		diff(a.Volume, b.Volume, "* tkhd volume", true)
		diff(a.Reserved3, b.Reserved3, "* tkhd reserved 3", true)
		diff(a.Matrix, b.Matrix, "* tkhd matrix", true)
		diff(a.Width, b.Width, "* tkhd width", true)
		diff(a.Height, b.Height, "* tkhd height", true)
		diff(bytes.Equal(a.Remainder, b.Remainder), true,
			"* tkhd extra data", true)
	}

	// and now mdia
	// and guess what! it's a recursive box!!
	// but thankfully, only the mdhd box differs, and that one isn't recursive
	ra = bytes.NewReader(mdiaa.Contents)
	rb = bytes.NewReader(mdiab.Contents)
	i = 1
	var mdhda, mdhdb *Box
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// likewise, remove all this to dump mdia's contents
		if a.Type.EqualsString("mdhd") {
			mdhda = a
			mdhdb = b
			break
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("block %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// now process the mdhds
	// yet again, only the creation time and modification time differ; this is perfectly fine
	mda, err := NewMediaHeaderBox(mdhda)
	if err != nil {
		panic(err)
	}
	mdb, err := NewMediaHeaderBox(mdhdb)
	if err != nil {
		panic(err)
	}
	diff = func(a interface{}, b interface{}, name string, mark bool) {
		// remove this line to unsilence ctime and mtime
		if !mark { return }
		if !reflect.DeepEqual(a, b) {
			fmt.Printf("%s differs (%v vs %v)\n", name, a, b)
			if mark {
				bad = true
			}
		}
	}
	{
		a := mda
		b := mdb
		diff(a.Version, b.Version, "* mdhd version", true)
		diff(a.Flags, b.Flags, "* mdhd flags", true)
		diff(a.CreationTime, b.CreationTime, "* mdhd creation time", false)
		diff(a.ModificationTime, b.ModificationTime, "* mdhd modification time", false)
		diff(a.Timescale, b.Timescale, "* mdhd track ID", true)
		diff(a.Duration, b.Duration, "* mdhd duration", true)
		diff(a.Language, b.Language, "* mdhd language", true)
		diff(a.Predefined, b.Predefined, "* mdhd predefined", true)
		diff(bytes.Equal(a.Remainder, b.Remainder), true,
			"* mdhd extra data", true)
	}

	// and now udta, which is also a recursive box
	// it only contains meta in all the files I need(ed)
	ra = bytes.NewReader(udtaa.Contents)
	rb = bytes.NewReader(udtab.Contents)
	i = 1
	var metaa, metab *Box
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// likewise, remove all this to dump udta's contents
		if a.Type.EqualsString("meta") {
			metaa = a
			metab = b
			break
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("block %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// the meta box also contains boxes
	// however, it's also a FullBox, so we have to process the versixxxxxx and flags
	if !bytes.Equal(metaa.Contents[:4], metab.Contents[:4]) {
		fmt.Printf("* meta version+flags differ (%x vs %x)\n",
			metaa.Contents[:4], metab.Contents[:4])
		bad = true
	}
	// technically there is a format, but in the case of my files we don't have to worry
	// the only differing box is the hdlr box
	ra = bytes.NewReader(metaa.Contents[4:])
	rb = bytes.NewReader(metab.Contents[4:])
	i = 1
	var hdlra, hdlrb *Box
	for {
		a, err := ReadBox(ra)
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		b, err := ReadBox(rb)
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
		if err != nil {
			panic(err)
		}

		skip := false

		if a.Size != b.Size {
			fmt.Printf("block %d: size field differs (%v vs %v)\n",
				i, a.Size, b.Size)
			bad = true
			skip = true
		}
		if a.Type != b.Type {
			fmt.Printf("block %d: type field differs (%v vs %v)\n",
				i, a.Type, b.Type)
			bad = true
			skip = true
		}
		if a.LargeSize != b.LargeSize {
			fmt.Printf("block %d: large size field differs (%v vs %v)\n",
				i, a.LargeSize, b.LargeSize)
			bad = true
			skip = true
		}
		if a.UserType != b.UserType {
			fmt.Printf("block %d: user type field differs (%x vs %x)\n",
				i, a.UserType, b.UserType)
			bad = true
			skip = true
		}

		// likewise, remove all this to dump meta's contents
		if a.Type.EqualsString("hdlr") {
			hdlra = a
			hdlrb = b
			break
		}
		continue

		if len(a.Contents) != len(b.Contents) {
			fmt.Printf("block %d: content size differs (%v vs %v)\n",
				i, len(a.Contents), len(b.Contents))
			bad = true
		}
		if !bytes.Equal(a.Contents, b.Contents) {
			fmt.Printf("block %d: contents differ \n", i)
			bad = true
		}

		fmt.Printf("- %d ", i)
		if skip {
			fmt.Printf("(skipped)")
		} else {
			fmt.Printf("type %v", a.Type)
		}
		fmt.Printf("\n")
		i++
	}

	// and FINALLY, process the hdlrs
	// as above, only the creation time and modification time differ; this is perfectly fine
	ha, err := NewHandlerBox(hdlra)
	if err != nil {
		panic(err)
	}
	hb, err := NewHandlerBox(hdlrb)
	if err != nil {
		panic(err)
	}
	diff = func(a interface{}, b interface{}, name string, mark bool) {
		// remove this line to unsilence ctime and mtime
//		if !mark { return }
		if !reflect.DeepEqual(a, b) {
			fmt.Printf("%s differs (%v vs %v)\n", name, a, b)
			if mark {
				bad = true
			}
		}
	}
	{
		a := ha
		b := hb
		diff(a.Version, b.Version, "* hdlr version", true)
		diff(a.Flags, b.Flags, "* hdlr flags", true)
		diff(a.Predefined, b.Predefined, "* hdlr predefined", true)
		diff(a.HandlerType, b.HandlerType, "* hdlr handler type", true)
		diff(a.Reserved, b.Reserved, "* hdlr reserved", true)
		diff(a.Name, b.Name, "* hdlr name", true)
		// this is the only other thing that's different
		// I just have no idea what this extra byte is for
		// is it just uninitialized memory?
		// remove the false && to mark it as a problem
		if false && !bytes.Equal(a.Remainder, b.Remainder) {
			fmt.Printf("* hdlr extra data differs: \na:\n%sb:\n%s\n",
				hex.Dump(a.Remainder),
				hex.Dump(b.Remainder))
			bad = true
		}
	}

	if bad {
		os.Exit(1)
	}
}
