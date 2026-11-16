                           # Main application
├── haarcascade_frontalface_default.xml   #
```

## How It Works

1. Open the webcam with `VideoCapture`.
2. Load the Haar Cascade model.
3. Convert each frame to grayscale.
4. Detect faces with `detectMultiScale()`.
5. Draw a rectangle around each face and display the count.

## Troubleshooting

| Problem | Fix |
| --- | --- |
| Black screen or "camera not found" | Change the camera index from `1` to `0` |
| Cascade file fails to load | Run the app from the folder containing the `.xml` file |
| `opencv4` not found by pkg-config | Install OpenCV dev packages, e.g. `sudo apt install libopencv-dev` |

## Roadmap

- [ ] Eye and smile detection
- [ ] Deep-learning face detection (e.g. DNN-based models)
- [ ] Save snapshots
- [ ] Video recording

## Contributing

Contributions are welcome. Fork the repo, create a feature branch, and open a pull request.

## License

Released under the [MIT License](LICENSE).
