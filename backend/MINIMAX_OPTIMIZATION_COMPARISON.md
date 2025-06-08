# Comparison between stages of the minimax algorithm optimization

## Minimax without optimizations

### RUN 1
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 108. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 1196. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 13900. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 172979. Time: 25 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 2306647. Time: 292 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 32478446. Time: 4050 ms

### RUN 2
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 108. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 1196. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 13900. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 172979. Time: 24 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 2306647. Time: 292 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 32478446. Time: 4109 ms

### RUN 3
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 108. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 1196. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 13900. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 172979. Time: 24 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 2306647. Time: 287 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 32478446. Time: 4062 ms

## Minimax with alpha-beta pruning

### RUN 1
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 74. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 673. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 6. Nodes: 4691. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 8. Nodes: 39284. Time: 13 ms
- Completed depth 6. Best score: -2240. Candidates: 5. Nodes: 296451. Time: 64 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 2806695. Time: 516 ms
- Completed depth 8. Best score: -2240. Candidates: 6. Nodes: 25200634. Time: 4967 ms

### RUN 2
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 74. Time: 1 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 673. Time: 2 ms
- Completed depth 4. Best score: -2240. Candidates: 6. Nodes: 4691. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 8. Nodes: 39284. Time: 12 ms
- Completed depth 6. Best score: -2240. Candidates: 5. Nodes: 296451. Time: 63 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 2806695. Time: 514 ms
- Completed depth 8. Best score: -2240. Candidates: 6. Nodes: 25200634. Time: 4959 ms

### RUN 3
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 74. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 673. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 6. Nodes: 4691. Time: 3 ms
- Completed depth 5. Best score: 0. Candidates: 8. Nodes: 39284. Time: 10 ms
- Completed depth 6. Best score: -2240. Candidates: 5. Nodes: 296451. Time: 62 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 2806695. Time: 514 ms
- Completed depth 8. Best score: -2240. Candidates: 6. Nodes: 25200634. Time: 4958 ms

## Minimax with alpha-beta pruning and sorted successors

### RUN 1
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 36. Time: 0 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 258. Time: 1 ms
- Completed depth 4. Best score: -2240. Candidates: 4. Nodes: 749. Time: 3 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3025. Time: 5 ms
- Completed depth 6. Best score: -2240. Candidates: 3. Nodes: 9605. Time: 8 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 20522. Time: 14 ms
- Completed depth 8. Best score: -2240. Candidates: 2. Nodes: 80915. Time: 40 ms
- Completed depth 9. Best score: 220. Candidates: 5. Nodes: 236862. Time: 116 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 802924. Time: 349 ms
- Completed depth 11. Best score: 0. Candidates: 7. Nodes: 2465327. Time: 1200 ms
- Completed depth 12. Best score: -2040. Candidates: 2. Nodes: 9223957. Time: 3997 ms

### RUN 2
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 36. Time: 2 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 258. Time: 3 ms
- Completed depth 4. Best score: -2240. Candidates: 4. Nodes: 749. Time: 4 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3025. Time: 6 ms
- Completed depth 6. Best score: -2240. Candidates: 3. Nodes: 9605. Time: 9 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 20522. Time: 15 ms
- Completed depth 8. Best score: -2240. Candidates: 2. Nodes: 80915. Time: 41 ms
- Completed depth 9. Best score: 220. Candidates: 5. Nodes: 236862. Time: 118 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 802924. Time: 352 ms
- Completed depth 11. Best score: 0. Candidates: 7. Nodes: 2465327. Time: 1206 ms
- Completed depth 12. Best score: -2040. Candidates: 2. Nodes: 9223957. Time: 4025 ms

### RUN 3
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 9. Time: 0 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 36. Time: 1 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 258. Time: 2 ms
- Completed depth 4. Best score: -2240. Candidates: 4. Nodes: 749. Time: 3 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3025. Time: 5 ms
- Completed depth 6. Best score: -2240. Candidates: 3. Nodes: 9605. Time: 8 ms
- Completed depth 7. Best score: 0. Candidates: 8. Nodes: 20522. Time: 15 ms
- Completed depth 8. Best score: -2240. Candidates: 2. Nodes: 80915. Time: 40 ms
- Completed depth 9. Best score: 220. Candidates: 5. Nodes: 236862. Time: 119 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 802924. Time: 362 ms
- Completed depth 11. Best score: 0. Candidates: 7. Nodes: 2465327. Time: 1237 ms
- Completed depth 12. Best score: -2040. Candidates: 2. Nodes: 9223957. Time: 4057 ms

## Minimax with alpha-beta pruning, sorted successors, and transposition table

### RUN 1
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 27 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 29 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 346. TT Size: 74. Time: 30 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 1032. TT Size: 238. Time: 31 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3544. TT Size: 784. Time: 34 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9741. TT Size: 1996. Time: 37 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 23158. TT Size: 5013. Time: 45 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 66422. TT Size: 12077. Time: 64 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 180564. TT Size: 36082. Time: 122 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 572363. TT Size: 93404. Time: 287 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1694090. TT Size: 322839. Time: 887 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 4679953. TT Size: 721642. Time: 2157 ms

### RUN 2
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 30 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 32 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 345. TT Size: 74. Time: 33 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 1029. TT Size: 237. Time: 34 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3536. TT Size: 783. Time: 37 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9722. TT Size: 1991. Time: 41 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 23126. TT Size: 5006. Time: 48 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 66311. TT Size: 12064. Time: 67 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 180150. TT Size: 36011. Time: 125 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 573041. TT Size: 93699. Time: 290 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1734328. TT Size: 333585. Time: 921 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 4746193. TT Size: 736908. Time: 2222 ms

### RUN 3
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 33 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 35 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 344. TT Size: 74. Time: 36 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 1026. TT Size: 236. Time: 38 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3606. TT Size: 809. Time: 40 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9791. TT Size: 2017. Time: 45 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 23527. TT Size: 5161. Time: 59 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 66707. TT Size: 12216. Time: 79 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 180034. TT Size: 36068. Time: 140 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 569434. TT Size: 92908. Time: 339 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1683687. TT Size: 321208. Time: 1231 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 4669328. TT Size: 720045. Time: 2535 ms

## Minimax with alpha-beta pruning, sorted successors, transposition table, and pvs (principal variation search)

### RUN 1
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 27 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 29 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 300. TT Size: 74. Time: 30 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 986. TT Size: 227. Time: 32 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3135. TT Size: 781. Time: 34 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9174. TT Size: 1947. Time: 38 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 21054. TT Size: 4983. Time: 45 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 60061. TT Size: 11720. Time: 63 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 160070. TT Size: 35752. Time: 118 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 489933. TT Size: 89041. Time: 263 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1182136. TT Size: 266026. Time: 713 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 3686315. TT Size: 673446. Time: 1869 ms

### RUN 2
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 30 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 32 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 302. TT Size: 74. Time: 34 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 992. TT Size: 229. Time: 36 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3105. TT Size: 756. Time: 38 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9146. TT Size: 1922. Time: 42 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 20707. TT Size: 4829. Time: 50 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 59757. TT Size: 11580. Time: 70 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 159952. TT Size: 35671. Time: 130 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 494795. TT Size: 90457. Time: 288 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1201464. TT Size: 269632. Time: 735 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 3716888. TT Size: 677925. Time: 1900 ms

### RUN 3
- Completed depth 1. Best score: 999320. Candidates: 2. Nodes: 10. TT Size: 0. Time: 34 ms
- Completed depth 2. Best score: -2720. Candidates: 2. Nodes: 48. TT Size: 10. Time: 37 ms
- Completed depth 3. Best score: 0. Candidates: 4. Nodes: 302. TT Size: 74. Time: 39 ms
- Completed depth 4. Best score: -2240. Candidates: 2. Nodes: 992. TT Size: 229. Time: 41 ms
- Completed depth 5. Best score: 0. Candidates: 4. Nodes: 3105. TT Size: 756. Time: 43 ms
- Completed depth 6. Best score: -2240. Candidates: 1. Nodes: 9146. TT Size: 1922. Time: 46 ms
- Completed depth 7. Best score: 0. Candidates: 4. Nodes: 20707. TT Size: 4829. Time: 55 ms
- Completed depth 8. Best score: -2240. Candidates: 1. Nodes: 59757. TT Size: 11580. Time: 72 ms
- Completed depth 9. Best score: 220. Candidates: 1. Nodes: 159952. TT Size: 35671. Time: 128 ms
- Completed depth 10. Best score: -2240. Candidates: 2. Nodes: 494795. TT Size: 90457. Time: 278 ms
- Completed depth 11. Best score: 0. Candidates: 2. Nodes: 1201464. TT Size: 269632. Time: 725 ms
- Completed depth 12. Best score: -2040. Candidates: 1. Nodes: 3716888. TT Size: 677925. Time: 1876 ms