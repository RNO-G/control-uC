use crate::cmd_common::{
    is_valid_name,
    is_valid_args
};

pub struct CmdInput {
    name: String,
    num_args: usize,
    args: Option<Vec<u8>>
}

impl CmdInput {
    pub fn new(cmd_str: &str) -> CmdInput {
        let cmd_str = cmd_str.to_owned();
        let mut cmd_str_iter = cmd_str.split_whitespace();

        match cmd_str_iter.next() {
            Some(name_str) => {
                let name = is_valid_name(name_str).unwrap().to_owned();
                let (num_args, args) = is_valid_args(cmd_str_iter).unwrap();

                CmdInput {
                    name,
                    num_args,
                    args
                }
            },
            _ => panic!("CmdInput::new : empty string")
        }
    }

    pub fn name(&self) -> &str {
        self.name.as_str()
    }

    pub fn num_args(&self) -> &usize {
        &self.num_args
    }

    pub fn get_arg(&self, index: usize) -> &u8 {
        match &self.args {
            Some(args) => {
                if index >= args.len() {
                    panic!("user_cmd::UserCmd::get_arg : index out of bounds");
                }
                else {
                    &args[index]
                }
            },
            None => panic!("user_cmd::UserCmd::get_arg : command has no arguments")
        }
    }
}