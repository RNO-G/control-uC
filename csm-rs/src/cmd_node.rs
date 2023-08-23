
pub struct CmdNode {
    name: String,
    num_args: usize,
    arg_bounds: Vec<(u8, u8)>,
    left: Option<Box<CmdNode>>,
    right: Option<Box<CmdNode>>
}

fn check_name(name: &str) -> bool {
    let mut name_iter = name.chars();

    if name_iter.all(char::is_alphabetic) {
        true
    }
    else {
        name_iter = name.chars();
        
        if name_iter.next().unwrap().is_alphabetic() {
            for (i, letter) in name_iter.enumerate() {

                if !letter.is_alphabetic() && letter != '-' {
                    return false
                }
                else if i == (name.len() - 1) && !letter.is_alphabetic() {
                    return false
                }
            }

            true
        }
        else {
            false
        }
    }
}

impl CmdNode {

    pub fn new(
        name: impl Into<String>, 
        num_args: usize, 
        arg_bounds: impl Into<Vec<(u8, u8)>>, 
        left: Option<Box<CmdNode>>, 
        right: Option<Box<CmdNode>>
    ) -> CmdNode {
        let name = name.into();

        if !check_name(name.as_str()){
            panic!("cmd_node::CmdNode::new : invalid command name");
        }
        else {
            let arg_bounds = arg_bounds.into();

            if arg_bounds.len() != num_args {
                panic!("cmd_node::CmdNode::new : number of argument bounds should match num_args");
            }
            else {
                let arg_bounds_iter = arg_bounds.iter();

                for arg_bound in arg_bounds_iter {
                    if arg_bound.0 >= arg_bound.1 {
                        panic!("cmd_node::CmdNode::new : minimum bound must be strictly less than maximum bound");
                    }
                }

                CmdNode {
                    name,
                    num_args,
                    arg_bounds,
                    left,
                    right
                }
            }
        }       
    }

    pub fn name(&self) -> &str {
        &self.name.as_str()
    }

    pub fn num_args(&self) -> &usize {
        &self.num_args
    }

    pub fn arg_bound(&self, index: usize) -> &(u8, u8) {
        if index > self.arg_bounds.len() {
            panic!("cmd_node::CmdNode::arg_bound : index out of bounds");
        }
        else {
            &self.arg_bounds[index]
        }
    }

    pub fn left(&self) -> Option<&Box<CmdNode>> {
        match &self.left {
            Some(node) => Some(&node),
            None => None
        }
    }

    pub fn set_left(&mut self, left: Option<Box<CmdNode>>) {
        self.left = left;
    }

    pub fn right(&self) -> Option<&Box<CmdNode>> {
        match &self.right {
            Some(node) => Some(&node),
            None => None
        }
    }
}

