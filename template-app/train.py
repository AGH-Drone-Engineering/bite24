import keras
import tensorflow as tf
import numpy as np
import torch
from torch.utils.data import DataLoader

from piwo_dataset import PiwoDataset


IMG_SIZE = 240
MASK_SIZE = 17


def export(model, dataset):
    saved_model_dir = 'saved_model'
    model.export(saved_model_dir)

    def representative_dataset():
        for i in range(2000):
            img, ann = dataset[i]
            yield [img[None]]

    converter = tf.lite.TFLiteConverter.from_saved_model(saved_model_dir)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    tflite_quant_model = converter.convert()

    with open('model_calibrated.tflite', 'wb') as f:
        f.write(tflite_quant_model)


def build_model():
    backbone = keras.applications.MobileNetV2(
        input_shape=(IMG_SIZE, IMG_SIZE, 3),
        include_top=False,
        pooling=None,
        weights='imagenet',
        alpha=0.35,
    )
    backbone = keras.Model(backbone.input, backbone.layers[119].output)
    model = keras.Sequential([
        keras.Input(shape=(IMG_SIZE, IMG_SIZE, 3)),
        backbone,
        keras.layers.Conv2D(1, 1, activation='sigmoid'),
    ])
    return model


def collate_fn(examples):
    images, masks = zip(*examples)
    images = torch.stack([torch.from_numpy(image) for image in images])
    masks = torch.stack([torch.from_numpy(mask) for mask in masks])
    return images, masks


def main():
    print("Loading dataset...")
    train_dataset = PiwoDataset('data/annotations/instances_train2017.json', 'data/train2017', IMG_SIZE, MASK_SIZE)
    print("Dataset loaded")

    model = build_model()

    epochs = 100
    batch_size = 512

    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True, collate_fn=collate_fn, num_workers=8)

    model.compile(
        optimizer=keras.optimizers.Adam(
            learning_rate=keras.optimizers.schedules.CosineDecay(
                initial_learning_rate=3e-4,
                decay_steps=epochs * len(train_loader),
            ),
            clipnorm=1.0,
        ),
        loss='binary_crossentropy',
        metrics=['accuracy', 'precision', 'recall', 'mae'],
    )

    model.fit(
        train_loader,
        epochs=epochs,
    )

    model.save('model.keras')
    export(model, train_dataset)


if __name__ == '__main__':
    main()
