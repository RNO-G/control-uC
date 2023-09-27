use std::{
    fs::{
        read_to_string,
        DirEntry
    },
    io::Error,
    path::PathBuf
};

use crate::cmd_common:: {
    is_valid_name,
    is_valid_arg_bounds,
    is_valid_arg_count,
    read_base_file
};
use crate::cmd_entry::CmdEntry;
use crate::cmd_input::CmdInput;
use crate::cmd_table::CmdTable;

pub struct CmdParser {
    base_cmd_dir: PathBuf,
    num_base_cmds: usize,
    base_cmd_table: CmdTable,
    custom_cmd_dirs: Option<Vec<PathBuf>>,
    num_custom_cmds: Option<Vec<usize>>,
    custom_cmd_tables: Option<Vec<CmdTable>>
}

impl CmdParser {
    pub fn new(base_cmd_dir_str: &str) -> Result<CmdParser, &str> {
        let base_cmd_dir = PathBuf::from(base_cmd_dir_str);
        let mut num_base_cmds = 0;
        let mut base_cmd_table = CmdTable::new();

        if base_cmd_dir.is_symlink() {
            return Err("CmdParser::new : Base Directory Path cannot be a symlink")
        }

        if !base_cmd_dir.is_dir() {
            return Err("CmdParser::new : Base Directory Path must be a valid path")
        }

        let files = base_cmd_dir.read_dir().expect("CmdParser::add_custom::cmd : directory path is invalid");

        for file in files {
            base_cmd_table.add_cmd(read_base_file(file).unwrap());
            num_base_cmds += 1;
        }

        Ok(CmdParser {
            base_cmd_dir,
            num_base_cmds,
            base_cmd_table,
            custom_cmd_dirs: None,
            num_custom_cmds: None,
            custom_cmd_tables: None
        })
    }

    fn is_valid_comp<'a>(&'a self, num_args: &usize, line: Option<&'a str>) -> Result<&str, &str> {
        let line = line.expect("CmdParser::is_valid_comp : command file must have at least three lines");
        let mut tokens = line.split_whitespace();
        let num_tokens = tokens.count();

        if num_tokens == 0 {
            return Err("CmdParser::is_valid_comp : command composition must not be empty")
        }

        tokens = line.split_whitespace();
        let mut args_found = 0;
        let mut token: &str;
        let mut cmd: &CmdEntry;
        let mut cmd_arg_count: &usize;
        let mut cmd_arg_bounds: &(u8, u8);
        let mut cur_token = 0;
        let mut is_hex: bool;
        let mut arg_val: u8;

        loop {
            token = tokens.next().expect("CmdParser::is_valid_comp : command name must not be empty");
            cmd = self.base_cmd_table.get_cmd(is_valid_name(token).unwrap()).unwrap();
            cmd_arg_count = cmd.num_args();

            if *cmd_arg_count > 0 {
                cur_token += 1;

                if cur_token == num_tokens {
                    return Ok(line)
                }
            }
            else {
                for i in 0..*cmd_arg_count {
                    token = tokens.next().expect("CmdParser::is_valid_cmp : command arg bound must not be empty");

                    if token == "x" {
                        args_found += 1;

                        if args_found > *num_args {
                            return Err("CmdParser::is_valid_comp : number of variable args found exceeds number given")
                        }

                        cur_token += 1;

                        if cur_token == num_tokens {
                            if i < *cmd_arg_count - 1 {
                                return Err("CmdParser::is_valid_comp : command was not provided with enough arguments")
                            }
                            else {
                                return Ok(line)
                            }
                        }
                    }
                    else {
                        is_hex = false;

                        for char in token.chars() {
                            if !char.is_ascii_digit() && !char.is_ascii_hexdigit() {
                                return Err("CmdParser::is_valid_comp : command arg must be valid decimal or hex digits")
                            }

                            if !char.is_ascii_digit() && char.is_ascii_hexdigit() {
                                is_hex = true;
                            }
                        }

                        if is_hex {
                            arg_val = u8::from_str_radix(token, 16).expect("CmdParser::is_valid_comp : invalid argument");
                        }
                        else {
                            arg_val = u8::from_str_radix(token, 10).expect("CmdParser::is_valid_comp : invalid argument");
                        }

                        cmd_arg_bounds = cmd.get_arg_bound(i).unwrap();

                        if arg_val < cmd_arg_bounds.0 || arg_val > cmd_arg_bounds.1 {
                            return Err("CmdParser::is_valid_comp : composition argument is not within command argument bounds")
                        }
                        else {
                            cur_token += 1;

                            if cur_token == num_tokens {
                                if i < *cmd_arg_count - 1 {
                                    return Err("CmdParser::is_valid_comp : command was not provided with enough arguments")
                                }
                                else {
                                    return Ok(line)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fn read_custom_file(&self, file: Result<DirEntry, Error>) -> Result<CmdEntry, &str> {
        let file = file.expect("CmdParser::read_custom_file : IO error while reading directory");
        let file_path = file.path();
        let file_name = file.file_name().into_string().expect("CmdParser::read_custom_file : file name must only have unicode characters");

        if file_name.len() > 5 && file_name.split_at(file_name.len() - 4).1 == ".cmd" {
            let file_contents = read_to_string(file_path).expect("CmdParser::read_custom_file : IO error while reading file");
            
            if file_contents.lines().count() < 2 {
                return Err("cmd_parser::read_file : file needs at least command name and arg count")
            }

            let mut lines = file_contents.lines();

            match lines.next() {
                Some(name_str) => {
                    let name = is_valid_name(name_str).unwrap();

                    match lines.next() {
                        Some(num_args_str) => {
                            let num_args = is_valid_arg_count(num_args_str).unwrap();

                            if num_args == 0 {
                                let comp = self.is_valid_comp(&num_args, lines.next()).unwrap();

                                Ok(CmdEntry::new(name, num_args, None, Some(comp)))
                            }
                            else {
                                let comp = self.is_valid_comp(&num_args, lines.nth_back(0)).unwrap();
                                let arg_bounds = is_valid_arg_bounds(lines).unwrap();

                                Ok(CmdEntry::new(name, num_args, Some(arg_bounds), Some(comp)))
                            }
                        },
                        _ => Err("CmdParser::read_custom_file : command file must have at least three lines")
                    }
                },
                _ => Err("CmdParser::read_custom_file : custom command file must have at least three lines")
            }
        }
        else {
            Err("CmdParser::read_custom_file : command files must end in .cmd extension")
        }
    }

    pub fn add_custom_cmd(&mut self, custom_cmd_dir: &str) {
        let custom_cmd_dir = PathBuf::from(custom_cmd_dir);
        let mut num_custom_cmds = 0;
        let mut custom_cmd_table = CmdTable::new();

        if custom_cmd_dir.is_symlink() {
            panic!("CmdParser::add_custom_cmd : directory path cannot be a symlink");
        }

        if !custom_cmd_dir.is_dir() {
            panic!("CmdParser::add_custom_cmd : directory path must be a valid path");
        }

        let files = custom_cmd_dir.read_dir().expect("CmdParser::add_custom_cmd : directory path is invalid");

        for file in files {
            custom_cmd_table.add_cmd(self.read_custom_file(file).unwrap());
            num_custom_cmds += 1;
        }

        if self.custom_cmd_dirs.is_none() {
            self.custom_cmd_dirs = Some(vec![custom_cmd_dir]);
            self.num_custom_cmds = Some(vec![num_custom_cmds]);
            self.custom_cmd_tables = Some(vec![custom_cmd_table]);
        }
        else {
            let mut existing_custom_cmd_dirs = self.custom_cmd_dirs.take().unwrap();
            let mut existing_num_custom_cmds = self.num_custom_cmds.take().unwrap();
            let mut existing_custom_cmd_tables = self.custom_cmd_tables.take().unwrap();

            existing_custom_cmd_dirs.push(custom_cmd_dir);
            existing_num_custom_cmds.push(num_custom_cmds);
            existing_custom_cmd_tables.push(custom_cmd_table);

            self.custom_cmd_dirs = Some(existing_custom_cmd_dirs);
            self.num_custom_cmds = Some(existing_num_custom_cmds);
            self.custom_cmd_tables = Some(existing_custom_cmd_tables);
        }
    }

    pub fn check_cmd_input(&self, cmd_input: &CmdInput) -> Result<&CmdEntry, &str> {
        match self.base_cmd_table.get_cmd(cmd_input.name()) {
            Ok(cmd_entry) => {
                if cmd_entry.num_args() == cmd_input.num_args() {
                    for i in 0..*cmd_entry.num_args() {
                        if *cmd_input.get_arg(i) < cmd_entry.get_arg_bound(i).unwrap().0 || *cmd_input.get_arg(i) > cmd_entry.get_arg_bound(i).unwrap().1 {
                            return Err("CmdParser::check_cmd_input : args must be within bounds")
                        }
                        else {
                            continue;
                        }
                    }

                    Ok(cmd_entry)
                }
                else {
                    return Err("CmdParser::check_cmd_input : number of args must match")
                }
            },
            _ => {
                if self.custom_cmd_tables.is_some() {
                    for (i, custom_cmd_table) in self.custom_cmd_tables.as_ref().unwrap().into_iter().enumerate() {
                        match custom_cmd_table.get_cmd(cmd_input.name()) {
                            Ok(cmd_entry) => {
                                if cmd_entry.num_args() == cmd_input.num_args() {
                                    for i in 0..*cmd_entry.num_args() {
                                        if *cmd_input.get_arg(i) < cmd_entry.get_arg_bound(i).unwrap().0 || *cmd_input.get_arg(i) > cmd_entry.get_arg_bound(i).unwrap().1 {
                                            return Err("CmdParser::check_cmd_input : args must be within bounds")
                                        }
                                        else {
                                            continue;
                                        }
                                    }

                                    return Ok(cmd_entry)
                                }
                                else {
                                    return Err("CmdParser::check_cmd_input : number of args must match")
                                }
                            },
                            _ => {
                                if i == self.custom_cmd_tables.as_ref().unwrap().len() - 1 {
                                    return Err("CmdParser::check_cmd_input : command not found")
                                }
                                else {
                                    continue;
                                }
                            }
                        }
                    }

                    return Err("CmdParser::check_cmd_input : command not found")
                }
                else {
                    return Err("CmdParser::check_cmd_input : command not found")
                }
            }
        }
    }
}
