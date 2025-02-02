import tensorflow as tf
import keras
import numpy as np
import torch
from torch.utils.data import DataLoader

from piwo_dataset import PiwoDataset


IMG_SIZE = 240
MASK_SIZE = 15


def export(model, dataset):
    saved_model_dir = 'saved_model'
    model.export(saved_model_dir)

    def representative_dataset():
        for i in range(len(dataset)):
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
    backbone = keras.Model(backbone.input, backbone.layers[115].output)
    model = keras.Sequential([
        keras.Input(shape=(IMG_SIZE, IMG_SIZE, 3)),
        backbone,
        keras.layers.Conv2D(1, 3, activation='sigmoid', padding='same'),
    ])
    return model


# def collate_fn(examples):
#     images, masks = zip(*examples)
#     images = torch.stack([torch.from_numpy(image) for image in images])
#     masks = torch.stack([torch.from_numpy(mask) for mask in masks])
#     return images, masks


def main():
    # keras.mixed_precision.set_global_policy("mixed_bfloat16")

    print("Loading dataset...")
    train_dataset = PiwoDataset('data/annotations/instances_train2017.json', 'data/train2017', IMG_SIZE, MASK_SIZE, augment=True)
    calib_dataset = PiwoDataset('data/annotations/instances_train2017.json', 'data/train2017', IMG_SIZE, MASK_SIZE, augment=False)
    print("Dataset loaded")

    epochs = 250
    batch_size = 256

    train_loader = DataLoader(
        train_dataset,
        batch_size=batch_size,
        shuffle=True,
        # num_workers=12,
    )

    model = build_model()

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
        callbacks=[
            keras.callbacks.ModelCheckpoint('model-{epoch:03d}.keras'),
        ]
    )

    model.save('model.keras')
    export(model, calib_dataset)


if __name__ == '__main__':
    main()
