pub struct CmdEntry {
    name: String,
    num_args: usize,
    arg_bounds: Option<Vec<(u8, u8)>>,
    comp: Option<String>
}

impl CmdEntry {
    pub fn new(name: &str, num_args: usize, arg_bounds: Option<Vec<(u8, u8)>>, comp: Option<&str>) -> CmdEntry {
        CmdEntry {
            name: name.to_owned(),
            num_args,
            arg_bounds,
            comp: {
                match comp {
                    Some(str_ref) => Some(str_ref.to_owned()),
                    None => None
                }
            }
        }
    }

    pub fn name(&self) -> &str {
        self.name.as_str()
    }

    pub fn num_args(&self) -> &usize {
        &self.num_args
    }

    pub fn get_arg_bound(&self, index: usize) -> Result<&(u8, u8), &str> {
        match &self.arg_bounds {
            Some(args) => {
                if index > args.len() {
                    Err("CmdEntry::get_arg_bound : index out of bounds")
                }
                else {
                    Ok(&args[index])
                }
            },
            None => {
                Err("CmdEntry::get_arg_bound : command has no arguments")
            }
        }
    }

    pub fn get_comp(&self) -> Option<&str> {
        match &self.comp {
            Some(str) => Some(str),
            None => None
        }
    }
}
