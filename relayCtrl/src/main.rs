use std::collections::HashMap;
use std::fs;
use std::time::Duration;
use anyhow::Result;
use log::{error, warn};
use serde::Deserialize;
use serialport::SerialPort;
use structopt::StructOpt;

//#[derive(StructOpt)]
//struct Opt {
//    relay_name: String,
//    action: String,
//}


#[derive(StructOpt)]
#[structopt(name = "relay_control", about = "Control relays through a serial connection.")]
struct Opt {
    /// Custom path to the configuration file
    #[structopt(short, long, default_value = "config.toml")]
    config: String,

    #[structopt(help = "Name of the relay to control.")]
    relay_name: String,

    #[structopt(help = "Action to perform on the relay.\nAvailable actions: set, reset, toggle, on, off")]
    action: String,
}

#[derive(Debug, Deserialize)]
struct Config {
    relay: String,
    baudrate: u32,
    relays: HashMap<String, u8>,
}

impl Config {
    fn from_file(file_path: &str) -> Result<Self> {
        let config_str = fs::read_to_string(file_path)?;
        let config: Config = toml::from_str(&config_str)?;
        Ok(config)
    }
}

struct Control {
    relays: HashMap<String, u8>,
    connection: Connection,
}

impl Control {
    pub fn new(config: &Config) -> Result<Self> {
        let connection = Connection::new(&config.relay, config.baudrate)?;
        let control = Control {
            relays: config.relays.clone(),
            connection,
        };
        Ok(control)
    }

    pub fn toggle_relay(&mut self, relay_name: &str) {
        if let Some(&pin) = self.relays.get(relay_name) {
            self.connection.send_command(8, pin);
        } else {
            warn!("Toggle unknown relay: {}", relay_name);
        }
    }
}

struct Connection {
    serial_port: Box<dyn SerialPort>,
}

impl Connection {
    pub fn new(port: &str, baudrate: u32) -> Result<Self> {
        let mut serial_port = serialport::new(port, baudrate).open().expect("Failed to open port");
        serial_port.set_timeout(Duration::from_secs(5))?;

        Ok(Self { serial_port })
    }

    pub fn send_command(&mut self, command: u8, data: u8) {
        let cmd = [command, 0, data, command ^ data];
        if let Err(e) = self.serial_port.write_all(&cmd) {
            error!("Send command '{}' failed: {}", command, e);
        }

        let mut resp = [0u8; 4];
        if let Err(e) = self.serial_port.read_exact(&mut resp) {
            error!("Read resp '{}' failed: {}", command, e);
        }

        let xor = resp[0] ^ resp[1] ^ resp[2];
        if resp[3] != xor {
            error!("Read resp '{}' failed: {:?}", command, resp);
        }
    }
}

fn main() -> Result<()> {
    env_logger::init();
    let opt = Opt::from_args();
    let config = Config::from_file(&opt.config)?;



    let mut control = Control::new(&config)?;


    match opt.action.as_str() {
        "set" => control.toggle_relay(&opt.relay_name),
        "reset" => control.toggle_relay(&opt.relay_name),
        "toggle" => control.toggle_relay(&opt.relay_name),
        "on" => control.toggle_relay(&opt.relay_name),
        "off" => control.toggle_relay(&opt.relay_name),
        _ => {
            eprintln!("Invalid action. Please use set, reset, toggle, on, or off.");
            std::process::exit(1);
        }
    }

    Ok(())
}

