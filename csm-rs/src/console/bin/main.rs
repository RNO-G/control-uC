mod console;

use std::env::args;

use crate::console::Console;

#[async_std::main]
async fn main() {
    let args: Vec<String> = args().collect();

    if args.len() != 2 {
        panic!("console : invalid arguments")
    }

    match Console::new(&args[1]).await {
        Ok(mut console) => console.run().await,
        Err(err) => panic!("{}", err)
    }
}