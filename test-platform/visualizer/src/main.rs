use std::{collections::VecDeque, fs::File, io::{BufRead, BufReader}, sync::{Arc, Mutex}, thread};
use std::cell::Cell;
use std::net::SocketAddr;
use eframe::Frame;
use egui::{Context, Vec2b, ViewportBuilder};
use std::time::Duration;
use egui_plot::{Line, Plot, PlotBounds, PlotPoints};
use serialport::SerialPort;
use tokio::net::TcpSocket;

pub const DEFAULT_CAPACITY: usize = 10_000;
pub const DEFAULT_BAUDRATE: u32 = 115_200;
pub const COMMON_BAUDRATES: [u32; 14] = [
    110, 150, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
];
#[derive(Debug)]
pub struct Data {
    data_points: usize,
    lc_all: VecDeque<DataRecord>,
    ps_all: VecDeque<DataRecord>,
    port_list: Vec<String>,
    current_port: Option<String>,
    current_baudrate: u32,
    serial_port: Option<Box<dyn SerialPort>>,
    request_stop: bool,
    latest_size: usize
}

#[derive(Debug)]
pub struct DataRecord {
    pub time: u64,
    pub value: f64,
}

#[derive(Default, serde::Serialize, serde::Deserialize, Debug)]
pub struct FileCsvLine {
    pub typ: String,
    pub time: u64,
    pub value: f64,
}

pub struct VisualizerApp {
    data: Arc<Mutex<Data>>,
}

impl VisualizerApp {
    pub fn new(_cc: &eframe::CreationContext<'_>) -> Self {
        let data = Data{
            data_points: DEFAULT_CAPACITY,
            lc_all: VecDeque::with_capacity(DEFAULT_CAPACITY),
            ps_all: VecDeque::with_capacity(DEFAULT_CAPACITY),
            port_list: scan_serial_ports(),
            current_port: None,
            current_baudrate: DEFAULT_BAUDRATE,
            serial_port: None,
            request_stop: false,
            latest_size: 1000,
        };
        let mut slf: Self = VisualizerApp {
            data: Arc::new(Mutex::new(data)),
        };
        slf.spawn_input_consumer();

        slf
    }

    fn spawn_input_consumer(&mut self) {
        let data = Arc::clone(&self.data);
        std::thread::spawn(move || Self::read_file(data));
    }

    fn read_file(data_arc: Arc<Mutex<Data>>) {
        // let file = File::open("/home/patyg/data.csv").unwrap();
        let (port, boudrate) = {
            let data = data_arc.lock().unwrap();
            (data.current_port.clone(), data.current_baudrate)
        };
        if port.is_none() {
            eprintln!("[PORT][ERR]### No port selected");
            return;
        }
        let port = port.unwrap();

        // let serial_port = serialport::new(port, boudrate)
        //     .timeout(Duration::from_millis(1000))
        //     .open();
        //
        // if let Err(e) = serial_port {
        //     eprintln!("[PORT][ERR]### Failed to open port: {}", e);
        //     return;
        // }
        // let serial_port = serial_port.unwrap();

        let file = File::open("/tmp/test_data");
        if let Err(e) = file {
            eprintln!("[PORT][ERR]### Failed to open test file: {}", e);
            return;
        }
        let file = file.unwrap();

        // let mut reader = BufReader::new(serial_port);
        let mut reader = BufReader::new(file);
        let mut buffer = String::new();

        // reader.read_line(&mut buffer).unwrap();
        // current system time in milliseconds
        let mut last_time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_millis();

        let mut x = 0;
        loop {
            buffer.clear();
            if (data_arc.lock().unwrap().request_stop) { // TODO better communication
                eprintln!("[PORT][INF]### Request to stop reading from port.");
                break;
            }

            match reader.read_line(&mut buffer) {
                Ok(0) => {
                    // EOF reached, exit the loop
                    eprintln!("[PORT][INF]### End of file reached.");
                    break;
                }
                Ok(_) => {
                    Self::process_input_line(&data_arc, &buffer);
                    let now = std::time::SystemTime::now()
                        .duration_since(std::time::UNIX_EPOCH)
                        .unwrap()
                        .as_millis();
                    if now - last_time > 33 {
                        last_time = now;
                        thread::sleep(Duration::from_millis(10));
                        println!("free {}", x);
                        x+= 1;
                    }
                }
                Err(e) => {
                    eprintln!("[PORT][ERR]### Failed to read line: {}", e);
                    break;
                }
            }
        }
        let mut data = data_arc.lock().unwrap();
        data.request_stop = false;
        data.serial_port = None;
    }

    fn process_input_line(data_arc: &Arc<Mutex<Data>>, line: &String) {
        //println!("[PORT][PROCESSING]### {}", line);
        // println!("[PORT][PROCESSING]### {}", line);
        if line.is_empty() || (!line.starts_with("lc,") && !line.starts_with("ps,")) {
            return;
        }

        let parts = line.split(',').collect::<Vec<&str>>();
        let t = parts[0];
        let time: u64 = parts[1].parse().unwrap();
        let value: f64 = parts[2].trim().parse().unwrap();

        let csv_line = FileCsvLine {
            typ: t.to_string(),
            time: time,
            value: value,
        };
        // println!("[PORT][  RECORD  ]### Type: {}, Time: {}, Value: {}", csv_line.typ, csv_line.time, csv_line.value);

        Self::process_file_data(data_arc, csv_line);
    }


    fn process_file_data(data_arc: &Arc<Mutex<Data>>, csv_line: FileCsvLine) {

        let mut data = data_arc.lock().unwrap();
        // eprintln!("{:?}", measurements);
        let record = DataRecord {time: csv_line.time, value: csv_line.value};
        //println!("[PORT][  RECORD  ]### Type: {}, Time: {}, Value: {}", csv_line.typ, record.time, record.value);
        if csv_line.typ == "lc" {
            data.lc_all.push_back(record);
            if data.lc_all.len() > data.data_points {
                data.lc_all.pop_front();
            }
        } else if csv_line.typ == "ps" {
            data.ps_all.push_back(record);
            if data.ps_all.len() > data.data_points {
                data.ps_all.pop_front();
            }
        } else {
            eprintln!("[PORT][ERR]### Unknown type: {}", csv_line.typ);
        }
    }

    fn draw_ui(&mut self, ctx: &Context) {
        let data_arc = Arc::clone(&self.data);
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.vertical(|ui| {
                ui.horizontal(|ui| {
                    let data : &mut Data = &mut data_arc.lock().unwrap();
                    ui.label(format!("Data Points: {}", data.data_points));
                    ui.label(format!("LC Count: {}", data.lc_all.len()));
                    ui.label(format!("PS Count: {}", data.ps_all.len()));
                    ui.separator();
                    let mut latest_size = data.latest_size.to_string();
                    if ui.text_edit_singleline(&mut latest_size).changed() {
                       data.latest_size = latest_size.trim().parse().unwrap_or(data.latest_size);
                    }
                    draw_port_selector(ui, data, || self.spawn_input_consumer());
                })
            });

            let data : &mut Data = &mut data_arc.lock().unwrap();
            if !data.lc_all.is_empty() {
                draw_chart(ui, data);
            } else {
                ui.label("No lc/ps data Available");
            }
        });
    }

}

impl eframe::App for VisualizerApp {
    fn update(&mut self, ctx: &Context, _frame: &mut Frame) {
        self.draw_ui(ctx);
        ctx.request_repaint_after(Duration::from_millis(33));
    }
}


fn draw_chart(ui: &mut egui::Ui, data: &Data) {

    egui_extras::StripBuilder::new(ui)
        .sizes(egui_extras::Size::relative(2f32.recip()), 2)
        .vertical(|mut vstrip| {
            vstrip.cell(|ui| {
                egui_extras::StripBuilder::new(ui)
                    .sizes(egui_extras::Size::relative(2f32.recip()), 2)
                    .horizontal(|mut hstrip| {
                        hstrip.cell(|ui| {
                            let lc_points: Vec<[f64; 2]> = data.lc_all.iter()
                                .skip(data.lc_all.len().checked_sub(data.latest_size).unwrap_or(0))
                                .map(|p| [p.time as f64 / 1000.0, p.value])
                                .collect();
                            draw_plot(ui, "latest lc", vec![
                                PlotData { name: "Load Cell", points: lc_points },
                            ]);
                        });
                        hstrip.cell(|ui| {
                            let ps_points: Vec<[f64; 2]> = data.ps_all.iter()
                                .skip(data.ps_all.len().checked_sub(data.latest_size).unwrap_or(0))
                                .map(|p| [p.time as f64 / 1000.0, p.value])
                                .collect();
                            draw_plot(ui, "latest ps", vec![
                                PlotData { name: "Pressure Sensor", points: ps_points }
                            ]);
                        });
                    });
            });
            vstrip.cell(|ui| {
                let lc_points: Vec<[f64; 2]> = data.lc_all.iter()
                    .map(|p| [p.time as f64 / 1000.0, p.value])
                    .collect();
                let ps_points: Vec<[f64; 2]> = data.ps_all.iter()
                    .map(|p| [p.time as f64 / 1000.0, p.value])
                    .collect();

                draw_plot(ui,  "All", vec![
                    PlotData { name: "Load Cell", points: lc_points },
                    PlotData { name: "Pressure Sensor", points: ps_points }
                ]);
            })
        });
        //.x_axis_formatter(|value, _| timestamp_formatter(value));
}

struct PlotData<'a> {
    name: &'a str,
    points: Vec<[f64; 2]>
}
fn draw_plot(ui: &mut egui::Ui, plot_name: &str, lines: Vec<PlotData>) {

    Plot::new(plot_name)
        // .include_y(0)
        // .include_y(100)
        .show(ui, |plot_ui| {
            for plot_data in lines {
                let line = Line::new(plot_data.name, PlotPoints::from(plot_data.points));
                //     .name(plot_data.name);
                plot_ui.line(line);
            }
            plot_ui.set_auto_bounds(Vec2b::new(true, true));
            // let ps_line = Line::new(PlotPoints::from_iter(
            //     data.ps_all.iter().map(|p| [p.time as f64 / 1000.0, p.value]),
            // ))
            //     .name("Pressure Sensor");
            //
            // plot_ui.line(lc_line);
            // plot_ui.line(ps_line);
            // plot_ui.set_auto_bounds(Vec2b::new(true, true));
        });
}

fn draw_port_selector<F>(ui: &mut egui::Ui, data: &mut Data, mut start_listening: F) where F: FnMut() {
    const EMPTY_PORT: &str = "Select Port";
    let ports = data.port_list.clone();
    let mut selected_port = data.current_port.clone();
    let mut selected_baudrate = data.current_baudrate;
    egui::ComboBox::from_id_source("port_selector")
        .selected_text(selected_port.clone().unwrap_or(EMPTY_PORT.to_string()))
        .show_ui(ui, |ui| {
            let mut value_changed = ui.selectable_value(&mut selected_port, None, EMPTY_PORT.to_string()).clicked();
            for port in ports {
                value_changed |= ui.selectable_value(&mut selected_port, Some(port.clone()), port).clicked();
            }
            if value_changed {
                data.current_port = selected_port.clone();
                println!("[ UI ][INF] Selected port: {:?}", selected_port);
            }
        });

    if ui.button("⟳").clicked() {
        data.port_list = scan_serial_ports();
    }

    egui::ComboBox::from_id_source("baudrate_selector")
        .selected_text(selected_baudrate.to_string())
        .show_ui(ui, |ui| {
            for baudrate in COMMON_BAUDRATES {
                if ui.selectable_value(&mut selected_baudrate, baudrate, baudrate.to_string()).clicked() {
                    data.current_baudrate = baudrate;
                    println!("Selected baudrate: {}", baudrate);
                }
            }
        });

    if data.serial_port.is_none() {
        if ui.button("Connect").clicked() {
            if let Some(port) = &data.current_port {
                eprintln!("[PORT][INF]### Connecting to port: {} at baudrate: {}", port, data.current_baudrate);
                // Here you would typically start reading from the serial port
                // For now, we just print the selected port and baudrate
                start_listening()
            } else {
                eprintln!("[PORT][ERR]### No port selected");
            }
        }
    }
    else if ui.button("D̶i̶s̶c̶o̶n̶n̶e̶c̶t̶").clicked() {
        eprintln!("[PORT][INF]### Disconnecting from port (TODO)");
        // TODO: Disconnect logic
        data.request_stop = true;
    }

}

fn scan_serial_ports() -> Vec<String> {
    let ports = serialport::available_ports().unwrap_or_default();
    ports.into_iter().map(|p| p.port_name).collect()
}

fn main() -> eframe::Result<()> {
    let options = eframe::NativeOptions {
        viewport: ViewportBuilder::default()
            .with_inner_size(egui::Vec2::new(1280.0, 720.0)),
        ..eframe::NativeOptions::default()
    };

    eframe::run_native(
        "Flight Visualizer",
        options,
        Box::new(|cc| Ok(Box::new(VisualizerApp::new(cc)))),
    )
}