# ---
# jupyter:
#   jupytext:
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.16.1
#   kernelspec:
#     display_name: Python 3 (ipykernel)
#     language: python
#     name: python3
# ---

# %matplotlib inline
# %env CUDA_VISIBLE_DEVICES=1

import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import os
import tensorflow as tf
import math

# # Extract Data

class DataExtract:
    _path_dir = None
    _n_channels = None
    _files_tau = None
    _files_dat = None
    _time_delays = None
    _angles = None
    def __init__(self, path_dir, n_channels):
        self._path_dir = path_dir
        self._n_channels = n_channels
        self._extract_angles()
        self._extract_delays()
        self._max_norm()
        self._fixed_ref_channel()

    # This function uses for sorting files
    def _key(self, file):
        # Sort by character at 26th place (digit)
        return(file[26])
        
    def _extract_angles(self):
        # Source signal information files
        self._files_dat = [i for i in os.listdir(self._path_dir) if os.path.isfile(os.path.join(self._path_dir,i)) and \
                 '.dat' in i]
        self._files_dat = sorted(self._files_dat, key = self._key)   
        print(self._files_dat)
        
        # Extract signal information
        dats = []
        for file in self._files_dat:
            with open(self._path_dir+file, 'r') as f:
                lines = f.readlines()
                for i,l in enumerate(lines):
                    if (i>0):
                        dat = [float(i) for i in l.split()]
                        dats.append(dat)
        self._angles = np.array(dats, dtype=np.float32)[:,2]
        self._angles = np.reshape(self._angles, [self._angles.shape[0], 1])

    def _extract_delays(self):
        # Time delay files
        self._files_tau = [i for i in os.listdir(self._path_dir) if os.path.isfile(os.path.join(self._path_dir,i)) and \
                           'tau.bin' in i]
        self._files_tau = sorted(self._files_tau, key = self._key)   
        print(self._files_tau)
        
        # Extract Time delay
        taus = []
        for file in self._files_tau:
            with open(self._path_dir+file, 'rb') as f:
                delay = f.read()
                delay = np.frombuffer(delay, dtype = np.float32)
                delay = np.resize(delay, (int(len(delay)/self._n_channels), self._n_channels))
                taus.append(delay)
        taus_np = np.array(taus, dtype=np.float32)
        self._time_delays = taus_np.reshape(taus_np.shape[0]*taus_np.shape[1], taus_np.shape[2])
        
    def _max_norm(self):
        abs_max = np.max(np.abs(self._time_delays), axis = (0,1))
        self._time_delays /= abs_max

    def _fixed_ref_channel(self):
        self._time_delays -= self._time_delays[:,[0]]
        
    def plot(self):
        # Time delay vs angle at different channel
        fig = plt.figure(figsize = (20,20))
        ax = plt.axes()
        for i in range(self._n_channels):
            ax.scatter(self._angles, self._time_delays[:,i], label="ch{}".format(i))
        plt.title("Time Delay vs. DOA for Each Channel")
        ax.set_xlabel('DOA (degree)', fontweight ='bold') 
        ax.set_ylabel('Time Delay (ms)', fontweight ='bold') 
        ax.legend()

    def get_delays(self, channels=[]):
        if channels:
            # return specific channels
            return(self._time_delays[:,channels])
        return(self._time_delays)
        
    def get_angles(self):
        return(self._angles) 


class DataSetPacker:
    def __init__(self, inputs, labels, channels=[]):
        self._inputs = inputs
        self._labels = labels
        self._channels = channels
        self._dataset = None
        self._inputs_subset = None
        
        self._dataset_train = None
        self._dataset_test = None
        self._dataset_val = None

        self._selected_channels()
        self._pack_data()
    def _selected_channels(self):
        # get the selected channels
        if self._channels:
            self._inputs_subset = self._inputs[:,self._channels]
        else:
            self._inputs_subset = self._inputs
    def _pack_data(self):
        # pack data into tensorflow dataset
        self._dataset = tf.data.Dataset.from_tensor_slices((self._inputs_subset, self._labels))
        
    def _shuffle_dataset(self, buffer_size):
        # shuffle dataset
        self._dataset = self._dataset.shuffle(buffer_size = buffer_size)
        
    def split(self, ratio=[0.7,0.15,0.15], shuffle=True, shuffle_buffer_size = 5):
        # return splits
        dataset_size = len(self._inputs)
        if shuffle:
            self._shuffle_dataset(shuffle_buffer_size)
        if len(ratio) == 3:
            train_size = int(ratio[0]*dataset_size)
            val_size = int(ratio[1]*dataset_size)
            test_size = int(ratio[2]*dataset_size)
            self._dataset_train = self._dataset.take(train_size)
            self._dataset_test = self._dataset.skip(train_size)
            self._dataset_val = self._dataset.skip(test_size)
            self._dataset_test = self._dataset.take(test_size)
            return self._dataset_train, self._dataset_val, self._dataset_test
        else:
            train_size = int(ratio[0]*dataset_size)
            test_size = int(ratio[1]*dataset_size)
            self._dataset_train = self._dataset.take(train_size)
            self._dataset_test = self._dataset.skip(train_size)
            return self._dataset_train, self._dataset_test
    def plot(self):
        # Time delay vs angle at different channel
        fig = plt.figure(figsize = (20,20))
        ax = plt.axes()
        for i in range(len(self._channels)):
            ax.scatter(self._labels, self._inputs_subset[:,i], label="ch{}".format(self._channels[i]))
        plt.title("Time Delay vs. DOA for Each Channel")
        ax.set_xlabel('DOA (degree)', fontweight ='bold') 
        ax.set_ylabel('Time Delay (ms)', fontweight ='bold') 
        ax.legend()


# +
class Visualizer:
    def __init__(self, AA_geometry):
        self._AA_geometry_cart = AA_geometry
        self._AA_geometry_polar = np.empty(AA_geometry[:,:2].shape)
        self._AA_geometry_sphe = np.empty(AA_geometry.shape)
        self._cart2pol()
        self._cart2sphe()
    def cartesian2D(self, fig_size = (12,12)):
        fig = plt.figure(figsize = fig_size)
        ax = plt.axes()
        for i, channel in enumerate(self._AA_geometry_cart):
            ax.scatter(channel[0],channel[1], label = "ch{}".format(i))
            ax.text(channel[0], channel[1], '[%s]'%(str(i)))
        plt.title("Array Cartesian Top View")
        ax.set_aspect('equal', adjustable='box')
        ax.set_xlabel('X-axis', fontweight ='bold') 
        ax.set_ylabel('Y-axis', fontweight ='bold') 
        ax.margins(0.2)
        ax.legend()
        #plt.tight_layout()
    def cartesian3D(self, fig_size = (12,12)):
        fig = plt.figure(figsize = fig_size)
        ax = plt.axes(projection = "3d")
        for i, channel in enumerate(self._AA_geometry_cart):
            ax.scatter(channel[0],channel[1],channel[2], label = "ch{}".format(i))
            ax.text(channel[0], channel[1], channel[2], '[%s]'%(str(i)))
        plt.title("Array Cartesian 3D View")
        ax.set_xlabel('X-axis', fontweight ='bold') 
        ax.set_ylabel('Y-axis', fontweight ='bold') 
        ax.set_zlabel('Z-axis', fontweight ='bold')
        ax.legend()
    def polar(self, angle = None,fig_size = (14,14)):
        fig = plt.figure(figsize = fig_size)
        ax = plt.axes(projection = "polar")
        for i, channel in enumerate(self._AA_geometry_polar):
            ax.scatter(channel[1],channel[0], label = "ch{}".format(i))
            ax.text(channel[1], channel[0], '[%s]'%(str(i)))
        if angle:
            angle = angle * np.pi / 180
            ax.vlines(angle,0,0.12, colors = 'r')
        plt.title("Array Cartesian 3D View")
        ax.margins(0.2)
        ax.legend()
    def plot_dataset(self, dataset):
        input_dataset = []
        label_dataset = []
        for x,y in dataset:
            input_dataset.append(x)
            label_dataset.append(y)
        input_dataset = np.array(input_dataset)
        label_dataset = np.array(label_dataset)
        
        fig = plt.figure(figsize = (20,20))
        ax = plt.axes()
        for i in range(input_dataset.shape[1]):
            ax.scatter(label_dataset, input_dataset[:,[i]], label="ch{}".format(i))
        plt.title("Time Delay vs. DOA for Each Channel")
        ax.set_xlabel('DOA (degree)', fontweight ='bold') 
        ax.set_ylabel('Time Delay (ms)', fontweight ='bold') 
        ax.legend()
    def _cart2pol(self):
        for i, channel in enumerate(AA_Geometry):
            x = channel[0]
            y = channel[1]
            r = np.sqrt(x**2 + y**2)
            if x == 0:
                theta = np.pi/2 if y > 0 else -np.pi/2
            else:
                theta = np.arctan(y/x)# * 180 / np.pi
                theta = theta if x > 0 else theta + np.pi
            self._AA_geometry_polar[i][0] = r
            self._AA_geometry_polar[i][1] = theta
    def _cart2sphe(self):
        pass
        #for i in range(self._AA_geometry_cart.shape[1]):
        #    x = self._AA_geometry_cart[0][i] + 0.0001
        #    y = self._AA_geometry_cart[1][i] + 0.0001
        #    z = self._AA_geometry_cart[2][i] + 0.0001
        #    
        #    r = np.sqrt(x**2 + y**2 + z**2)
        #    theta = np.arctan(y/x)
        #    phi = np.arccos(z/r)
        #    self._AA_geometry_sphe[:,i] = np.array([r,theta,phi])
        
        
# -

class MyModel(tf.keras.Model):
    # Todo
    # Include: batch normalization
    def __init__(self, n_channels):
        super().__init__()
        self.net = tf.keras.Sequential([
            tf.keras.layers.Input(n_channels),
            tf.keras.layers.Dense(units=16, activation='relu'),
            tf.keras.layers.Dense(units=8, activation='relu'),
            tf.keras.layers.Dense(units=1, activation = 'linear')
        ])

    def call(self, inputs):
        return self.net(inputs)


class Trainer:
    def __init__(self):
        pass
    def optimizer_config(self):
        pass
    def loss_func_config(self):
        pass
    def metrics_config(self):
        pass
    def plot(self):
        pass
    def fit(self):
        pass
    


