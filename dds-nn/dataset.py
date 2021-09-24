import glob
import os
import torch
import torchaudio
import numpy as np

from torch.utils.data import Dataset


# Training requires audio dataset in following format:
# dry.wav
# wet__0__00.wav
# wet__0__01.wav
# wet__0__02.wav
# wet__0__03.wav
# ...
#
# Where:
# dry.wav           => unprocessed audio
# wet__p1__p2.wav   => processed audio with device parameters p1 and p2
# p1                => S/F: 0 or 1 (not engaged/engaged)
# p2                => DELAY FINE: 0 to 10 (normalised to 0.0 to 1.0)

class DDS19Dataset(Dataset):
    def __init__(self, audio_dir, segment_length_seconds):
        self._data = []

        dry_file = os.path.join(audio_dir, "dry.wav")
        wet_files = glob.glob(os.path.join(audio_dir, "wet*.wav"))
        wet_files.sort()
        params_list = [(float(f.split("__")[1]), float(f.split("__")[2].replace(".wav", "")) / 10) for f in wet_files]

        print("\nAudio files & parameters:")
        dry, dry_sample_rate = torchaudio.load(dry_file)
        for index, (wet_file, params_values) in enumerate(zip(wet_files, params_list)):
            print(f"Dry: {dry_file} | Wet: {wet_file} | Params: {params_values}")

            wet, wet_sample_rate = torchaudio.load(wet_file)
            params = torch.tensor(params_values).unsqueeze(0)
            if dry_sample_rate != wet_sample_rate:
                raise RuntimeError(f"Dataset sample rate mismatch! Dry: {dry_sample_rate} | Wet: {wet_sample_rate}")

            segment_length = segment_length_seconds * dry_sample_rate
            num_frames = int(np.min([dry.shape[-1], wet.shape[-1]]))

            for n in range((num_frames // segment_length)):
                offset_start = int(n * segment_length)
                offset_end = offset_start + segment_length

                dry_segment = dry[:, offset_start:offset_end].T
                params_segment = params.repeat(segment_length, 1)
                input_segment = torch.cat((dry_segment, params_segment), 1)
                target_segment = wet[:, offset_start:offset_end].T

                self._data.append({"input_segment": input_segment,
                                   "target_segment": target_segment})

    def __getitem__(self, index):
        input_segment = self._data[index]["input_segment"]
        target_segment = self._data[index]["target_segment"]
        return input_segment, target_segment

    def __len__(self):
        return len(self._data)
