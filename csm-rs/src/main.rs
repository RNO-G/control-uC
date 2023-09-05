mod cmd;

use cmd::Cmd;

fn main() {
    let my_cmd = Cmd::new("AMPS-SET!", 2, Some(vec![(0x0, 0x3f), (0x0, 0x7)]), None);
    let custom_cmd = Cmd::new("LEFT-AMP", 1, Some(vec![(0x0, 0x3f)]), Some("AMPS-SET x 0x0"));

    println!("{}", my_cmd.name());
    
    for i in 0..*my_cmd.num_args() {
        println!("({}, {})", my_cmd.get_arg_bound(i).unwrap().0, my_cmd.get_arg_bound(i).unwrap().1);
    }

    for i in 0..*my_cmd.num_args() {
        println!("({}, {})", my_cmd.get_arg_bound(i).unwrap().0, my_cmd.get_arg_bound(i).unwrap().1);
    }

    match my_cmd.get_comp() {
        Some(str) => println!("comp: {}", str),
        None => println!("this is a base command")
    }

    match custom_cmd.get_comp() {
        Some(str) => println!("comp: {}", str),
        None => println!("this is a base command")
    }
}
