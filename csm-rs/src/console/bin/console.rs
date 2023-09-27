use async_std::io::ReadExt;
use async_std::io::WriteExt;
use async_std::io::stdin;
use async_std::net::TcpStream;
use async_std::net::ToSocketAddrs;
use async_std::net::Shutdown;
use async_std::stream::StreamExt;
use async_std::sync::Arc;
use async_std::sync::Mutex;
use signal_hook::consts::signal::{SIGINT, SIGTERM};
use signal_hook_async_std::Signals;

pub struct Console {
    socket: TcpStream,
    run: Arc<Mutex<bool>>,
    buf: String
}

impl Console {
    pub async fn new<'a>(socket_path: impl ToSocketAddrs) -> Result<Console, &'a str>{
        match TcpStream::connect(socket_path).await {
            Ok(socket) => Ok(Console {
                socket,
                run: Arc::new(Mutex::new(true)),
                buf: String::with_capacity(512)
            }),
            _ => Err("Console::new : unable to connect to server")
        }
    }

    pub async fn send_cmd(&mut self) -> Result<(), &str> {
        match self.socket.write_all(self.buf.as_bytes()).await {
            Ok(_) => match self.socket.read_to_string(&mut self.buf).await {
                Ok(_) => Ok(()),
                _ => Err("DID NOT RECIEVE ACKNOWLEDGEMENT FROM SERVER")
            },
            _ => Err("UNABLE TO SEND COMMAND")
        }
    }

    pub async fn run(&mut self) {
        match Signals::new(&[SIGINT, SIGTERM]) {
            Ok(mut signals) => {
                let run = Arc::clone(&self.run);
                let signal_handle = signals.handle();

                let task_handle = async_std::task::spawn(async move {
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

                let stdin = stdin();

                loop {
                    print!("rno-g console > ");
                    match stdin.read_line(&mut self.buf).await {
                        Ok(n) => {
                            if n > 512 || n == 0 {
                                println!("rno-g console > INVALID INPUT");
                            }
                            else {
                                match self.send_cmd().await {
                                    Ok(_) => println!("rno-g-console > {}", self.buf),
                                    Err(err) => println!("rno-g-console > {}", err)
                                }
                            }
                        },
                        _ => println!("rno-g-console > UNABLE TO RECIEVE DATA FROM SERVER")
                    }

                    if !*self.run.lock().await {
                        break;
                    }
                }

                signal_handle.close();
                task_handle.await;

                match self.socket.shutdown(Shutdown::Both) {
                    Ok(_) => (),
                    _ => panic!("Console::run : failed to gracefully close connection to server")
                }
            },
            _ => panic!("Console::run : failed to set signals")
        }
    }
}
