"""录制格式转换导出器。"""

from __future__ import annotations

from pathlib import Path

from embeddebug.serial_station.recording.format import (
    RecordingFormat,
    RecordingHeader,
    RecordingReader,
    RecordingWriter,
)


class RecordingExporter:
    """格式转换 + 通道子集 + 降采样。"""

    def export(
        self,
        source_path: str | Path,
        target_path: str | Path,
        target_format: RecordingFormat,
        channels: list[str] | None = None,
    ) -> bool:
        try:
            source_format = RecordingFormat.from_extension(source_path)
            reader = RecordingReader(source_format)
            header = reader.open(source_path)
            indices = self._channel_indices(header, channels)
            new_names = tuple(header.channel_names[i] for i in indices)
            new_header = RecordingHeader(start_time_ns=header.start_time_ns, dt_ns=header.dt_ns, channel_names=new_names)
            writer = RecordingWriter(target_format, new_header)
            writer.open(target_path)
            try:
                for batch in reader.iter_batches():
                    subset = batch.values[:, indices]
                    from embeddebug.serial_station.core.measurements import ChannelBatch
                    writer.write_batch(ChannelBatch(new_names, subset, batch.t0_ns, batch.dt_ns))
            finally:
                writer.close()
                reader.close()
            return True
        except (OSError, ValueError, KeyError):
            return False

    @staticmethod
    def _channel_indices(header: RecordingHeader, channels: list[str] | None) -> list[int]:
        if not channels:
            return list(range(header.channel_count))
        name_to_idx = {n: i for i, n in enumerate(header.channel_names)}
        return [name_to_idx[n] for n in channels]
