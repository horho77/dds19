import os

import auraloss
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, random_split
from torch.utils.tensorboard import SummaryWriter

from dataset import DDS19Dataset
from model import DDS19Model

SAMPLE_RATE = 44100
SEGMENT_SECONDS = 4
SEGMENT_SIZE = SAMPLE_RATE * SEGMENT_SECONDS

EPOCHS = 1000
LR_START = 0.001
LR_STOP = 1e-6

HIDDEN_SIZE = 32

DATASET_DIR = "dataset"
MODEL_DIR = "model"
TEST_DIR = "test"

MODEL_NAME = f"dds19_lstm{HIDDEN_SIZE}"
MODEL_CHECKPOINT_NAME = f"{MODEL_NAME}_checkpoint.pt"
MODEL_TRACED_NAME = f"{MODEL_NAME}_traced.pt"
MODEL_JSON_NAME = f"{MODEL_NAME}.json"

device = "cuda" if torch.cuda.is_available() else "cpu"
print(f"Using {device} device")

dataset = DDS19Dataset(DATASET_DIR, SEGMENT_SECONDS)
dataset_len = len(dataset)
train_len = int(dataset_len * 0.8)
val_len = dataset_len - train_len
train_data, val_data = random_split(dataset, [train_len, val_len])

train_loader = DataLoader(train_data, batch_size=8, shuffle=True)
val_loader = DataLoader(val_data, batch_size=8, shuffle=True)

model = DDS19Model(hidden_size=HIDDEN_SIZE).to(device)
loss_l1 = nn.L1Loss()
loss_stft = auraloss.freq.STFTLoss(device=device)
optimiser = optim.Adam(model.parameters(), lr=LR_START)
scheduler = optim.lr_scheduler.ReduceLROnPlateau(optimiser, factor=0.5, patience=1000, verbose=True)
writer = SummaryWriter(os.path.join("runs", MODEL_NAME))

print("\nModel:")
print(model)

checkpoint_epoch, checkpoint_loss, _, _ = model.load_checkpoint(MODEL_DIR, MODEL_CHECKPOINT_NAME, device)

print("\nTraining:")
for epoch in range(checkpoint_epoch, checkpoint_epoch + EPOCHS):
    train_loss = model.train_epoch(train_loader, device, loss_l1, loss_stft, writer, epoch, scheduler, optimiser)
    val_loss = model.validate_epoch(val_loader, device, loss_l1, loss_stft, writer, epoch)

    current_epoch = epoch + 1
    if checkpoint_loss > train_loss:
        checkpoint_loss = train_loss
        model.store_checkpoint(MODEL_DIR, MODEL_CHECKPOINT_NAME, optimiser, current_epoch, train_loss, val_loss)
        model.store_json(MODEL_DIR, MODEL_JSON_NAME)

    lr = optimiser.param_groups[0]["lr"]
    writer.add_scalar("Epoch Loss/Training", train_loss, current_epoch)
    writer.add_scalar("Epoch Loss/Validation", val_loss, current_epoch)
    writer.add_scalar("Learning rate", lr, current_epoch)

    print(f"Epoch: {current_epoch} | Train loss: {train_loss:>.7f} | Val loss: {val_loss:>.7f}\n")

    if lr < LR_STOP:
        break

print("\nTraining finished!\n")
