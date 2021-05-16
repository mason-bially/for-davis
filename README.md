On linux (or probably any computer) with bazel installed, run:

```
bazel run src/least-binary-number:A056744 -- 78 --from 2
```

Change 78 to whatever number you want. Add `--debug` or `--trace` to see detailed information (you should probably remove `--from` and choose the run number you are interested in when doing this).