from torch.utils.data import Dataset
import numpy as np
import albumentations as A
import os
import json
import cv2
from tqdm import tqdm


BOTTLE_ID = 44


class PiwoDataset(Dataset):
    def __init__(self, ann_path: str, img_path: str, img_size: int, mask_size: int):
        self.img_size = img_size
        self.mask_size = mask_size

        print("Loading annotations...")
        with open(ann_path, 'r') as f:
            coco_annotations = json.load(f)
        print("Annotations loaded")

        image_id_to_filename = {}
        image_filename_to_id = {}
        for img in tqdm(coco_annotations['images']):
            image_id_to_filename[img['id']] = os.path.join(img_path, img['file_name'])
            image_filename_to_id[os.path.join(img_path, img['file_name'])] = img['id']

        image_filenames = list(image_id_to_filename.values())
        image_filenames.sort()

        image_id_to_annotations = {}

        for coco_ann in tqdm(coco_annotations['annotations']):
            ann = image_id_to_annotations.get(coco_ann['image_id'], None)

            if coco_ann['category_id'] == BOTTLE_ID:
                if ann is not None:
                    ann = [*ann, coco_ann['bbox']]
                else:
                    ann = [coco_ann['bbox']]

            image_id_to_annotations[coco_ann['image_id']] = ann

        image_annotations = []
        for img_filename in tqdm(image_filenames):
            # img_id = [img_id for img_id, filename in image_id_to_filename.items() if filename == img_filename][0]
            img_id = image_filename_to_id[img_filename]
            ann = image_id_to_annotations.get(img_id, None)
            image_annotations.append(ann)

        good_idxs = [idx for idx in range(len(image_filenames)) if image_annotations[idx] is not None]
        print('Good examples:', len(good_idxs))

        self.x = [image_filenames[idx] for idx in good_idxs]
        self.y = [image_annotations[idx] for idx in good_idxs]

        self.transform = A.Compose([
            A.RandomResizedCrop(size=self.img_size),
            A.HorizontalFlip(),
            A.RandomBrightnessContrast(brightness_limit=0.1),
            A.ShotNoise((0.02, 0.04)),
            A.Normalize(mean=(0.5, 0.5, 0.5), std=(0.5, 0.5, 0.5)),
        ], bbox_params=A.BboxParams(format='coco', label_fields=['category_id']))

    def __len__(self):
        return len(self.x)

    def __getitem__(self, idx):
        image_path = self.x[idx]
        bboxes = self.y[idx]

        image = cv2.imread(image_path)
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

        transformed = self.transform(image=image, bboxes=bboxes, category_id=[BOTTLE_ID] * len(bboxes))
        image = transformed['image']
        bboxes = transformed['bboxes']

        mask = np.zeros((self.mask_size, self.mask_size, 1), dtype=np.float32)

        for bbox in bboxes:
            x, y, w, h = bbox
            x /= image.shape[1]
            y /= image.shape[0]
            w /= image.shape[1]
            h /= image.shape[0]
            x = int(x * self.mask_size)
            y = int(y * self.mask_size)
            w = int(w * self.mask_size)
            h = int(h * self.mask_size)
            cv2.rectangle(mask, (x, y), (x + w, y + h), 1, cv2.FILLED)

        return image, mask
