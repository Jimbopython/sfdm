# Detection Results on a specified dataset

The dataset the following plots were created with is
from [this](https://www.mdpi.com/2313-433X/7/9/163)
article. The green bars are "successes", which means "codes detected". The red bars, some of which
are on top of green bars, are "failures", which means "codes that were not detected". As a result,
the total codes within an image are green + red.

## Overall results

On the X Axis, one can see the different code reader implementations of the sfdm library. On the Y
Axis, one can see the total found/not found codes of the specific implementation.

![](plots/Overall.png)

There are 111 codes to detect.

| Library  | Detected | Timeout | Percentage |
|----------|----------|---------|------------|
| ZXing    | 70       | 0ms     | 63%        |
| Libdmtx  | 80       | 100ms   | 72%        |
| Libdmtx  | 82       | 200ms   | 73%        |
| Libdmtx  | 85       | 0ms     | 76%        |
| Combined | 95       | 0ms     | 86%        |

## Result for each code

On the X Axis, one can see the different code filenames. On the Y Axis, one can see the number of
detected codes.

## ZXing detection on all codes

![](plots/ZXing_None.png)

## Libdmtx detection on all codes

![](plots/LibDMTX_100ms.png)

![](plots/LibDMTX_200ms.png)

![](plots/LibDMTX_0ms.png)

## Combined detection on all codes

![](plots/Combined_None.png)