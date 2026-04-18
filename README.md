# ChessBot V1
<img width="2993" alt="ChessBot V1" src="https://github.com/user-attachments/assets/f9259ac6-0bac-4ade-8fab-4b74a4baa816" />

This repository includes the PCB (gerber), CAD (.step), code (.ino and .py), and printing (.stl) files for ChessBot V1, an Arduino powered self-playing chess robot. 

A video overview of ChessBot V1 can be found here: [https://youtu.be/M-bemH251Hs](https://youtu.be/tLgXvUgsYmw)

The design has been updated slightly since the video, and the board coloring has been corrected to make A1 a black tile. The code is functional, but there is plenty of room for improvement. Feel free to modify the design and make it your own. Happy building! 

## Bill of Materials
| Component | Specs | Qty | Link |
|:---------:|:-----:|:---:|:-----|
| ESP32 Dev Board | 38-pin version | 1 | https://www.amazon.com/dp/B0CNYK7WT2?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_14
| Electromagnet | 5V, 50N, 25mm diameter, 20mm height | 1 | |
| Motor Driver | DRV8872 | 1 |  |
| Hall Sensor | AH49FNTR-G1 | 64 |  |
| Momentary Switch | 6mm x 6mm | 2 |  |
| Barrel Jack Extension | 5.5x2.1mm | 1 | https://www.amazon.com/dp/B0D9B7WR23?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_1&th=1 |
| Power Supply | 6V, 36W, 5.5x2.1mm Barrel Jack | 1 | https://www.amazon.com/dp/B07N18XN84?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_3 |
| Stepper Driver | TMC2208 (or similar) | 2 |  |
| Stepper Motor | Nema 17, 22.5mm body height | 2 |  |
| Linear Rod | 8mm x 250mm | 2 |  |
| Linear Rod | 8mm x 265mm | 2 |  |
| Sleeve Bearing | 8mm ID, 10mm OD, 15mm length | 2 | https://www.amazon.com/dp/B07SLYD55F?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_4&th=1 |
| Sleeve Bearing | 8mm ID, 10mm OD, 6mm length | 6 | https://www.amazon.com/dp/B07SN3RBKW?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_1&th=1 |
| Timing Belt | 2GT 6mm | 5 meters |  |
| Belt Pulley | 2GT, 20 Teeth, 5mm ID | 2 |  |
| Idler Pulley | 3mm ID, 9.7mm OD, 13mm flange diameter, 8.5mm height | 12 |  |
| Neodymium Magnets | 8mm x 2mm | 32 |  |
| Button Head Screw | M3 x 30mm | 10 |  |
| Button Head Screw | M3 x 22mm | 2 |  |
| Button Head Screw | M3 x 12mm | 6 |  |
| Button Head Screw | M3 x 6mm | 8 |  |
| Washer | M3, 0.5mm thick | 50 |  |
| Threaded Insert | M3, 4mm thick | 24 |  |
| Rubber Feet | 3mm ID, 12mm OD, 6mm height | 4 |  |
