# Test Plan: OTA Module & DataLogger

> Module Under Test: XModemTransfer, YModemTransfer, ZModemTransfer, OtaManager, OtaHistoryModel, DataLogger
> Version: 0.1.0
> Author: QA Engineer
> Date: 2026-05-31

---

## 1. Test Environment & Infrastructure

### 1.1 Test Framework

| Item | Choice | Reason |
|------|--------|--------|
| Unit Test Framework | Qt Test (`QTest`) | Native Qt signal/slot support, QSignalSpy for async verification |
| Mock Object | Custom `MockConnection` implementing `IConnection` | Full control over received data injection and write capture |
| Build Integration | CMake `add_test()` via `enable_testing()` | CI/CD compatible, `ctest` runner |

### 1.2 MockConnection Specification

`MockConnection` is the core test infrastructure. It must:

- Implement `IConnection` interface
- Capture all `write()` calls into a `QList<QByteArray>` for assertion
- Provide `simulateReceive(const QByteArray&)` to inject data into `dataReceived` signal
- Return predefined `ConnectionState`, `ConnectionType`, and `name()`
- `open()` / `close()` / `configure()` are no-ops that return success

### 1.3 Test Helpers

| Helper | Purpose |
|--------|---------|
| `buildXmodemBlock(int blockNum, const QByteArray& data, XModemTransfer::Mode mode)` | Construct a valid XMODEM block for receiver simulation |
| `buildXmodemCrc(const QByteArray& data)` | CRC16-CCITT matching `CRC::crc16Ccitt` |
| `createTempFile(int sizeBytes, const QString& name)` | Generate temp binary files of arbitrary size |
| `waitForSignal(QObject* obj, const char* signal, int timeoutMs)` | Block until signal emitted or timeout |

### 1.4 Risk-Based Priority

| Priority | Modules | Reason |
|----------|---------|--------|
| P0 (Critical) | XModemTransfer normal flow, DataLogger format integrity | Core functionality, any failure blocks user's primary task |
| P1 (High) | YModemTransfer batch, ZModemTransfer handshake, OtaHistoryModel persistence | Multi-step protocols with complex state machines |
| P2 (Medium) | Timeout/retry, mode degradation, playback speed | Error recovery paths, less frequently exercised |
| P3 (Low) | Empty file, corrupt input, max boundary | Defensive checks, rare in normal operation |

---

## 2. XMODEM Test Cases

### 2.1 Normal Transfer - Checksum Mode

| Field | Value |
|-------|-------|
| **ID** | XM-001 |
| **Priority** | P0 |
| **Feature** | XMODEM-Checksum full transfer |

**Preconditions:**
- `MockConnection` is connected to `XModemTransfer`
- Mode is set to `XModemTransfer::Checksum`
- Temp file containing 300 bytes of known data is prepared

**Steps:**
1. Call `setFilePath()` with the temp file path
2. Call `start()`, verify return value is `true`
3. Verify state is `WaitingForStart` (indirectly: no data written yet)
4. Simulate receiver sending `NAK` (0x15)
5. Verify `MockConnection.write()` captured: `SOH + blockNum(1) + ~blockNum + 128B data + checksum`
6. Simulate receiver sending `ACK` (0x06)
7. Verify second block is written (blockNum=2, next 128 bytes + 0x1A padding to 128)
8. Simulate receiver sending `ACK` again
9. Verify EOT (0x04) is written (no more data blocks needed; 300 bytes = 3 blocks, block 3 has 44 bytes + 84 bytes padding)
10. Simulate receiver sending `ACK`

**Expected Results:**
- `transferComplete()` signal emitted
- `progress(100, 300, 300)` signal emitted
- `isRunning()` returns `false`
- Total 3 data blocks sent + 1 EOT
- Block 3 padding bytes are 0x1A, checksum matches `CRC::checksum(blockData)`
- Block numbers cycle: 1, 2, 3

---

### 2.2 Normal Transfer - CRC Mode

| Field | Value |
|-------|-------|
| **ID** | XM-002 |
| **Priority** | P0 |
| **Feature** | XMODEM-CRC full transfer |

**Preconditions:**
- Mode is `XModemTransfer::CRC`
- Temp file with 256 bytes of data prepared

**Steps:**
1. Call `start()`
2. Simulate receiver sending `'C'` (0x43)
3. Verify first block written: `SOH + blockNum(1) + ~blockNum + 128B data + CRC16_hi + CRC16_lo`
4. Simulate `ACK`, verify second block sent
5. Simulate `ACK`, verify EOT sent
6. Simulate `ACK`

**Expected Results:**
- `transferComplete()` emitted
- CRC16 bytes match `CRC::crc16Ccitt(blockData)`
- 2 data blocks + 1 EOT total

---

### 2.3 Normal Transfer - 1K Mode

| Field | Value |
|-------|-------|
| **ID** | XM-003 |
| **Priority** | P0 |
| **Feature** | XMODEM-1K full transfer |

**Preconditions:**
- Mode is `XModemTransfer::OneK`
- Temp file with 1500 bytes of data prepared

**Steps:**
1. Call `start()`
2. Simulate receiver sending `'C'`
3. Verify first block starts with `STX` (0x02) instead of `SOH`
4. Verify block payload is 1024 bytes + CRC16 (2 bytes)
5. Simulate `ACK`
6. Verify second block: `STX`, 476 bytes data + 548 bytes 0x1A padding + CRC16
7. Simulate `ACK`
8. Verify EOT
9. Simulate `ACK`

**Expected Results:**
- `transferComplete()` emitted
- Block header is `STX` not `SOH`
- Total 2 data blocks (1024 + 476 padded to 1024)
- CRC16 appended to each block

---

### 2.4 Receiver Timeout Retransmission

| Field | Value |
|-------|-------|
| **ID** | XM-004 |
| **Priority** | P2 |
| **Feature** | Timeout-triggered retransmission |

**Preconditions:**
- Mode is `CRC`, file with 128 bytes prepared
- Timer is mocked or accelerated for test

**Steps:**
1. Call `start()`
2. Simulate receiver sending `'C'` to start transfer
3. Verify first block is written
4. Do NOT send `ACK` -- let the 5-second timeout fire
5. Verify same block (blockNum=1) is retransmitted
6. Send `ACK`
7. Verify EOT sent
8. Send `ACK`

**Expected Results:**
- After timeout, the same block data is re-sent (identical payload)
- `retryCount` increments internally but transfer continues
- After `ACK`, transfer proceeds normally to completion
- `transferComplete()` emitted eventually

---

### 2.5 Receiver NAK Retransmission

| Field | Value |
|-------|-------|
| **ID** | XM-005 |
| **Priority** | P1 |
| **Feature** | NAK-triggered retransmission of current block |

**Preconditions:**
- Mode is `CRC`, file with 256 bytes prepared

**Steps:**
1. Call `start()`, simulate `'C'` to start
2. First block is written
3. Simulate receiver sending `NAK` (0x15) instead of `ACK`
4. Verify the same block (blockNum=1) is retransmitted
5. Simulate `ACK`
6. Verify next block (blockNum=2) is sent
7. Simulate `ACK`, verify EOT, simulate `ACK`

**Expected Results:**
- Block 1 is retransmitted after NAK (identical data)
- Block number does NOT advance on NAK
- Transfer completes successfully

---

### 2.6 NAK Retry Exhaustion

| Field | Value |
|-------|-------|
| **ID** | XM-006 |
| **Priority** | P2 |
| **Feature** | Max NAK retries exceeded |

**Preconditions:**
- Mode is `CRC`, file with 128 bytes prepared

**Steps:**
1. Call `start()`, simulate `'C'`
2. First block is written
3. Simulate sending `NAK` 11 times (exceeds `kMaxRetries = 10`)

**Expected Results:**
- After the 10th NAK retry, `transferError("Too many NAK retries")` is emitted
- Two `CAN` (0x18) bytes are written to the connection
- `isRunning()` returns `false`

---

### 2.7 Receiver Cancel (CAN)

| Field | Value |
|-------|-------|
| **ID** | XM-007 |
| **Priority** | P1 |
| **Feature** | Receiver-initiated cancellation |

**Preconditions:**
- Mode is `CRC`, file with 256 bytes prepared

**Steps:**
1. Call `start()`, simulate `'C'`
2. First block is written
3. Simulate receiver sending `CAN` (0x18)

**Expected Results:**
- `transferError("Transfer cancelled by receiver")` is emitted immediately
- No further data blocks are written
- `isRunning()` returns `false`

---

### 2.8 Mode Degradation (CRC to Checksum)

| Field | Value |
|-------|-------|
| **ID** | XM-008 |
| **Priority** | P2 |
| **Feature** | Automatic fallback from CRC to Checksum mode |

**Preconditions:**
- Mode is set to `XModemTransfer::CRC`
- File with 128 bytes prepared

**Steps:**
1. Call `start()`
2. Simulate receiver sending `NAK` (0x15) instead of `'C'`
3. Verify transfer starts using Checksum mode:
   - Block ends with 1-byte Sum8 checksum instead of 2-byte CRC16
4. Simulate `ACK`, verify EOT, simulate `ACK`

**Expected Results:**
- Mode is silently downgraded to `Checksum`
- Block trailer is 1 byte (`CRC::checksum(blockData)`) not 2 bytes
- Transfer completes successfully
- `transferComplete()` emitted

---

### 2.9 Empty File / Empty Data

| Field | Value |
|-------|-------|
| **ID** | XM-009 |
| **Priority** | P3 |
| **Feature** | Rejection of empty data |

**Preconditions:**
- `setData()` called with empty `QByteArray` and no file path set

**Steps:**
1. Call `start()`

**Expected Results:**
- `start()` returns `false`
- `transferError("No data to transfer")` is emitted
- No data written to connection

---

### 2.10 Large File Boundary (Block Number Rollover)

| Field | Value |
|-------|-------|
| **ID** | XM-010 |
| **Priority** | P3 |
| **Feature** | Block number 1-255 cycling |

**Preconditions:**
- Mode is `OneK`
- Temp file with 256 * 1024 + 512 bytes (exceeds 255 blocks)

**Steps:**
1. Call `start()`, simulate `'C'`
2. Simulate `ACK` for 255 blocks, verifying blockNum goes from 1 to 255
3. Verify block 256's blockNum wraps to 0 (255 + 1 = 256, `256 & 0xFF = 0`)
4. Continue to completion

**Expected Results:**
- Block numbers follow: 1, 2, ..., 255, 0, 1, 2, ...
- Each block's complement byte is `~blockNum & 0xFF`
- Transfer completes without block number errors

---

### 2.11 Mid-Transfer User Cancel

| Field | Value |
|-------|-------|
| **ID** | XM-011 |
| **Priority** | P1 |
| **Feature** | Sender-initiated cancellation during transfer |

**Preconditions:**
- Mode is `CRC`, file with 512 bytes prepared

**Steps:**
1. Call `start()`, simulate `'C'`
2. First block is written, simulate `ACK`
3. Second block is written
4. Call `cancel()`

**Expected Results:**
- Two `CAN` (0x18) bytes are written to the connection
- `transferError("Transfer cancelled by user")` is emitted
- `isRunning()` returns `false`
- Timer is stopped

---

### 2.12 Startup Timeout (No Receiver Response)

| Field | Value |
|-------|-------|
| **ID** | XM-012 |
| **Priority** | P2 |
| **Feature** | Timeout when receiver never sends NAK or 'C' |

**Preconditions:**
- Mode is `CRC`, file with 128 bytes prepared
- Timer accelerated for test

**Steps:**
1. Call `start()`
2. Do not send any data from receiver
3. Wait for startup timeout (`kTimeoutMs * 3` = 15 seconds, accelerated)
4. Repeat 10 times (exceed `kMaxRetries`)

**Expected Results:**
- After 10 retry timeouts, `transferError("Transfer timeout: max retries exceeded")` is emitted
- Two `CAN` bytes written

---

### 2.13 EOT NAK Retry

| Field | Value |
|-------|-------|
| **ID** | XM-013 |
| **Priority** | P2 |
| **Feature** | EOT retransmission on NAK |

**Preconditions:**
- Mode is `CRC`, file with 128 bytes prepared

**Steps:**
1. Call `start()`, simulate `'C'`
2. First block sent, simulate `ACK`
3. EOT is sent
4. Simulate receiver sending `NAK` instead of `ACK`
5. Verify EOT is retransmitted
6. Simulate `ACK`

**Expected Results:**
- EOT is retransmitted after NAK
- `transferComplete()` emitted after final ACK

---

### 2.14 Start Returns False When Already Running

| Field | Value |
|-------|-------|
| **ID** | XM-014 |
| **Priority** | P3 |
| **Feature** | Duplicate start prevention |

**Preconditions:**
- Transfer is already in progress (state != Idle)

**Steps:**
1. Start a transfer (call `start()`, simulate `'C'`)
2. Call `start()` again without completing the first transfer

**Expected Results:**
- Second `start()` returns `false`
- First transfer is not affected

---

### 2.15 setData Overrides FilePath

| Field | Value |
|-------|-------|
| **ID** | XM-015 |
| **Priority** | P3 |
| **Feature** | Direct data input bypasses file read |

**Preconditions:**
- `setFilePath()` called with a valid path
- `setData()` called afterward with non-empty data

**Steps:**
1. Call `setFilePath("some_file.bin")`
2. Call `setData(QByteArray("test data"))`
3. Call `start()`, simulate `'C'`

**Expected Results:**
- Transfer uses the data from `setData()`, not from the file
- `m_filePath` is cleared (per implementation: `m_filePath.clear()`)
- Transfer proceeds normally

---

## 3. YMODEM Test Cases

### 3.1 Block 0 File Information Correctness

| Field | Value |
|-------|-------|
| **ID** | YM-001 |
| **Priority** | P0 |
| **Feature** | YMODEM Block 0 content validation |

**Preconditions:**
- `MockConnection` connected to `YModemTransfer`
- File "firmware_v2.bin" with 10240 bytes prepared

**Steps:**
1. Call `setFilePath()` with the file
2. Call `start()`
3. Simulate receiver sending `'C'` (0x43)
4. Capture the written Block 0 data

**Expected Results:**
- Block 0 structure: `SOH + 0x00 + 0xFF + 128B payload + CRC16_hi + CRC16_lo`
- Payload starts with: `"firmware_v2.bin\0" + "10240\0" + "0\0" + "0\0"` + zero padding to 128 bytes
- Block number is 0, complement is 0xFF
- CRC16 matches `CRC::crc16Ccitt(payload)`

---

### 3.2 Single File Transfer Complete Flow

| Field | Value |
|-------|-------|
| **ID** | YM-002 |
| **Priority** | P0 |
| **Feature** | YMODEM single file end-to-end |

**Preconditions:**
- File with 200 bytes prepared

**Steps:**
1. Call `setFilePath()`, `start()`
2. Simulate `'C'` -- Block 0 is sent
3. Simulate `ACK` -- sender now expects `'C'` for data phase
4. (Sender transitions to `SendingData` with `blockNumber=1`)
5. Simulate `'C'` or the sender auto-proceeds (verify implementation: after Block0 ACK, it waits for `'C'` in `SendingData` state -- but actually the state goes to `SendingData` and immediately sends block 1)
6. Simulate `ACK` for block 1 (128 bytes)
7. Block 2 is sent (72 bytes + 56 bytes 0x1A padding)
8. Simulate `ACK`
9. EOT is sent
10. Simulate `ACK`
11. (All files done) sender transitions to `WaitFinalC`
12. Simulate `'C'`
13. Empty Block 0 (all zeros) is sent
14. Simulate `ACK`

**Expected Results:**
- `transferComplete()` emitted
- Progress signals emitted with correct total bytes
- 1 Block 0 + 2 data blocks + 1 EOT + 1 empty Block 0

---

### 3.3 Batch Transfer (Multiple Files)

| Field | Value |
|-------|-------|
| **ID** | YM-003 |
| **Priority** | P1 |
| **Feature** | YMODEM batch file transfer |

**Preconditions:**
- Three files prepared: "a.bin" (128B), "b.bin" (256B), "c.bin" (64B)
- `setFilePaths()` called with all three paths

**Steps:**
1. Call `start()`
2. Simulate `'C'` -- Block 0 for "a.bin" sent
3. Simulate `ACK`
4. Simulate `ACK` for data block 1 (128 bytes of "a.bin")
5. EOT sent, simulate `ACK`
6. (m_fileIndex now = 1, state = `WaitBlock0Ack`)
7. Simulate `'C'` -- Block 0 for "b.bin" sent
8. Simulate `ACK`
9. Simulate `ACK` for block 1, simulate `ACK` for block 2
10. EOT sent, simulate `ACK`
11. (m_fileIndex now = 2, state = `WaitBlock0Ack`)
12. Simulate `'C'` -- Block 0 for "c.bin" sent
13. Simulate `ACK`
14. Simulate `ACK` for data block 1 (64 bytes + 64 bytes padding)
15. EOT sent, simulate `ACK`
16. (All files done, state = `WaitFinalC`)
17. Simulate `'C'` -- empty Block 0 sent
18. Simulate `ACK`

**Expected Results:**
- `transferComplete()` emitted
- Three separate Block 0 headers, each with correct filename and size
- Progress reflects cumulative bytes across all files
- Final empty Block 0 signals batch end

---

### 3.4 Empty Block 0 (Batch End Marker)

| Field | Value |
|-------|-------|
| **ID** | YM-004 |
| **Priority** | P1 |
| **Feature** | End-of-batch empty Block 0 |

**Preconditions:**
- Single file with 128 bytes prepared

**Steps:**
1. Complete single file transfer as in YM-002
2. Verify the final Block 0 written to MockConnection

**Expected Results:**
- Final Block 0 is 128 bytes of all `0x00`
- Wrapped in standard block format: `SOH + 0x00 + 0xFF + 128*B0x00 + CRC16`
- After receiver sends `ACK`, `transferComplete()` is emitted

---

### 3.5 Block 0 NAK Retry

| Field | Value |
|-------|-------|
| **ID** | YM-005 |
| **Priority** | P2 |
| **Feature** | Block 0 retransmission on NAK |

**Preconditions:**
- File with 128 bytes prepared

**Steps:**
1. `start()`, simulate `'C'` -- Block 0 sent
2. Simulate `NAK` -- Block 0 should be retransmitted
3. Simulate `ACK` -- proceed with data blocks

**Expected Results:**
- Block 0 is retransmitted with identical payload
- Transfer proceeds normally after `ACK`
- Retry count is tracked internally

---

### 3.6 Batch Transfer File Open Failure

| Field | Value |
|-------|-------|
| **ID** | YM-006 |
| **Priority** | P2 |
| **Feature** | Handling of inaccessible second file in batch |

**Preconditions:**
- Two file paths set: first file valid, second file non-existent

**Steps:**
1. `start()`, complete transfer of first file
2. After first EOT ACK, sender transitions to `WaitBlock0Ack`
3. Simulate `'C'` -- sender attempts to open second file

**Expected Results:**
- `transferError("Cannot open file: <path>")` is emitted
- `CAN` bytes are written to cancel the session
- Transfer terminates in error state

---

### 3.7 Empty Files Rejection

| Field | Value |
|-------|-------|
| **ID** | YM-007 |
| **Priority** | P3 |
| **Feature** | Reject batch where all files are empty |

**Preconditions:**
- Multiple zero-byte files set via `setFilePaths()`

**Steps:**
1. Call `start()`

**Expected Results:**
- `start()` returns `false`
- `transferError("All files are empty")` is emitted

---

### 3.8 Receiver CAN During YMODEM

| Field | Value |
|-------|-------|
| **ID** | YM-008 |
| **Priority** | P1 |
| **Feature** | Receiver cancellation at various YMODEM stages |

**Preconditions:**
- File with 256 bytes prepared

**Steps (sub-case A: during Block 0):**
1. `start()`, simulate `'C'`
2. Block 0 sent, simulate `CAN`

**Steps (sub-case B: during data blocks):**
1. `start()`, simulate `'C'`
2. Block 0 sent, simulate `ACK`
3. Data block 1 sent, simulate `CAN`

**Expected Results (both sub-cases):**
- `transferError("Transfer cancelled by receiver")` emitted
- Transfer terminates immediately
- `isRunning()` returns `false`

---

## 4. ZMODEM Test Cases

### 4.1 ZRQINIT / ZRINIT Handshake

| Field | Value |
|-------|-------|
| **ID** | ZM-001 |
| **Priority** | P0 |
| **Feature** | ZMODEM session initialization |

**Preconditions:**
- `MockConnection` connected to `ZModemTransfer`
- File "app.bin" with 2048 bytes prepared

**Steps:**
1. Call `setFilePath()`, `start()`
2. Verify `MockConnection.write()` contains ZRQINIT hex header: `ZPAD + ZDLE + ZHEX + "00" + "0000" + CRC16_hex + \r\n`
3. Simulate receiver sending ZRINIT hex frame: `ZPAD + ZDLE + ZHEX + "01" + "00000000" + CRC16_hex + \r\n`
4. Verify ZFILE binary header is written

**Expected Results:**
- ZRQINIT frame type byte is `0x00`
- After receiving ZRINIT, state transitions to `SendingFile`
- ZFILE frame is sent immediately after ZRINIT

---

### 4.2 ZFILE Frame Parsing

| Field | Value |
|-------|-------|
| **ID** | ZM-002 |
| **Priority** | P1 |
| **Feature** | ZFILE data sub-packet content |

**Preconditions:**
- Same as ZM-001, after ZRINIT received

**Steps:**
1. Complete handshake (ZRQINIT -> ZRINIT)
2. Capture the ZFILE binary frame and its data sub-packet
3. Decode the ZDLE-escaped data sub-packet

**Expected Results:**
- ZFILE frame type is `0x04` (ZFILE)
- Data sub-packet contains: `"app.bin <filesize> 0\0"`
- Sub-packet ends with `ZCRCW` flag (waits for receiver response)
- CRC32 is correct for data + end flag

---

### 4.3 ZDATA Continuous Send

| Field | Value |
|-------|-------|
| **ID** | ZM-003 |
| **Priority** | P0 |
| **Feature** | ZMODEM data sub-packet streaming |

**Preconditions:**
- After ZFILE accepted, receiver sends ZRPOS with offset 0

**Steps:**
1. Complete handshake and ZFILE phase
2. Simulate receiver sending ZRPOS with offset = 0 (4 bytes little-endian: `00 00 00 00`)
3. Capture all data written to MockConnection
4. Verify ZDATA header with file offset
5. Verify data sub-packets: first uses `ZCRCG` (continue), last uses `ZCRCW` (wait)
6. For 2048-byte file with `kDataLen=1024`: expect 2 sub-packets

**Expected Results:**
- ZDATA header contains file offset in little-endian
- First sub-packet: 1024 bytes of file data, end flag `ZCRCG` ('i')
- Second sub-packet: 1024 bytes, end flag `ZCRCW` ('k')
- Each sub-packet has correct CRC32 over data + end flag
- ZDLE escaping applied to control characters (0x18, 0x0D, 0x0A, 0x11, 0x13, 0x2A)
- Progress signals emitted

---

### 4.4 ZRPOS Error Retransmission

| Field | Value |
|-------|-------|
| **ID** | ZM-004 |
| **Priority** | P1 |
| **Feature** | Receiver requests retransmission from offset |

**Preconditions:**
- File with 3072 bytes (3 sub-packets of 1024)

**Steps:**
1. Complete handshake, ZFILE, simulate ZRPOS(0)
2. All 3 sub-packets sent (state = `WaitingZAck`)
3. Simulate receiver sending ZRPOS with offset = 1024 (requests retransmit from byte 1024)
4. Verify new ZDATA header with offset 1024
5. Verify 2 sub-packets sent (bytes 1024-2047 and 2048-3071)

**Expected Results:**
- Retransmission starts from the requested offset
- `m_retryCount` incremented
- Transfer completes after retransmission

---

### 4.5 ZRPOS Retry Exhaustion

| Field | Value |
|-------|-------|
| **ID** | ZM-005 |
| **Priority** | P2 |
| **Feature** | Max retransmission requests exceeded |

**Preconditions:**
- File with 1024 bytes

**Steps:**
1. Complete handshake and ZFILE
2. Simulate ZRPOS(0), data sent
3. Simulate ZRPOS(0) again (requesting retransmit from start)
4. Repeat 10 more times (exceed `kMaxRetries`)

**Expected Results:**
- After 10th ZRPOS, `transferError("Too many retransmission requests")` emitted
- Cancel sequence written (8 backspace + 2 CAN)
- Transfer terminates

---

### 4.6 ZFIN End Handshake

| Field | Value |
|-------|-------|
| **ID** | ZM-006 |
| **Priority** | P0 |
| **Feature** | ZMODEM session termination |

**Preconditions:**
- Complete data transfer, ZEOF sent, receiver sent ZRINIT

**Steps:**
1. Complete full transfer through ZDATA phase
2. State is `WaitingZAck`, simulate ZACK
3. ZEOF is sent (contains file size as offset)
4. Simulate ZRINIT (file accepted)
5. ZFIN is sent
6. Simulate ZFIN from receiver
7. Verify "OO" (Over and Out) written

**Expected Results:**
- ZEOF header contains file size in 4 bytes little-endian
- After receiver ZFIN, "OO" bytes are written
- `transferComplete()` emitted
- `progress(100, fileSize, fileSize)` emitted

---

### 4.7 ZSKIP (Receiver Skips File)

| Field | Value |
|-------|-------|
| **ID** | ZM-007 |
| **Priority** | P2 |
| **Feature** | Receiver rejects file with ZSKIP |

**Preconditions:**
- After ZFILE sent, in `SendingFile` state

**Steps:**
1. Complete handshake
2. ZFILE sent
3. Simulate ZSKIP (type=5) from receiver

**Expected Results:**
- ZFIN is sent immediately (skipping data transfer)
- After receiver ZFIN, "OO" is written
- `transferComplete()` emitted (session ends cleanly, even though file was skipped)

---

### 4.8 Empty File Rejection

| Field | Value |
|-------|-------|
| **ID** | ZM-008 |
| **Priority** | P3 |
| **Feature** | ZMODEM rejects empty files |

**Preconditions:**
- Empty file path set

**Steps:**
1. Call `start()`

**Expected Results:**
- `start()` returns `false`
- `transferError("File is empty")` emitted

---

### 4.9 Hex Frame Parsing - Insufficient Data

| Field | Value |
|-------|-------|
| **ID** | ZM-009 |
| **Priority** | P2 |
| **Feature** | Partial hex frame handling |

**Preconditions:**
- Transfer in `WaitingRinit` state, ZRQINIT sent

**Steps:**
1. Simulate receiving only the first 5 bytes of a ZRINIT frame (e.g., `ZPAD + ZDLE + ZHEX + "0"`)
2. Verify no frame is parsed (buffer retains partial data)
3. Simulate receiving the remaining bytes to complete the frame

**Expected Results:**
- After partial data, `parseHexFrame` returns `false`
- Receive buffer retains the partial data
- After remaining bytes arrive, frame is parsed correctly

---

### 4.10 Receive Buffer Overflow Protection

| Field | Value |
|-------|-------|
| **ID** | ZM-010 |
| **Priority** | P2 |
| **Feature** | Buffer cleared when exceeds 4096 bytes of unparseable data |

**Preconditions:**
- Transfer in `WaitingRinit` state

**Steps:**
1. Simulate receiving 4097 bytes of garbage (no ZPAD marker)

**Expected Results:**
- Receive buffer is cleared (`m_receiveBuffer.clear()`)
- No crash or infinite loop
- Transfer remains in `WaitingRinit`, waiting for valid frame

---

### 4.11 User Cancel During ZMODEM

| Field | Value |
|-------|-------|
| **ID** | ZM-011 |
| **Priority** | P1 |
| **Feature** | Sender cancellation at various ZMODEM stages |

**Steps (sub-case A: during WaitingRinit):**
1. `start()` (ZRQINIT sent)
2. Call `cancel()`

**Steps (sub-case B: during SendingData):**
1. Complete through ZFILE + ZRPOS
2. During data send, call `cancel()`

**Expected Results:**
- Cancel sequence written: 8 bytes of `0x08` (backspace) + 2 bytes of `0x18` (CAN)
- `transferError("Transfer cancelled by user")` emitted
- Timer stopped, state reset to Idle

---

## 5. OtaManager Integration Test Cases

### 5.1 Protocol Routing

| Field | Value |
|-------|-------|
| **ID** | OM-001 |
| **Priority** | P0 |
| **Feature** | Correct protocol selection based on string |

**Steps & Expected Results:**

| Input Protocol | Expected Action |
|----------------|----------------|
| `"xmodem-checksum"` | `XModemTransfer::setMode(Checksum)` then `xmodem->start()` |
| `"xmodem-crc"` | `XModemTransfer::setMode(CRC)` then `xmodem->start()` |
| `"xmodem-1k"` | `XModemTransfer::setMode(OneK)` then `xmodem->start()` |
| `"ymodem"` | `ymodem->setFilePath()` then `ymodem->start()` |
| `"zmodem"` | `zmodem->setFilePath()` then `zmodem->start()` |
| `"anything-else"` | Defaults to `XModemTransfer::CRC` |

### 5.2 Cancel Dispatches to Active Protocol

| Field | Value |
|-------|-------|
| **ID** | OM-002 |
| **Priority** | P1 |

**Steps:**
1. Start YMODEM transfer
2. Call `cancelTransfer()`

**Expected Results:**
- `ymodem->cancel()` is called
- `xmodem->cancel()` and `zmodem->cancel()` are also called (defensive, but only ymodem is running)
- `isTransferring()` returns `false`

### 5.3 No Connection Error

| Field | Value |
|-------|-------|
| **ID** | OM-003 |
| **Priority** | P2 |

**Steps:**
1. Do NOT call `setConnection()`
2. Call `startTransfer("file.bin", "xmodem-crc")`

**Expected Results:**
- `transferError("No connection available")` emitted
- Returns `false`

### 5.4 Signal Forwarding

| Field | Value |
|-------|-------|
| **ID** | OM-004 |
| **Priority** | P0 |

**Steps:**
1. Set connection, start XMODEM transfer
2. Complete the transfer (simulate full protocol flow)

**Expected Results:**
- `OtaManager::progress` signal emitted when `XModemTransfer::progress` fires
- `OtaManager::transferComplete` emitted when `XModemTransfer::transferComplete` fires
- `OtaManager::transferError` emitted when `XModemTransfer::transferError` fires

---

## 6. DataLogger Test Cases

### 6.1 Normal Recording Flow

| Field | Value |
|-------|-------|
| **ID** | DL-001 |
| **Priority** | P0 |
| **Feature** | Start, log data, stop recording |

**Preconditions:**
- Writable temp directory available

**Steps:**
1. Call `startRecording("<temp>/test.edl")`
2. Verify `recordingStarted()` signal emitted
3. Call `logData(QByteArray("Hello"), Direction::Sent)` at T=0ms
4. Wait 50ms
5. Call `logData(QByteArray("World"), Direction::Received)` at T~50ms
6. Call `stopRecording()`

**Expected Results:**
- `recordingStopped(path, 2, durationMs)` signal emitted
- `recordCount()` was 2 before stop
- File exists on disk, size > 8 (header) + record data
- EDL file header: magic "EDL" + version 1 + record count 2 (as quint32 big-endian)

---

### 6.2 Pause and Resume Recording

| Field | Value |
|-------|-------|
| **ID** | DL-002 |
| **Priority** | P1 |
| **Feature** | Time pause does not create gaps |

**Preconditions:**
- Recording started

**Steps:**
1. `startRecording()`, `logData("A", Sent)` at T=0
2. Wait 100ms
3. `pauseRecording()` at T~100ms
4. Wait 500ms (paused -- time should not advance)
5. `logData("B", Received)` -- should be silently ignored (paused)
6. `resumeRecording()` at T~600ms
7. Wait 100ms
8. `logData("C", Sent)` at T~200ms effective (600ms real - 500ms pause offset)

**Expected Results:**
- `isPaused()` returns `true` during pause
- `logData()` calls during pause are silently ignored (data "B" not recorded)
- Third record "C" has timestamp ~200ms (100ms before pause + 100ms after resume)
- Record count is 2 (A and C only)

---

### 6.3 EDL File Format Validation

| Field | Value |
|-------|-------|
| **ID** | DL-003 |
| **Priority** | P0 |
| **Feature** | Binary format correctness |

**Preconditions:**
- Recording completed with 1 record: `logData("\x01\x02\x03", Direction::Received)` at known timestamp

**Steps:**
1. Open the EDL file in raw binary mode
2. Read and validate header (8 bytes)
3. Read and validate record

**Expected Results:**
- Bytes 0-2: `'E' 'D' 'L'` (magic)
- Byte 3: `0x01` (version)
- Bytes 4-7: `0x00 0x00 0x00 0x01` (record count = 1, big-endian quint32)
- Record: timestamp (8 bytes big-endian quint64) + direction (1 byte: `0x00` for Received) + length (4 bytes big-endian quint32: `0x03`) + raw data `\x01\x02\x03`

---

### 6.4 Normal Playback Flow

| Field | Value |
|-------|-------|
| **ID** | DL-004 |
| **Priority** | P0 |
| **Feature** | Play back a recorded EDL file |

**Preconditions:**
- EDL file recorded with 3 records at timestamps 0ms, 100ms, 200ms

**Steps:**
1. Call `startPlayback("<path>/test.edl")`
2. Record emitted `playbackData()` signals
3. Wait for `playbackFinished()` signal

**Expected Results:**
- First `playbackData()` emitted immediately with record 1 data
- Second `playbackData()` emitted ~100ms later
- Third `playbackData()` emitted ~100ms after second
- `playbackProgress()` emitted with values: ~0.33, ~0.67, 1.0
- `playbackFinished()` emitted after last record
- `isPlaying()` returns `false` after finish

---

### 6.5 Variable Speed Playback

| Field | Value |
|-------|-------|
| **ID** | DL-005 |
| **Priority** | P1 |
| **Feature** | Speed multiplier changes playback timing |

**Preconditions:**
- EDL file with 2 records at timestamps 0ms and 1000ms

**Steps (sub-case A: 2x speed):**
1. `setPlaybackSpeed(2.0)`
2. `startPlayback()`
3. Measure time between first and second `playbackData()` signals

**Steps (sub-case B: 0.5x speed):**
1. `setPlaybackSpeed(0.5)`
2. `startPlayback()`
3. Measure time between signals

**Expected Results:**
- 2x: second signal at ~500ms (1000ms / 2.0)
- 0.5x: second signal at ~2000ms (1000ms / 0.5)
- `playbackSpeedChanged(2.0)` / `playbackSpeedChanged(0.5)` emitted

---

### 6.6 Playback Pause and Resume

| Field | Value |
|-------|-------|
| **ID** | DL-006 |
| **Priority** | P1 |
| **Feature** | Pause/resume mid-playback |

**Preconditions:**
- EDL file with records at 0ms, 500ms, 1000ms

**Steps:**
1. `startPlayback()` -- record 1 emitted immediately
2. Wait 200ms, call `pausePlayback()`
3. Wait 1000ms (should not emit any data during pause)
4. Call `resumePlayback()`
5. Wait for remaining records

**Expected Results:**
- No `playbackData()` during pause
- After resume, record 2 emitted at the correct remaining time offset
- Record 3 follows at correct interval
- `playbackFinished()` emitted eventually

---

### 6.7 Speed Boundary Clamping

| Field | Value |
|-------|-------|
| **ID** | DL-007 |
| **Priority** | P3 |
| **Feature** | `setPlaybackSpeed` clamps to [0.1, 100.0] |

**Steps:**
1. `setPlaybackSpeed(0.01)`
2. `setPlaybackSpeed(200.0)`

**Expected Results:**
- First call: speed clamped to 0.1
- Second call: speed clamped to 100.0
- `playbackSpeedChanged()` emits clamped values

---

### 6.8 Invalid EDL File - Wrong Magic

| Field | Value |
|-------|-------|
| **ID** | DL-008 |
| **Priority** | P3 |
| **Feature** | Reject file with wrong magic bytes |

**Steps:**
1. Create a file with content "XYZ" + version byte + padding
2. Call `startPlayback()`

**Expected Results:**
- `startPlayback()` returns `false`
- `error("Invalid log file format")` emitted
- No playback begins

---

### 6.9 Invalid EDL File - Unsupported Version

| Field | Value |
|-------|-------|
| **ID** | DL-009 |
| **Priority** | P3 |

**Steps:**
1. Create a file with magic "EDL" + version byte = 99
2. Call `startPlayback()`

**Expected Results:**
- `error("Unsupported log version: 99")` emitted
- Returns `false`

---

### 6.10 Empty EDL File

| Field | Value |
|-------|-------|
| **ID** | DL-010 |
| **Priority** | P3 |
| **Feature** | Valid header but zero records |

**Steps:**
1. Create a valid EDL file with magic + version + count=0
2. Call `startPlayback()`

**Expected Results:**
- `error("Log file is empty")` emitted
- Returns `false`

---

### 6.11 Corrupt Record (Truncated File)

| Field | Value |
|-------|-------|
| **ID** | DL-011 |
| **Priority** | P3 |
| **Feature** | Handle truncated record gracefully |

**Preconditions:**
- EDL file with header claiming 2 records, but only 1 complete record + partial second record

**Steps:**
1. `startPlayback()`
2. Let first record play normally
3. Wait for second record to be attempted

**Expected Results:**
- First record plays correctly
- `readNextRecord()` returns `false` for truncated record
- `stopPlayback()` is called internally
- `playbackFinished()` emitted
- No crash

---

### 6.12 Recording Overwrite Protection

| Field | Value |
|-------|-------|
| **ID** | DL-012 |
| **Priority** | P2 |
| **Feature** | Starting new recording while one is active |

**Steps:**
1. `startRecording("file1.edl")`
2. Log some data
3. `startRecording("file2.edl")` (should stop first recording)

**Expected Results:**
- First recording is stopped (`recordingStopped` emitted for file1)
- Second recording starts fresh (`recordingStarted` emitted)
- File 1 is properly closed and valid on disk

---

### 6.13 Recording Duration Calculation

| Field | Value |
|-------|-------|
| **ID** | DL-013 |
| **Priority** | P2 |
| **Feature** | `recordingDuration()` excludes pause time |

**Steps:**
1. `startRecording()`
2. Record data, note `recordingDuration()` value D1
3. `pauseRecording()`, wait 2 seconds
4. Verify `recordingDuration()` has NOT increased by 2 seconds
5. `resumeRecording()`, wait 1 second
6. Verify `recordingDuration()` has increased by ~1 second

**Expected Results:**
- Pause time is excluded from duration
- Duration is approximately (real_elapsed - pause_time)

---

## 7. OtaHistoryModel Test Cases

### 7.1 Add Record

| Field | Value |
|-------|-------|
| **ID** | OH-001 |
| **Priority** | P0 |
| **Feature** | Adding an OTA record to the model |

**Preconditions:**
- Fresh `OtaHistoryModel` instance (no existing records)

**Steps:**
1. Create an `OtaRecord` with: fileName="fw.bin", protocol="xmodem-crc", fileSize=1024, success=true
2. Call `addRecord(record)`
3. Verify `rowCount()` returns 1
4. Verify `data(index(0, ColFileName))` returns "fw.bin"
5. Verify `data(index(0, ColProtocol))` returns "XMODEM-CRC" (uppercased)
6. Verify `data(index(0, ColSize))` returns "1.0 KB"
7. Verify `data(index(0, ColResult))` returns "Success"

**Expected Results:**
- Record is prepended at row 0
- All column data displays correctly
- `count()` returns 1

---

### 7.2 Add Multiple Records (Prepend Order)

| Field | Value |
|-------|-------|
| **ID** | OH-002 |
| **Priority** | P1 |
| **Feature** | Records appear in reverse chronological order |

**Steps:**
1. Add record A with startTime = 10:00:00
2. Add record B with startTime = 10:05:00
3. Check `data(index(0, ColTime))` and `data(index(1, ColTime))`

**Expected Results:**
- Row 0 = record B (newest first)
- Row 1 = record A (oldest second)
- `count()` = 2

---

### 7.3 Clear History

| Field | Value |
|-------|-------|
| **ID** | OH-003 |
| **Priority** | P1 |
| **Feature** | Removing all records |

**Steps:**
1. Add 5 records
2. Call `clearHistory()`
3. Check `rowCount()` and `count()`

**Expected Results:**
- `rowCount()` = 0
- `count()` = 0
- Model reset signal emitted (view updates correctly)

---

### 7.4 Persistence Save

| Field | Value |
|-------|-------|
| **ID** | OH-004 |
| **Priority** | P0 |
| **Feature** | Records saved to SettingsManager as JSON |

**Preconditions:**
- SettingsManager initialized with a test backend

**Steps:**
1. Add 3 records with known values
2. Verify `saveToSettings()` is called automatically by `addRecord()`
3. Read `SettingsManager.get("ota_history/records")`
4. Parse the JSON and validate structure

**Expected Results:**
- JSON is a valid `QJsonArray` with 3 elements
- Each element contains keys: "fileName", "protocol", "fileSize", "startTime", "durationMs", "success", "errorMessage"
- Values match the input records exactly
- `startTime` is in ISO date format

---

### 7.5 Persistence Load

| Field | Value |
|-------|-------|
| **ID** | OH-005 |
| **Priority** | P0 |
| **Feature** | Records loaded from SettingsManager on construction |

**Preconditions:**
- SettingsManager contains saved JSON from test OH-004

**Steps:**
1. Create a new `OtaHistoryModel` instance (constructor calls `loadFromSettings()`)

**Expected Results:**
- `count()` = 3
- All records match the previously saved data
- Field types correct: `fileSize` is integer, `success` is boolean, `startTime` parses correctly

---

### 7.6 Load Corrupt JSON

| Field | Value |
|-------|-------|
| **ID** | OH-006 |
| **Priority** | P3 |
| **Feature** | Graceful handling of invalid persisted data |

**Preconditions:**
- SettingsManager contains "ota_history/records" = `{not valid json`

**Steps:**
1. Create a new `OtaHistoryModel` instance

**Expected Results:**
- No crash
- `count()` = 0
- Model is in valid empty state

---

### 7.7 Load Non-Array JSON

| Field | Value |
|-------|-------|
| **ID** | OH-007 |
| **Priority** | P3 |

**Preconditions:**
- SettingsManager contains "ota_history/records" = `"hello"` (valid JSON but not an array)

**Steps:**
1. Create `OtaHistoryModel`

**Expected Results:**
- No crash
- `count()` = 0

---

### 7.8 Maximum Record Limit

| Field | Value |
|-------|-------|
| **ID** | OH-008 |
| **Priority** | P2 |
| **Feature** | Records capped at `kMaxRecords = 200` |

**Steps:**
1. Add 201 records sequentially
2. Check `count()`

**Expected Results:**
- `count()` = 200
- The oldest record (first added) is evicted
- The newest record (last added) is at row 0
- Model remove rows signal emitted for the evicted record

---

### 7.9 Failed Record Display

| Field | Value |
|-------|-------|
| **ID** | OH-009 |
| **Priority** | P1 |
| **Feature** | Failed record shows error info |

**Steps:**
1. Add a record with `success = false`, `errorMessage = "Timeout at block 42"`
2. Check `data(index(0, ColResult))` with `Qt::DisplayRole`
3. Check `data(index(0, ColResult))` with `Qt::ForegroundRole`
4. Check `data(index(0, ColResult))` with `Qt::ToolTipRole`

**Expected Results:**
- DisplayRole: "Failed"
- ForegroundRole: `QColor("#f38ba8")` (error color)
- ToolTipRole: "Timeout at block 42"

---

### 7.10 File Size Formatting

| Field | Value |
|-------|-------|
| **ID** | OH-010 |
| **Priority** | P3 |
| **Feature** | Human-readable file size display |

| Input Size | Expected Display |
|------------|-----------------|
| 500 | "500 B" |
| 1536 | "1.5 KB" |
| 5242880 | "5.00 MB" |

**Steps:**
1. Add records with each file size
2. Read `data(index(row, ColSize))`

---

### 7.11 Duration Formatting

| Field | Value |
|-------|-------|
| **ID** | OH-011 |
| **Priority** | P3 |

| Input Duration | Expected Display |
|----------------|-----------------|
| 500 | "500 ms" |
| 3500 | "3.5 s" |

---

### 7.12 Header Data

| Field | Value |
|-------|-------|
| **ID** | OH-012 |
| **Priority** | P3 |
| **Feature** | Column header labels |

**Steps:**
1. Call `headerData(0, Qt::Horizontal)` through `headerData(5, Qt::Horizontal)`

**Expected Results:**
- Col 0: "Time"
- Col 1: "File"
- Col 2: "Protocol"
- Col 3: "Size"
- Col 4: "Duration"
- Col 5: "Result"

---

### 7.13 Record Accessor

| Field | Value |
|-------|-------|
| **ID** | OH-013 |
| **Priority** | P2 |
| **Feature** | `record(int row)` returns correct reference |

**Steps:**
1. Add 3 records with distinct values
2. Call `record(0)`, `record(1)`, `record(2)`
3. Verify each field matches the original input

**Expected Results:**
- `record(0)` is the most recently added (newest)
- All fields accessible and correct
- Out-of-range row causes assertion/crash (expected: caller must check bounds)

---

### 7.14 Save After Clear

| Field | Value |
|-------|-------|
| **ID** | OH-014 |
| **Priority** | P2 |
| **Feature** | Clear persists (empty JSON array saved) |

**Steps:**
1. Add records
2. `clearHistory()`
3. Read SettingsManager value

**Expected Results:**
- SettingsManager contains `"[]"` (empty JSON array)
- Loading from this state produces an empty model

---

## 8. Summary Statistics

| Module | P0 | P1 | P2 | P3 | Total |
|--------|----|----|----|----|-------|
| XModemTransfer | 3 | 2 | 5 | 5 | 15 |
| YModemTransfer | 2 | 2 | 2 | 1 | 7 |
| ZModemTransfer | 3 | 2 | 4 | 1 | 10 |
| OtaManager | 2 | 1 | 1 | 0 | 4 |
| DataLogger | 3 | 2 | 2 | 5 | 12 |
| OtaHistoryModel | 3 | 2 | 2 | 6 | 13 |
| **Total** | **16** | **11** | **16** | **18** | **61** |

### Execution Order Recommendation

1. Build `MockConnection` and test helpers first (enables all protocol tests)
2. Execute P0 tests across all modules (core correctness)
3. Execute P1 tests (error recovery and complex flows)
4. Execute P2 tests (edge cases and boundaries)
5. Execute P3 tests (defensive checks)

### Dependencies Between Test Modules

```
CRC.h (validated independently)
  |
  v
MockConnection --> XModemTransfer tests
               --> YModemTransfer tests (depends on XMODEM CRC correctness)
               --> ZModemTransfer tests
               --> OtaManager tests (depends on all 3 protocol tests)
SettingsManager --> OtaHistoryModel tests
File I/O        --> DataLogger tests
```
