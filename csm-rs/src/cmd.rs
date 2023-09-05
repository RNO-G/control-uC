pub struct Cmd {
    name: String,
    num_args: usize,
    arg_bounds: Option<Vec<(u8, u8)>>,
    comp: Option<String>
}

impl Cmd {
    pub fn new(name: &str, num_args: usize, arg_bounds: Option<Vec<(u8, u8)>>, comp: Option<&str>) -> Cmd {
        let name = name.to_owned();

        let name_chars = name.as_str().chars();

        for (i, char) in name_chars.enumerate() {
            if i == 0 && !char.is_alphabetic() {
                panic!("cmd::Cmd::new : command name must start with a letter");
            }
            else if i == name.len() - 1 && (!char.is_alphabetic() && char != '!') {
                panic!("cmd::Cmd::new : command name must end with a letter or an exclamation point");
            }
            else if i != name.len() - 1 && !char.is_alphabetic() && char != '-' {
                panic!("cmd::Cmd::new : command name must have a body composed of letters or dashes");
            }
        }

        let mut arg_bounds = arg_bounds;

        match arg_bounds {
            Some(args) => {
                if args.len() != num_args {
                    panic!("cmd::Cmd::new : command arguments list must match number of command arguments");
                }

                let arg_bounds_iter = args.iter();

                for arg_bounds_pair in arg_bounds_iter {
                    if arg_bounds_pair.0 >= arg_bounds_pair.1 {
                        panic!("cmd::Cmd::new : command argument lower bound must be less than upper bound");
                    }
                }

                arg_bounds = Some(args);
            },
            None => {
                if num_args > 0 {
                    panic!("cmd::Cmd::new : command arguments list must match number of command arguments");
                }
            }
        }

        match comp {
            Some(str) =>  {
                Cmd {
                    name,
                    num_args,
                    arg_bounds,
                    comp: Some(str.to_owned())
                }
            },
            None => {
                Cmd {
                    name,
                    num_args,
                    arg_bounds,
                    comp: None
                }
            }
        }

    }

    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn num_args(&self) -> &usize {
        &self.num_args
    }

    pub fn get_arg_bound(&self, index: usize) -> Option<&(u8, u8)> {
        match &self.arg_bounds {
            Some(args) => {
                if index > args.len() {
                    panic!("cmd::Cmd::get_arg_bound : index out of bounds");
                }
                else {
                    Some(&args[index])
                }
            },
            None => {
                if index > 0 {
                    panic!("cmd::Cmd::get_arg_bound : index out of bounds");
                }
                else {
                    None
                }
            }
        }
    }

    pub fn get_comp(&self) -> Option<&str> {
        match &self.comp {
            Some(str) => Some(str.as_str()),
            None => None
        }
    }
}
