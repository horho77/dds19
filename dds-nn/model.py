import os
import json
import torch
import torchaudio
import torch.nn as nn


class DDS19Model(nn.Module):
    def __init__(self, hidden_size):
        super(DDS19Model, self).__init__()
        self.lstm = nn.LSTM(input_size=3, hidden_size=hidden_size, num_layers=1, batch_first=True)
        self.linear = nn.Linear(in_features=hidden_size, out_features=1)

    def forward(self, data_in):
        lstm_out, _ = self.lstm(data_in)
        linear_out = self.linear(lstm_out)
        return linear_out

    @torch.jit.ignore
    def train_epoch(self, loader, device, loss_l1, loss_stft, writer, epoch, scheduler, optimiser):
        train_loss_sum = 0
        last_batch_num = 0
        num_batches = len(loader)
        for batch, (train_input, train_target) in enumerate(loader):
            train_input, train_target = train_input.to(device), train_target.to(device)

            self.zero_grad()
            train_pred = self(train_input)
            train_loss_l1 = loss_l1(train_pred, train_target)
            train_loss_stft = loss_stft(train_pred.permute(0, 2, 1), train_target.permute(0, 2, 1))
            train_loss = train_loss_l1 + train_loss_stft
            train_loss.backward()
            optimiser.step()

            step = epoch * num_batches + batch
            writer.add_scalar("Step Loss/Total", train_loss.item(), step)
            writer.add_scalar("Step Loss/L1", train_loss_l1.item(), step)
            writer.add_scalar("Step Loss/STFT", train_loss_stft.item(), step)

            print(f"Step: {step} | Total loss: {train_loss.item():>.7f}"
                  f" | L1 loss: {train_loss_l1.item():>.7f} | STFT loss: {train_loss_stft.item():>.7f}")

            scheduler.step(train_loss)
            train_loss_sum += train_loss
            last_batch_num = batch

        train_loss_avg = train_loss_sum / (last_batch_num + 1)
        return train_loss_avg.item()

    @torch.jit.ignore
    def validate_epoch(self, loader, device, loss_l1, loss_stft, writer, epoch):
        with torch.no_grad():
            print("Validating...")
            val_loss_sum = 0
            last_batch_num = 0
            self.eval()
            for batch, (val_input, val_target) in enumerate(loader):
                val_input, val_target = val_input.to(device), val_target.to(device)

                val_pred = self(val_input)
                val_loss_l1 = loss_l1(val_pred, val_target)
                val_loss_stft = loss_stft(val_pred.permute(0, 2, 1), val_target.permute(0, 2, 1))
                val_loss = val_loss_l1 + val_loss_stft

                val_loss_sum += val_loss
                last_batch_num = batch

            self.train()
            val_loss_avg = val_loss_sum / (last_batch_num + 1)
            return val_loss_avg.item()

    @torch.jit.ignore
    def store_checkpoint(self, store_dir, store_name, optimiser, epoch, train_loss, val_loss):
        if not os.path.exists(store_dir):
            os.makedirs(store_dir)

        store_path = os.path.join(store_dir, store_name)
        torch.save({
            'model_state_dict': self.state_dict(),
            'optimiser_state_dict': optimiser.state_dict(),
            'epoch': epoch,
            'train_loss': train_loss,
            'val_loss': val_loss
        }, store_path)

    @torch.jit.ignore
    def load_checkpoint(self, load_dir, load_name, device):
        load_path = os.path.join(load_dir, load_name)
        if not os.path.exists(load_path):
            return 0, 1000, 1000, None

        model_dict = torch.load(load_path, map_location=torch.device(device))

        self.load_state_dict(model_dict['model_state_dict'])
        optimiser_state_dict = model_dict['optimiser_state_dict']
        epoch = model_dict['epoch']
        train_loss = model_dict['train_loss']
        val_loss = model_dict['val_loss']

        return epoch, train_loss, val_loss, optimiser_state_dict

    @torch.jit.ignore
    def store_json(self, store_dir, store_name):
        if not os.path.exists(store_dir):
            os.makedirs(store_dir)

        store_path = os.path.join(store_dir, store_name)
        model_state = self.state_dict()
        for item in model_state:
            model_state[item] = model_state[item].tolist()

        with open(store_path, 'w') as fp:
            json.dump(model_state, fp)

    @torch.jit.ignore
    def store_traced(self, export_dir, export_name, segment_size, device):
        with torch.no_grad():
            if not os.path.exists(export_dir):
                os.makedirs(export_dir)

            export_path_gpu = os.path.join(export_dir, export_name.replace(".pt", "_gpu.pt"))
            export_path_cpu = os.path.join(export_dir, export_name.replace(".pt", "_cpu.pt"))

            random_data_cpu = torch.rand(1, segment_size, 3)
            random_data_gpu = torch.rand(1, segment_size, 3).to(device)

            self.eval()
            if device == 'cuda':
                traced_model_gpu = torch.jit.trace(self, random_data_gpu)
                traced_model_gpu.save(export_path_gpu)
                traced_model_cpu = torch.jit.trace(self.to('cpu'), random_data_cpu)
                traced_model_cpu.save(export_path_cpu)
                self.to('cuda')
            else:
                traced_model = torch.jit.trace(self, random_data_cpu)
                traced_model.save(export_path_cpu)
            self.train()
