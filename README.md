## What's this?

A small and fast JSON producer written in C.

## Goals

* Performance
  * No implicit heap allocations
* Correctness
* Clean code
* Small footprint
  * Be able to produce json larger than available memory
* Portability

## Non-goals

* Stable API/ABI
* Parser
* Validator
* DOM 

## benchmark

![Graph](./bench/result.png)

* It measures the speed to generate JSON like [this](bench/example.json).

* Larger is better.

* Taken on a macOS/amd64 laptop:

  ```
  MacBook Pro (15-inch, 2018)
  2.2 GHz 6-Core Intel Core i7
  ```

* [Raw values](./bench/result.csv)

### Notes

* It ended up with benchmarking the conversion from binary numbers to
  strings. (itoa/dtoa)
  `jsonsink+snprintf`, `parson`, and `snprintf` use libc snprintf for
  the conversion.
  `jsonsink+jnum`, `rapidjson`, and `ljson` use more performant implementations
  of the conversion.
  `flatbuffers` is fast mainly because it doesn't involve the conversions.

* `jsonsink (malloc)` is expected to be about twice slower than
  `jsonsink (static buffer)` because it calculates the necessay buffer size 
  with a dry-run.

* `flatbuffers` is not a fair comparison because it isn't a JSON producer.
  i included it just as a base line.

* `parson` is not a fair comparison because it uses a DOM-based api.

* `snprintf` is cheating a bit by using the apriori knowledge of
  the necessary buffer size.
