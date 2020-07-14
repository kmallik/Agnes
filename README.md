# Agnes

Agnes is a tool for solving the distributed reactive synthesis problem. While the problem at hand is undecidable in general, Agnes promises to provide a sound but incomplete solution. That is, whenever Agnes produces a solution the solution is valid. The core underlying theory of Agnes can be found in our EMSOFT '20 paper.

Right now, Agnes is in its nascent stage: It can only support very simple architecture (two systems connected in feedback) with a restriction on the class of specifications (safety and deterministic Büchi); the class of solutions is also restricted to only safe contracts (a heuristics that works well for some examples).
