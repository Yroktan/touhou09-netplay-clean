# Natural-order link

`just exe` builds the standalone exe through `scripts/exe_layout.py`,
which places every compiled contribution at its original address itself
and links one combined object. That gets everything but the unmatched
functions and the Rich header identical, but the Rich header counts the
link's objects, so it can never match that way.

`just natural` links the exe's own objects directly instead, with the
original's libraries and `/OPT:REF /OPT:ICF` but no `/ORDER`, into
`build/natural/th09.exe`. It reports how many game functions sit at
their original address and how far from 0x401000 every function does
(the placed prefix). The goal is the original's layout from the objects
alone, as th08's exe has, after which the layout hook can go.

## Why not `/ORDER`

th08 reaches its layout with `/ORDER`. th09 cannot: its 79 `static`
functions get custom register conventions under `/O2` (making them
public changes their code), `/ORDER` cannot name them (LNK4037), and the
original has them among their objects' other functions. So the original
was linked in natural order, and its layout follows from the objects,
their order, and the order each object emits its functions.

## How cl 7.1 orders an object's functions

`/O2` implies `/Gy`: every function is its own COMDAT, emitted only if
used, and LINK keeps a COMDAT's first copy in link order. Probes with
the project's flags (`/O2 /Ob0 /Oy- /Gr /GX`) show that the code
generator works bottom-up:

- Functions are compiled in definition order, header inlines included,
  as the compiler reaches them.
- A function whose callees defined in the same file are not compiled
  yet waits. The waiting functions are compiled at the end of the file,
  in definition order, even when what they wait for is compiled long
  before (a probe: a function waiting for a header's 20th inline
  function still comes after the file's last one). A function still
  waiting for another waiting function when its turn comes is passed
  over and compiled after the rest.
- A call the compiler drops still makes its caller wait: a call to an
  empty inline function (or one whose result is unused and that does
  nothing) leaves no code once the callee is compiled, so the caller
  waits for it, and the callee's one-byte COMDAT is emitted all the
  same (LINK folds it into another `ret`). That is how a function with
  no inline callee in its code can wait, as Stage::RemoveChain does
  (it calls the empty Stage::OnRemoved). `__forceinline` does not
  inline under `/Ob0`, and inline functions the file defines but
  never uses do not move the header along.
- With a precompiled header, its inline functions are not compiled
  where the header defines them. The compiler works through them in
  the header's order alongside the file: after each function the file
  defines, it compiles the header's next inline functions, as many as
  balance that function's size (its code, about one per 20 bytes of
  machine code, not its comments or declarations). Every inline
  function of the header counts, used or not, and windows.h, math.h
  and d3dx8math.inl hold hundreds before the game's: math.h's cosf is
  about 80 small functions in, d3dx8math.inl's D3DXVec3Length about
  480. A file's functions that call a header inline not compiled yet
  wait for the end of the file. A small file reaches its end first, so
  the header's inline functions follow its own there, as the rule
  above had it.
- An implicit constructor or the `vector constructor iterator` is
  generated at its first use, which can be a header inline the compiler
  works through: Sprite's constructor generates SpriteBase's and the
  iterator.
- A header outside the precompiled one, which the source includes
  itself, is different: its inline functions are compiled where the
  source includes it, before the file's own functions (the ones the
  file uses are emitted). So an object of the original that starts
  with header inline functions includes that header itself: the GDI
  surface object starts with GdiSurface's accessors, the enemy object
  with EclManager::GetTimelineCount, pl00's with SpawnPlayerEnemy and
  ExAttack::GetSide, the bullet object with
  BulletManager::ClearDrawBuckets. gdisurface.h, enemy.h, bullet.h and
  midi.h are not in global.h for that, and pl.h holds what the
  character objects share. Taking out player.h, gui.h, ascii.h,
  archive.h or stage.h instead loses neighbours.

Both compilers, 13.10.3077 (C:\msvc710) and 13.10.6030, order the same
way, deterministically; /Zi, /Gm and the header's own optimization
change nothing.

With the precompiled global.h (the original's ./system/global.h), the
stage object compiles HermiteInterpolate, then cosf, sinf and sqrtf,
then ModulateChannel, as at 0x401000, only because RunScript and the
other functions that wait are defined in between: the leaf functions
alone are far too small to move through math.h and d3dx8math.inl.

So a small accessor between two of an object's functions in the
original is a header inline (with `/Ob0` it stays a call, and the first
object in link order that uses it keeps it), and an object's functions
follow its definition order once each function's callees and the
header's progress are accounted for.

## Identical code folding

`/OPT:ICF` does not keep the first copy in link order. LINK 7.1's code
(link.exe, decompiled; the `/VERBOSE` "Selected symbol" report prints
the result) works like this:

- Pass 1 adds every section that can fold to a group named by its
  section name (`.text`, `.rdata`, `.text$x`...), in the order it
  reads them: the objects in link order, then the library members as
  they are loaded. A section can fold unless it is writable,
  discardable or empty, or is a `_$E` dynamic initializer; of
  same-named COMDATs only the copy kept is added.
- Each group is sorted with msvcr71's `qsort`, which is not stable, by
  a key (zero in every probe) and then by size. Within each run of
  equal key and size, every section is compared with the ones after
  it and an identical one is folded into it: the copy kept is the
  first of its kind in the sorted group. Then the group is sorted and
  folded again while anything folds (twice more at most), so that
  sections whose relocations now reach folded targets fold too.

So which copy of two identical functions survives depends on every
section of the group and its size, the libraries' included: in a
probe with five identical functions, adding two unused functions to
another object moved the kept copy. The probes (the order of
`/VERBOSE`'s report, for 11 link orders) follow this exactly, but a
model of the whole link does not yet predict its 47 folds: the
library sections' order in the group is not known well enough. Of the
two identical static ModulateChannel functions, the stage's and the
sprite engine's, the natural link keeps the sprite engine's; the
original kept the stage's, which can follow from the objects matching
the original's (the same sections and sizes), not from their order.
The map lists every folded name under the kept copy's address but not
always under its object. The kept copy moves with unrelated changes:
with Tan made an (unused, so never emitted) inline function, the
natural link keeps the stage's ModulateChannel and the placed prefix
reaches 0x401470; with Tan out of line again it keeps the sprite
engine's. Timer::operator== and the small accessors that fold with it
(GdiSurface::GetHeight, Timer::AsFrames) move the same way.

Fmod and Tan stay out of line in unsorted.cpp for another reason: they
stand for math.h's fmodf and tanf (0x405710, 0x42e900), which the
code calls directly, so as inline functions nothing would emit them
and `just compare` would lose them.

Making a function inline-visible can change its callers' code (a caller
knows what an inline function does not modify, as with
`SpriteEngine::GetAnm`), so every move needs `just compare`.

## Measuring an object's order

`scripts/emit_order.py SOURCE` compiles one source (or a probe copy of
one) as the exe does and lists its code COMDATs in the order LINK
places them, with the original's address of each and a `<` where the
order goes back. `--ruler N` adds N tiny inline functions to the
precompiled header, at its start or (`--at timer.h`) just before one of
its headers, and references them from the end of the file, so that
they are all emitted: the rulers that follow each of the file's
functions show how far through the header the compiler is. For the
stage, HermiteInterpolate is followed by about 150 rulers, as much as
it and the three functions after it that wait balance (about 3000
bytes): the code of a waiting function moves the header along when
the next function is compiled.

`scripts/natural_breaks.py [OBJECT] [--summary]` lists the natural
link's functions in the original's order with an `X` at every broken
pair (what `just natural` counts), or with `--summary` the broken pairs
of each object.

## Open in the stage object

- The original's `vector constructor iterator` comes before
  Timer::Initialize, the first of timer.h's functions the stage uses;
  ours is generated with SpriteBase's constructor, after timer.h's
  functions, before AnmLoaded::StartScriptAtOrigin.
- Timer::Timer waits for Timer::Initialize and SpriteBase's
  constructor for it; the original has them after DrawLayer, so the
  compiler reached timer.h before it reached OnDraw. Ours reaches it at
  the end of the file only: even with DrawLayer defined between Reset
  and StepObjects, the header does not get there before OnDraw.

## The original's header order

The two objects built with `/O1` (the GameManager's and the main
menu's) do not use the precompiled header, and they emit the inline
functions they call but do not expand in the order their translation
unit defines them. That order is the original's header order. The main
menu has PlayerInput::IsPressed, MidiPlayer::Restart,
App::ClearUnknown77c, Sprite::SetMoveInterp and SetColorInterp,
SpriteEngine::HasTexture, SoundPlayer::UnPause,
PlayRecord::GetClearFlags, AsciiText::SetScale; the GameManager
ChainElem::SetCallback, Rng::GetSeed, GameConfig's two, then
App::ResetSubThread and GameManager::IsDemo. So SoundPlayer and
PlayRecord come after sprite.h, which the ending object agrees with
(SpriteEngine's clearers, then PlayRecord's accessors, then
GameManager::IsMode0): SoundPlayer and its tables are in sound.h and
PlayRecord in scoredat.h, both just after sprite.h in global.h (one
neighbour more). midi.h back in global.h just before App, and chain.h
before unsorted.h, as the same lists suggest, gain nothing (chain.h
first loses 29).

## Searching global.h

A loop that moves one include line of global.h (or of a header it
includes) to another place, rebuilds every object and keeps the move
when the adjacent count rises finds little: a header change rebuilds
all 46 objects (about 20 seconds; copies of the tree in a tmpfs that
share the Wine prefix by a symlink run six at once). Most moves lose
1 to 60 neighbours. Before the sound.h split the best was stage.h
before gamemanager.h (one more); after it, of 103 moves of stage.h,
ascii.h, player.h, gamemanager.h, sound.h, scoredat.h, timer.h and
archive.h, only scoredat.h before timer.h gains one, by undoing the
ending object's two and winning back two in the sprite engine's,
against the order the `/O1` objects show, so it is not taken.
common.h before math.h loses 31. The header's order is not where the
remaining breaks come from; its length before the game's headers is.

## What the original's lead would take

The original's objects reach the header's inline functions far sooner
than ours: the ending object passes PlayerInput's accessors within its
first 0x1c4 bytes of code, the app object has math.h's tanf and
d3dx8math.inl's D3DXVec3Cross before its first function, and the GUI
object starts with PlayRecord::AddClear5. Probes of what could explain
it:

- Unused inline functions do count: 400 of them at the start of
  global.h move the stage's cosf from just after HermiteInterpolate to
  after Stage::StepObjects.
- Dead code would: an unused function before an object's first one
  moves the header along like any other (/OPT:REF drops it, as th06's
  `TH06_EXE` stand-ins). But 8 KB of it before the ending's ReadInt
  does not get past d3dx8math.inl and the MemoryManager, and 21 KB
  gets past sprite.h, where the original stops at PlayerInput. Before
  the sound.h split, stand-ins of 3.4 KB gained two neighbours in the
  app object and three in the GUI's, losing most of the placed prefix
  to the code folding; after it, stand-ins of up to 6.7 KB before any
  of the first 12 functions of the app, GUI and player objects gain
  none.
- <d3dx8.h> later in unsorted.h (with <d3d8.h> where it was, before
  the Rng, GameWindow, Rotate2D, PlayerInput or Camera) loses 14 to 18
  neighbours, though the ECL object then starts as the original does
  (atan2f and fmodf right after EclManager::Load).
- The Rich header counts 88 C++ objects of cl 13.10.3077, a few of
  them the CRT's; the exe links 46 game sources. The original's objects
  were smaller, so each started with less of the header behind it, not
  more.

## The system headers' share of the queue

A probe header with three rulers between each group of includes, and a
probe source of 3000 one-line functions (each balancing about one
ruler), measures what each part of global.h puts in the queue, in
ruler units (about 15 to 20 bytes of the file's code each):

| part                                  | units |
|---------------------------------------|------:|
| windows.h (with what d3d8/dinput add) |    36 |
| math.h (97 inline functions)          |   109 |
| dinput.h, d3d8.h                      |     1 |
| d3dx8math.h and .inl (178 functions)  |   439 |
| the rest of d3dx8.h, the CRT headers  |     0 |
| mmsystem.h, dsound.h                  |     1 |
| common.h, dsutil.h                    |    16 |
| unsorted.h                            |    41 |
| timer.h                               |    32 |
| sprite.h                              |    55 |
| the other game headers                |   106 |

So the system headers are about 590 of the queue's 880 units, nearly
all of it math.h and d3dx8math.inl, which no define changes. In the
queue, cosf is at about 85, sqrtf 90, D3DXVECTOR3(x, y, z) 190, its
operator* 210, D3DXVec3Length 475, MemoryManager::Free 615,
PlayerInput::IsHeld 645, PlayRecord::AddClear5 785, GameManager::IsMode0
810.

The stage pins the original's lead. With rulers at the start of the
header, the stage has balanced 147 units by ModulateChannel and 331 by
Stage::Reset. The original compiles cosf to sqrtf before ModulateChannel
but the vector constructor after it, and D3DXVECTOR3::operator* before
Reset but D3DXVec3Length after it. So what came before d3dx8math.inl in
the original was within about 60 units (shorter) to 45 (longer) of
ours: the same windows.h and math.h, in front of the game headers.

Every other object needs far more than that. How much each one is
missing, against ours (rulers before its first function):

- the ending passes PlayerInput::IsHeld after UpdateFade, where ours
  has balanced 16 units: about 630 missing;
- the app has tanf, tan and D3DXVec3Cross before its first function,
  where ours has 6: about 490;
- the GUI has PlayRecord::AddClear5 before Gui::LoadAnm, where ours
  has 25: about 760.

A change to global.h moves every object by the same amount, and
the stage allows at most about 60 units. These leads differ from
object to object, and each is ten times that. Each object of the
original compiled something before its own functions that ours does
not (code it later dropped, or inline functions of headers it
included outside global.h that it does not use), and the stage
compiled nothing. Measured with `just natural` (800 before):

- WIN32_LEAN_AND_MEAN (12 units fewer from windows.h), alone or with
  VC_EXTRALEAN: 799, but 99 functions at their original address
  rather than 75 (the code folding again);
- VC_EXTRALEAN, NOMINMAX, STRICT, _MBCS, _WIN32_WINNT 0x400 or 0x500:
  800, no change (no inline functions depend on them); WINVER 0x400
  does not compile (window.cpp's SPI_ constants);
- D3DXINLINE as `__inline` or `__forceinline`: the same queue;
- the DirectX 8.1 headers: d3dx8math.inl is 20 units longer, the
  wrong way. (The build's record of flags does not notice an include
  path change: touch global.h afterwards, or the exe links stale
  objects.)
- in global.h, a static or extern function, a global with a
  constructor, an explicit template instantiation, an inline function
  that calls math.h's and D3DX's, and `#pragma optimize`,
  `inline_depth`, `function`, `intrinsic`, `pack` or `warning`
  compile nothing while the header is precompiled: they join the
  queue or leave it as it was.
- compiling the stage and the ending in one cl invocation, as the IDE
  batches sources, leaves the ending's order as it is alone.

C:\psdk2003 holds only libraries, and C:\msvc71 has no Platform SDK
headers, so C:\psdk is the only windows.h to try. th06 to th08 define
nothing but DIRECTINPUT_VERSION 0x0800 and include the same system
headers, and they are built with /Od, where the queue does not show.

## Open elsewhere

- Most of the breaks left in the large objects (player, GUI, app,
  main menu, sound, MIDI, sprite engine) are functions that wait for
  a header inline function in ours but not in the original: there the
  header's inline functions are compiled well before ours reach them.
  Neither reordering global.h nor its system headers or defines
  close that lead (see above): it differs from object to object, so
  it is code each object of the original compiled before its own
  functions and dropped.
- The MIDI object's UnloadFile, FreeTracks and ParseFile wait for the
  MemoryManager's inline functions, which the original had compiled
  by then; it also kept the one-byte `ret` that DebugPrint and the
  other empty functions fold into, at 0x43d2b0 between
  MidiTimer::StopTimer and MidiDevice::~MidiDevice.
- The archive object compiles archive.h's File::ReadDword,
  DatArchiveCursor's and DatArchiveEntry's inline functions after
  DuplicateString, its third function: too early for the header's
  progress, too late for a header outside it.
- Bullet::ClearCommands is at the top of the ECL object now that
  bullet.h is out of global.h; the original has it after RunEnemy.
  BulletManager::ClearDrawBuckets, at the top of the bullet object,
  wants bullet.h out, so the two may have been in different headers.
- The spell card object starts with Player::SetUnknown0, as from a
  header outside global.h, but taking player.h out loses neighbours
  elsewhere; Player::StartClearCircle1 to 3, which follow, call
  Timer::operator= and so wait in ours.
- App starts with Tan, tan and D3DXVec3Cross before
  Network::GetOtherUnknown1c, which nothing explains yet.
