# MTL_BaroTactile

Open-source barometric tactile sensor.

## Publication

This work is based on our paper:
[MTL_BaroTactile](https://ieeexplore.ieee.org/abstract/document/10770605)

## Repository Structure

- **[Code](Code/)** - Arduino code for sensor (MPL3115A2 Barometer) and testing rig
- **[Hardware](Hardware/)** - PCB designs, 3D CAD files, and testing rig hardware
  - **[PCB/MTL_BaroTactile_MPL3115A2](Hardware/PCB/MTL_BaroTactile_MPL3115A2/)** - Main sensor version (used in paper)
  - **[PCB/MTL_BaroTactile_MS5840](Hardware/PCB/MTL_BaroTactile_MS5840/)** - Alternative version with MS5840
  - **[Mold](Hardware/Mold/)** - 3D CAD files for sensor fabrication
  - **[TestingRig](Hardware/TestingRig/)** - Testing equipment designs

## Hardware Versions

### MPL3115A2 Version (Paper Implementation)
Primary sensor design using MPL3115A2 barometric pressure sensor.

### MS5840 Version (Alternative)
Alternative design using MS5840 barometer - recommended when MPL3115A2 stock is limited. The MS5840 provides comparable performance for barometric tactile sensing.


## Citation

If you use this work in your research, please cite our paper:

```bibtex
@ARTICLE{10770605,
  author={Hou, Jian and Zhou, Xin and Spiers, Adam},
  journal={IEEE Transactions on Robotics}, 
  title={Location and Orientation Super-Resolution Sensing With a Cost-Efficient and Repairable Barometric Tactile Sensor}, 
  year={2025},
  volume={41},
  pages={729-741},
  doi={10.1109/TRO.2024.3508315}
}
```
