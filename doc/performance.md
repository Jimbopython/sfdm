# Performance

| Library                        | Timeout | Mean   | Std. Dev. |
|--------------------------------|---------|--------|-----------|
| ZXingCodeReader                | 0ms     | 1.92ms | 2.57ms    |
| LibdmtxCodeReader              | 0ms     | 302ms  | 874ms     |
| LibdmtxCodeReader              | 100ms   | 37ms   | 49ms      |
| LibdmtxCodeReader              | 200ms   | 55ms   | 83 ms     |
| LibdmtxZXingCombinedCodeReader | 0ms     | 300ms  | 860ms     |
| LibdmtxZXingCombinedCodeReader | 100ms   | 38ms   | 49ms      |
| LibdmtxZXingCombinedCodeReader | 200ms   | 56ms   | 83ms      |