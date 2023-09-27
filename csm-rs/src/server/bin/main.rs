mod cmd_common;
mod cmd_entry;
mod cmd_input;
mod cmd_parser;
mod cmd_table;

mod server;

use std::env::args;

use crate::server::Server;

#[async_std::main]
async fn main() {
    let args: Vec<String> = args().collect();

    if args.len() < 4 {
        panic!("server : at least three paths must be given")
    }

    let custom_dirs: Option<Vec<&str>>;

    if args.len() > 4 {
        let mut custom_dirs_vec = Vec::new();
        for arg in &args[4..args.len()] {
            custom_dirs_vec.push(arg.as_str());
        }

        custom_dirs = Some(custom_dirs_vec);
    }
    else {
        custom_dirs = None;
    }

    match Server::new(&args[1], &args[2], &args[3], custom_dirs).await {
        Ok(mut server) => {
            server.run().await;
        },
        Err(err) => panic!("{}", err)
    }
}