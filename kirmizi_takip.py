import os
os.environ["OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS"] = "0"
import cv2
import numpy as np

# start cam 0 is default
cam = cv2.VideoCapture(0)

print("started exit q")

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

    # convert to hsv for red detection
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # red color ranges
    low_red1 = np.array([0, 120, 70])
    high_red1 = np.array([10, 255, 255])
    low_red2 = np.array([170, 120, 70])
    high_red2 = np.array([180, 255, 255])

    mask1 = cv2.inRange(hsv, low_red1, high_red1)
    mask2 = cv2.inRange(hsv, low_red2, high_red2)
    mask = mask1 + mask2

    # clean noise
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # find contours
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # draw center crosshair
    cv2.line(frame, (cx - 15, cy), (cx + 15, cy), (255, 255, 255), 2)
    cv2.line(frame, (cx, cy - 15), (cx, cy + 15), (255, 255, 255), 2)

    if len(contours) > 0:
        # get biggest red object
        c = max(contours, key=cv2.contourArea)
        if cv2.contourArea(c) > 500:
            ((x, y), r) = cv2.minEnclosingCircle(c)
            tx, ty, r = int(x), int(y), int(r)

            # draw target and line to center
            cv2.circle(frame, (tx, ty), r, (0, 255, 0), 2)
            cv2.circle(frame, (tx, ty), 5, (0, 0, 255), -1)
            cv2.line(frame, (cx, cy), (tx, ty), (0, 255, 255), 2)

            # calculate error
            dx = tx - cx
            dy = ty - cy

            if abs(dx) < 35 and abs(dy) < 35:
                cv2.putText(frame, "LOCKED - FIRE", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
            else:
                cv2.putText(frame, f"dx: {dx} dy: {dy}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("cam", frame)
    cv2.imshow("mask", mask)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cam.release()
cv2.destroyAllWindows()
