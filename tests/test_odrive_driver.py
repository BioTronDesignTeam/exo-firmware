"""Exercise the real CAN driver against a host HAL stub, without motor hardware."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ODriveDriverTests(unittest.TestCase):
    def test_node_addressing_frames_and_initialization(self):
        with tempfile.TemporaryDirectory() as directory:
            for node in (0, 7, 62):
                with self.subTest(node=node):
                    executable = pathlib.Path(directory) / f"odrive-{node}"
                    subprocess.run([
                        "g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        f"-DODRIVE_CAN_NODE_ID={node}",
                        "-I" + str(ROOT / "tests/odrive_stubs"),
                        "-I" + str(ROOT / "Drivers/Peripherals/Motors/OdriveS1"),
                        "-I" + str(ROOT / "Drivers/Peripherals/Communication/CAN_Simple"),
                        str(ROOT / "Drivers/Peripherals/Motors/OdriveS1/odriveS1.cpp"),
                        str(ROOT / "tests/odrive_driver_test.cpp"),
                        "-o", str(executable),
                    ], check=True)
                    subprocess.run([str(executable)], check=True)

    def test_motor_start_stop_and_faults(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = pathlib.Path(directory) / "motor-controller"
            subprocess.run([
                "g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-I" + str(ROOT / "tests/odrive_stubs"),
                "-I" + str(ROOT / "Tasks/Inc"),
                "-I" + str(ROOT / "Drivers/Peripherals/Motors/OdriveS1"),
                "-I" + str(ROOT / "Drivers/Peripherals/Communication/CAN_Simple"),
                str(ROOT / "Drivers/Peripherals/Motors/OdriveS1/odriveS1.cpp"),
                str(ROOT / "Tasks/Src/motor_controller.cpp"),
                str(ROOT / "tests/motor_controller_test.cpp"),
                "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
