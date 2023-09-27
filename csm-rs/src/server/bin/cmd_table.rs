use std::collections::HashMap;

use crate::cmd_entry::CmdEntry;

pub struct CmdTable {
    num_cmds: usize,
    table: HashMap<String, CmdEntry>
}

impl CmdTable {
    pub fn new() -> CmdTable {
        CmdTable {
            num_cmds: 0,
            table: HashMap::new()
        }
    }

    pub fn get_cmd(&self, name: &str) -> Result<&CmdEntry, &str> {
        match self.table.get(name) {
            Some(cmd_entry_ref) => Ok(cmd_entry_ref),
            None => Err("CmdTable::get_cmd : command does not exist in table")
        }
    }

    pub fn add_cmd(&mut self, cmd: CmdEntry) {
        match self.table.insert(cmd.name().to_owned(), cmd) {
            None => self.num_cmds += 1,
            _ => panic!("CmdTable::add_cmd : command already exists in table"),
        }
    }
}
