import os
os.environ["OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS"] = "0"
import cv2
from ultralytics import YOLO

# load trained balloon model
model = YOLO("balloon.pt")

# start cam 0 is default
cam = cv2.VideoCapture(0)

print("started exit q")

# memory to prevent stutter when moving fast
smooth_tx, smooth_ty = None, None
lost_count = 0
max_lost_frames = 10  # keep tracking for 10 frames if lost during fast move

while True:
    ret, frame = cam.read()
    if not ret:
        print("no visual found")
        break

    # mirror cam
    frame = cv2.flip(frame, 1)

    # find center of screen
    h, w, _ = frame.shape
    cx = w // 2
    cy = h // 2

    # use track() with lower conf (0.25) so motion blur doesnt drop target
    results = model.track(frame, persist=True, conf=0.25, iou=0.5, verbose=False)
    boxes = results[0].boxes

    # draw center crosshair
    cv2.line(frame, (cx - 15, cy), (cx + 15, cy), (255, 255, 255), 2)
    cv2.line(frame, (cx, cy - 15), (cx, cy + 15), (255, 255, 255), 2)

    if len(boxes) > 0:
        lost_count = 0
        # pick best confidence balloon
        best_box = max(boxes, key=lambda b: float(b.conf[0]))
        x1, y1, x2, y2 = map(int, best_box.xyxy[0])
        conf = float(best_box.conf[0])

        raw_tx = (x1 + x2) // 2
        raw_ty = (y1 + y2) // 2

        # smooth position to stop motor jitter/stutter
        if smooth_tx is None:
            smooth_tx, smooth_ty = raw_tx, raw_ty
        else:
            smooth_tx = int(0.6 * smooth_tx + 0.4 * raw_tx)
            smooth_ty = int(0.6 * smooth_ty + 0.4 * raw_ty)

        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(frame, f"balloon {conf:.2f}", (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    else:
        lost_count += 1
        if lost_count > max_lost_frames:
            smooth_tx, smooth_ty = None, None

    # if we have a target (either seen now or remembered from last few frames)
    if smooth_tx is not None and smooth_ty is not None:
        cv2.circle(frame, (smooth_tx, smooth_ty), 6, (0, 0, 255), -1)
        cv2.line(frame, (cx, cy), (smooth_tx, smooth_ty), (0, 255, 255), 2)

        dx = smooth_tx - cx
        dy = smooth_ty - cy

        if abs(dx) < 35 and abs(dy) < 35:
            cv2.putText(frame, "LOCKED - FIRE", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        else:
            cv2.putText(frame, f"dx: {dx} dy: {dy}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("cam", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cam.release()
cv2.destroyAllWindows()
