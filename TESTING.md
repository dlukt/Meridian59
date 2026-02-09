# Testing Guide for class_ptr Optimization

This document describes how to validate the `class_ptr` caching optimization in the Meridian 59 server.

## What is the Optimization?

The PR adds a `class_node *class_ptr` field to the `object_node` structure. This caches the pointer to an object's class, eliminating repeated hash table lookups in `RetrieveValue` when accessing class variables (`CLASS_VAR`).

### Files Changed
- `blakserv/object.h` - Added `class_ptr` field to `object_node` struct
- `blakserv/object.c` - Set `class_ptr` during object allocation and moves
- `blakserv/sendmsg.h` - Updated inline `RetrieveValue` implementation to use cached `class_ptr`

## Automated Tests

### Unit Tests

Run the unit test suite to verify structural correctness:

```bash
make -C tests test
```

This runs:
1. **Existing utility tests** - CRC, MD5, time functions (7 tests)
2. **class_ptr optimization tests** - Structural validation (4 tests)

The class_ptr tests verify:
- ✅ The `class_ptr` field exists in `object_node` struct
- ✅ The field is properly positioned in memory layout
- ✅ `RetrieveValue` has the correct function signature
- ✅ The struct size remains reasonable (no excessive bloat)

### Server Compilation

Verify the server builds without errors:

```bash
cd blakserv
make -f makefile.linux
```

The server must compile cleanly with `-Werror` (warnings treated as errors).

## Functional Validation

The automated tests verify structural correctness, but full validation requires running the server with Blakod scripts.

### Manual Testing Steps

1. **Build the server**
   ```bash
   cd blakserv
   make -f makefile.linux
   ```

2. **Run the server**
   ```bash
   cd ../run/server
   ./blakserv
   ```

3. **Execute Blakod scripts that access class variables**
   - Log in to the server
   - Perform actions that trigger Blakod execution
   - Verify class variable reads work correctly
   - Check for any error messages in server logs

4. **Monitor for issues**
   - Watch for segmentation faults
   - Check for incorrect class variable values
   - Verify objects are created and destroyed correctly

### What to Test

Focus on operations that access class variables:
- **Object creation** - Creates objects with `class_ptr` initialized
- **Class variable reads** - Uses the cached pointer in `RetrieveValue`
- **Object moves** - Preserves `class_ptr` during moves
- **Multiple classes** - Ensures different objects cache correct class pointers

## Continuous Integration

The GitHub Actions workflow automatically runs tests on every commit:

```bash
.github/workflows/unit-tests.yml
```

This ensures:
- ✅ Code compiles successfully
- ✅ All unit tests pass
- ✅ No regressions are introduced

## Performance Validation

To measure the performance impact:

1. **Before/After Comparison**
   - Check out the commit before this PR
   - Build and run server, note CPU usage during heavy Blakod execution
   - Check out this PR's commit
   - Build and run server, compare CPU usage

2. **Profiling**
   - Use `perf` or `gprof` to profile the server
   - Look for reduced time in `GetClassByID` calls
   - Verify `RetrieveValue` spends less time on hash lookups

## Expected Results

✅ **All automated tests pass**  
✅ **Server compiles without warnings**  
✅ **Server runs without crashes**  
✅ **Class variables are accessed correctly**  
✅ **Small reduction in CPU usage for script-heavy workloads**

## Troubleshooting

### Test Failures

If `test_class_ptr_optimization` fails:
- Check that `class_ptr` field is present in `object_node`
- Verify `object.c` sets `class_ptr` in `AllocateObject` and `MoveObject`
- Ensure `sendmsg.h` uses `class_ptr` in `RetrieveValue` for `CLASS_VAR` case

### Runtime Crashes

If the server crashes:
- Check for NULL `class_ptr` dereferences
- Verify `class_ptr` is set before any object is used
- Ensure `MoveObject` correctly copies `class_ptr`

### Incorrect Values

If class variables have wrong values:
- Verify `class_ptr` points to the correct class
- Check that `class_ptr` matches `class_id`
- Ensure no stale pointers after class reloads (if supported)

## Safety

The optimization is safe because:
1. **class_ptr is set at allocation time** - Every object gets a valid pointer
2. **class_ptr matches class_id** - Consistency is maintained
3. **class_ptr is preserved during moves** - Object identity is maintained
4. **Classes are not deallocated during runtime** - Pointers remain valid

The cached pointer is valid for the lifetime of the object, as class definitions
are loaded at startup and not modified or freed during normal operation.
