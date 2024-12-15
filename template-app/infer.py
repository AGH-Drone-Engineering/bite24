import cv2
import tensorflow.lite as tflite


def main():
    model = tflite.Interpreter('model-fin/model_calibrated.tflite')
    model.allocate_tensors()

    cap = cv2.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        if not ret:
            break
        frame_original = frame

        frame = cv2.resize(frame, (240, 240), interpolation=cv2.INTER_AREA)
        cv2.imshow('frame_infer', frame)
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frame = (frame.astype('int32') - 128).astype('int8')
        frame = frame[None]

        model.set_tensor(model.get_input_details()[0]['index'], frame)
        model.invoke()
        mask = model.get_tensor(model.get_output_details()[0]['index'])

        mask = mask[0, :, :, 0]
        mask = (mask.astype('int32') + 128).astype('uint8')
        print(mask)
        mask = cv2.resize(mask, (frame_original.shape[1], frame_original.shape[0]), interpolation=cv2.INTER_NEAREST)

        cv2.imshow('frame', frame_original)
        cv2.imshow('mask', mask)
        cv2.waitKey(1)


if __name__ == '__main__':
    main()
