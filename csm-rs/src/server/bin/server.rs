use async_std::fs::File;
use async_std::io::ReadExt;
use async_std::io::WriteExt;
use async_std::net::TcpListener;
use async_std::net::TcpStream;
use async_std::net::ToSocketAddrs;
use async_std::net::Shutdown;
use async_std::stream::StreamExt;
use async_std::sync::Arc;
use async_std::sync::Mutex;
use async_std::task;
use async_std::task::JoinHandle;
use signal_hook::consts::signal::{SIGINT, SIGTERM, SIGUSR1};//, SIGUSR2};
use signal_hook_async_std::Signals;

use crate::cmd_input::CmdInput;
use crate::cmd_parser::CmdParser;

pub struct Server {
    socket: TcpListener,
    parser: Arc<Mutex<CmdParser>>,
    uart: Arc<Mutex<File>>,
    run: Arc<Mutex<bool>>
}

impl Server {
    pub async fn new<'a>(addr: impl ToSocketAddrs, base_path: &'a str, uart_path: &'a str, custom_path: Option<Vec<&'a str>>) -> Result<Server, &'a str> {
        match TcpListener::bind(addr).await {
            Ok(socket) => {
                match CmdParser::new(base_path) {
                    Ok(mut parser) => {
                        match custom_path {
                            Some(paths) => {
                                for path in paths {
                                    parser.add_custom_cmd(path);
                                }

                                match File::open(uart_path).await {
                                    Ok(uart) => Ok(Server {
                                        socket,
                                        parser: Arc::new(Mutex::new(parser)),
                                        uart: Arc::new(Mutex::new(uart)),
                                        run: Arc::new(Mutex::new(true))
                                    }),
                                    _ => Err("Server::new : invalid uart path")
                                }
                            },
                            None => {
                                match File::open(uart_path).await {
                                    Ok(uart) => Ok(Server {
                                        socket,
                                        parser: Arc::new(Mutex::new(parser)),
                                        uart: Arc::new(Mutex::new(uart)),
                                        run: Arc::new(Mutex::new(true))
                                    }),
                                    _ => Err("Server::new : invalid uart path")
                                }
                            }
                        }
                    },
                    _ => Err("Server::new : invalid base command directory")
                }
            },
            _ => Err("Server::new : unable to bind server to specified IP address")
        }
    }

    pub async fn run(&mut self) {
        match Signals::new(&[SIGINT, SIGTERM]) {
            Ok(mut signals) => {
                let run = Arc::clone(&self.run);
                let signal_handle = signals.handle();

                let sig_task_handle = task::spawn(async move {
                    while let Some(signal) = signals.next().await {
                        match signal {
                            SIGINT => {
                                let mut run_guard = run.lock().await;
                                *run_guard = false;
                                break;
                            },
                            SIGTERM => {
                                let mut run_guard = run.lock().await;
                                *run_guard = false;
                                break;
                            },
                            _ => unreachable!()
                        }
                    }
                });

                let client_task_count = 10;
                let mut client_task_handles: Vec<JoinHandle<()>> = Vec::with_capacity(client_task_count);
                let mut client_task_statuses: Vec<Arc<Mutex<(bool, bool)>>> = Vec::with_capacity(client_task_count);
                let mut client_task_addresses: Vec<Arc<Mutex<Option<TcpStream>>>> = Vec::with_capacity(client_task_count);

                for i in 0..client_task_count {
                    let task_parser_ref = Arc::clone(&self.parser);

                    client_task_statuses.push(Arc::new(Mutex::new((true, false))));
                    client_task_addresses.push(Arc::new(Mutex::new(None)));

                    let task_status = Arc::clone(&client_task_statuses[i]);
                    let task_address = Arc::clone(&client_task_addresses[i]);
                    let task_uart = Arc::clone(&self.uart);

                    client_task_handles.push(task::spawn(async move {
                        match Signals::new(&[SIGINT, SIGTERM, SIGUSR1]) {
                            Ok(mut task_signals) => {
                                let task_status_clone = Arc::clone(&task_status);
                                let task_signal_handle = task_signals.handle();

                                let task_sig_handle = task::spawn(async move {
                                    while let Some(signal) = task_signals.next().await {
                                        match signal {
                                            SIGINT => {},
                                            SIGTERM => {},
                                            SIGUSR1 => {
                                                let mut task_status_clone_guard = task_status_clone.lock().await;
                                                task_status_clone_guard.0 = false;
                                            },
                                            _ => unreachable!()
                                        }
                                    }
                                });

                                let mut buf: String = String::with_capacity(512);
                                let mut cmd_input: CmdInput;

                                loop {
                                    let mut task_status_guard = task_status.lock().await;

                                    if task_status_guard.0 == false {
                                        break;
                                    }

                                    if task_status_guard.1 == true {
                                        let mut task_address_guard = task_address.lock().await.take().unwrap();
                                        task_address_guard.read_to_string(&mut buf).await.unwrap();
                                        
                                        if buf == "DISCONNECT" {
                                            task_status_guard.1 = false;
                                            match task_address_guard.shutdown(Shutdown::Both) {
                                                Ok(()) => break,
                                                _ => panic!("Server::run : unable to gracefully disconnect client")
                                            }
                                        }
                                        else {
                                            let task_parser_guard = task_parser_ref.lock().await;
                                            cmd_input = CmdInput::new(buf.as_str());
                                            match task_parser_guard.check_cmd_input(&cmd_input) {
                                                Ok(cmd_entry) => {
                                                    match cmd_entry.get_comp() {
                                                        Some(comp) => {
                                                            let mut arg_count = 0;

                                                            buf.clear();

                                                            for token in comp.split_whitespace() {
                                                                match u8::from_str_radix(token, 10) {
                                                                    Ok(_) => {
                                                                        buf.push(' ');
                                                                        buf.push_str(token);
                                                                    },
                                                                    _ => {
                                                                        match u8::from_str_radix(token, 16) {
                                                                            Ok(_) => {
                                                                                buf.push(' ');
                                                                                buf.push_str(token);
                                                                            },
                                                                            _ => {
                                                                                if token == "x" {
                                                                                    buf.push(' ');
                                                                                    buf.push_str(cmd_input.get_arg(arg_count).to_string().as_str());
                                                                                    arg_count += 1;
                                                                                }
                                                                                else {
                                                                                    if buf.len() > 0 {
                                                                                        let mut task_uart_guard = task_uart.lock().await;

                                                                                        match task_uart_guard.write_all(buf.as_bytes()).await {
                                                                                            Ok(()) => (),
                                                                                            _ => panic!("Server::run : unable to write to uart")
                                                                                        }

                                                                                        match task_uart_guard.read_to_string(&mut buf).await {
                                                                                            Ok(_) => (),
                                                                                            _ => panic!("Server::run : unable to read from uart")
                                                                                        }

                                                                                        match task_address_guard.write_all(buf.as_bytes()).await {
                                                                                            Ok(()) => (),
                                                                                            _ => panic!("Server::run : unable to write to client")
                                                                                        }

                                                                                        buf.clear();
                                                                                        buf.push('#');
                                                                                        buf.push_str(token);
                                                                                    }
                                                                                    else {
                                                                                        buf.push('#');
                                                                                        buf.push_str(token);
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        },
                                                        None => {
                                                            buf.insert_str(0, "#");
                                                            //write to uart
                                                        }
                                                    }
                                                },
                                                Err(err) => match task_address_guard.write_all(err.as_bytes()).await {
                                                    Ok(()) => (),
                                                    _ => panic!("Server::run : unable to write to client")
                                                }
                                            }
                                        }
                                    }
                                }

                                task_signal_handle.close();
                                task_sig_handle.await;
                            },
                            _ => panic!("client task was unable to set signals")
                        }
                    }));
                }

                loop {
                    match self.socket.accept().await {
                        Ok((client, _)) => {
                            for i in 0..client_task_count {
                                let mut client_task_status = client_task_statuses[i].lock().await;
                                if client_task_status.1 == false {
                                    let mut client_task_address = client_task_addresses[i].lock().await;
                                    client_task_address.replace(client);
                                    client_task_status.1 = true;
                                    break;
                                }
                            }
                        },
                        _ => panic!("Server::run : unable to get client address")
                    }

                    if *self.run.lock().await == false {
                        for i in 0..client_task_count {
                            client_task_statuses[i].lock().await.0 = false;
                        }
                        break;
                    }
                }

                signal_handle.close();
                sig_task_handle.await;
            }
            _ => panic!("Server::run : failed to set signals")
        }
    }
}
