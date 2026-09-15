# Working on Pawapuro

Read [docs/README.md](docs/README.md), then the relevant
[design](docs/design/batting-feel.md) and [milestone](docs/milestones/01-batting-feel.md).
The [Jai review](docs/research/jai-design-adoption-review.md) records research and
trade-offs; it is not a mandate to implement every discussed feature.

## Scope and workflow

- Character animation work must read [Pawapuro Character Motion Rules](docs/design/character-motion.md) and establish support/contact, body intent, lead/lag, attachment, and momentum resolution before full motion authoring.
- Do only the explicitly approved task. Do not start the next milestone step implicitly.
- If a task reveals a larger architectural decision, stop and report it rather than silently expanding scope.
- Prefer small, reviewable changes. Verify the affected behavior and review the diff; distinguish observed results from assumptions.
- Create or update relevant research, design and milestone Markdown under `docs/`. Keep each decision in its owning document rather than duplicating architecture.
- Never force push or rewrite Git history.
- Never inspect, print, export, recover, expose or modify stored credentials. Use the configured Git credential helper for authorized normal Git operations.

## Documentation language

- Write design, architecture, research, milestone and development documents under `docs/` in Traditional Chinese using Taiwan usage, unless explicitly requested otherwise.
- Keep technical proper nouns and established terms in English when clearer or more natural (for example Direct3D 12, C++, fixed timestep, hot reload, replay, allocator, Lua and SDL3).
- Source-code identifiers, APIs, filenames/directories, commit messages and code comments should normally use English. Keep `AGENTS.md` itself in English.
- Maintain one source of truth; do not duplicate internal documents in Chinese and English without a real user/contributor need.
- A future top-level public README may be bilingual; this convention does not request its creation.

## Engineering rules

- Real Pawapuro requirements drive Engine capabilities. Baseball semantics remain in Pawapuro until an operation has demonstrated general value; mathematical code is not automatically Engine code.
- Before adding an abstraction, state the current duplication, error source, ownership problem or real requirement it solves. Prefer concrete structs, ordinary functions, explicit ownership and direct data flow.
- Avoid speculative ECS, SOA frameworks, generic job systems, service locators, reflection systems, factory/interface hierarchies, plugin architectures and migration layers.
- Decide ownership, borrowing duration and release conditions before selecting allocators or containers. CPU frame, background task and GPU fence lifetimes differ.
- Data-oriented design follows measured access patterns; it does not imply ECS or SOA by default.
- Keep the Batting Feel boundaries: Engine Native provides demonstrated general operations; Pawapuro Native owns baseball simulation. Deterministic per-tick simulation belongs in Native, frequently tuned values in Data, and higher-level sequencing/scenario coordination in Lua. Lua must not become a hidden second physics/game engine.
- Preserve concept locality: related Native code, script, data and short documentation live near the same game concept. Create only the files actually needed.
- Optimize the entire edit → observe → replay loop, not merely compile time.
- Hot reload validates a candidate before publishing at an explicit safe boundary. Failure keeps the previous working version; retire resources only after their users, including the GPU, finish.
- Keep simulation replayable and inspectable: record inputs, ticks, parameters and relevant versions; expose Native owner, script and data provenance. Keep nondeterministic inference outside the mandatory simulation tick. State the supported replay boundary rather than promising cross-platform bit identity.
- Do not recreate Jai compiler features in C++20 through elaborate templates, macros, reflection or compiler tooling. Keep persistent formats clear; introduce C ABI boundaries only where a current need warrants them.
- Comments primarily explain constraints, reasons and trade-offs rather than restating code. Support performance and architectural claims with reproducible evidence.
- AI's ability to generate large amounts of code never justifies architectural complexity.
