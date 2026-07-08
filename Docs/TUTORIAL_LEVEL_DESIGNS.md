# Chess Cadets — Tutorial Level Designs (Story Mode)

Source of truth for the 6 tutorial levels. Provided by Isabella (UX/UI Lead).

## LEVEL 1: PAWN — "Street Dash"
*Inspired by: Pawn Wars (classic chess teaching game) + Magnus Kingdom corridors*

Small grid (4 columns wide). **Marco** guides you.
- **Phase 1:** Empty path. Just move forward one square at a time to reach the end. "Pawns only go forward!"
- **Phase 2:** First move lets you go two squares. "On your first step, you can sprint!"
- **Phase 3:** Enemy pawns block your path head-on. You're stuck — diagonal capture arrows glow. Click diagonally to capture and keep going. "You can't go through them, but you can go AROUND them!"
- **Boss: PAWN RACE** — You and an AI pawn on parallel lanes, both heading to the other side. First to reach the end wins. If they're next to you, capture diagonally to remove them.

**One thing taught:** Pawns go forward, capture diagonally.

## LEVEL 2: ROOK — "Laser Grid"
*Inspired by: Rook Road + Capture the Flag (from chess teaching curricula) + Fritz delivery game*

Top-down grid. **Tara** guides you. Energy orbs scattered across rows and columns.
- **Phase 1:** Orbs in a single row. Slide left and right to collect. "Rooks slide in straight lines!"
- **Phase 2:** Orbs in rows AND columns. Switch between horizontal and vertical. "Up, down, left, right — as far as you want!"
- **Phase 3:** Wall obstacles appear. You can't slide through them. Find a path around. "Rooks can't go through walls — find another way!"
- **Boss: CATCH THE RUNNER** — A fleeing target moves one square per turn. Chase it using only straight-line slides. Catch it within the move limit.

**One thing taught:** Rooks slide unlimited squares in straight lines, can't pass through things.

## LEVEL 3: BISHOP — "Color Lock"
*Inspired by: Bishop Colour Path + Magnus Kingdom's diagonal-only terrain + Fritz painting game*

Grid board. Half the squares are neon green, half are neon purple. **Zig** guides you.
- **Phase 1:** You're on green. Targets only on green squares. Move diagonally to collect. The purple squares are visually darkened — you literally can't go there. "Bishops only move diagonally!"
- **Phase 2:** Targets appear on purple squares. Zig says "I can't reach those — they're not my color!" Kid sees the limitation naturally without a lecture.
- **Phase 3:** Obstacles on your diagonals. Plan a longer diagonal route around them. "Find another diagonal!"
- **Boss: PAINT THE BOARD** — Cover as many green squares as possible in a set number of moves. Shows how bishops dominate diagonals.

**One thing taught:** Bishops move diagonally, stuck on one color.

## LEVEL 4: KNIGHT — "Rooftop Hop"
*Inspired by: Hungry Knight / Treasure Knight (most popular knight teaching game across all platforms) + Dinosaur Chess mice game + Fritz frog-on-lily-pads*

Rooftops from above. **Luna** guides you. Each rooftop is a grid square.
- **Phase 1:** Only 2-3 rooftops light up in the L-shape from where you stand. Stars sit on some of them. Hop to collect. "Knights move in an L — two squares one way, one square to the side!"
- **Phase 2:** Obstacles (walls, gaps) sit between you and the stars. But you jump right over them. "Knights can JUMP! Nothing can block you!"
- **Phase 3:** Stars form a trail. Follow the trail using L-hops — like connect-the-dots with knight moves. More stars, trickier routing.
- **Boss: TAG** — Luna hops around the board. Chase her and land on her square using L-moves within a move limit. She's predictable but fast.

**One thing taught:** Knights move in L-shapes, can jump over anything.

## LEVEL 5: QUEEN — "Power Grid"
*Inspired by: Queen vs 8 Pawns + Magnus Kingdom's "unlock all abilities" moment*

Open grid arena. **Nova** guides you.
- **Phase 1:** Targets in a row. "Move like Tara's rook!" Slide straight to collect.
- **Phase 2:** Targets on diagonals. "Move like Zig's bishop!" Slide diagonal to collect.
- **Phase 3:** Targets EVERYWHERE — rows, columns, diagonals, all mixed. "Now use BOTH! You're unstoppable!" The kid feels the power click.
- **Boss: CLEAR THE BOARD** — 10 targets scattered across the board. Collect them all. Par score displayed (e.g., "Can you do it in 6 moves?"). Shows how dominant the queen is.

**One thing taught:** Queen = rook + bishop combined. Most powerful piece.

## LEVEL 6: KING + FINAL BATTLE — "The Core"
*Inspired by: King Maze + Magnus Kingdom's "rabbit chase" for checkmate*

- **Phase 1: KING MAZE** — Small grid. Danger squares glow red (enemy pieces threaten them). Move one square in any direction to reach the exit. "The king can go anywhere — but only one step at a time. And stay safe!"
- **Phase 2: RECAP** — Each friend appears briefly. Quick 10-second refresher per piece. "Remember how I move?" They demo their movement one more time.
- **Boss: AXIOM** — Full chess match. All friends become your pieces. AI is on easy (makes intentional mistakes). The goal is for the kid to WIN and feel like a hero. Cutscene when they checkmate — friends celebrate, AXIOM defeated, Neo City is free.

**One thing taught:** King moves one square any direction + everything comes together.

## KEY DESIGN RULES (proven by research)
- **Isolation** — One piece per level, no mixing
- **Collection mechanic** — Scatter targets, navigate to collect. Works for every piece.
- **No fail state** — Wrong move? "Try again!" No game over screen. Kids experiment freely.
- **3 phases** — Simple → Add a twist → "I get it!" moment
- **Show, don't tell** — Glowing squares show legal moves. Darkened squares show where you CAN'T go. Minimal text.
- **Feel the power** — Each piece should feel special. The knight jumps over stuff! The queen goes everywhere! Make each one exciting.
