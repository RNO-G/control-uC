use std::{
    str::{
        SplitWhitespace,
        Lines
    },
    fs::{
        read_to_string,
        DirEntry
    },
    io::Error
};

use crate::cmd_entry::CmdEntry;

pub fn is_valid_name(name: &str) -> Result<&str, &str> {
    let name_iter = name.chars();
    
    for (i, char) in name_iter.enumerate() {
        if i == 0 && !char.is_alphabetic() {
            return Err("cmd_common::is_valid_name : command name must begin with alphabetic character")
        }
        else if i == name.len() - 1 && (!char.is_alphabetic() && char != '!') {
            return Err("cmd_common::is_valid_name : command name must end with alphabetic character or exclamation mark")
        }
        else if i != name.len() - 1 && !char.is_alphabetic() && char != '-' {
            return Err("cmd_common::is_valid_name : command name must be composed of alphabetic characters or dashes")
        }
    }

    Ok(name)
}

pub fn is_valid_args(mut args_str: SplitWhitespace) -> Result<(usize, Option<Vec<u8>>), &str> {
    let mut arg = args_str.next();
    let mut is_hex: bool;
    let mut num_args = 0;
    let mut args: Vec<u8> = Vec::new();

    loop {
        match arg {
            Some(arg_chars) => {
                is_hex = false;

                for char in arg_chars.chars() {
                    if !char.is_ascii_digit() && !char.is_ascii_hexdigit() {
                        return Err("cmd_common::is_valid_arg : command args must be valid decimal or hex digits")
                    }

                    if !char.is_ascii_digit() && char.is_ascii_hexdigit() {
                        is_hex = true;
                    }
                }

                if is_hex {
                    match u8::from_str_radix(arg_chars, 16) {
                        Ok(arg_val) => {
                            args.push(arg_val);
                        },
                        _ => return Err("cmd_common::is_valid_arg : invalid argument")
                    }
                }
                else {
                    match u8::from_str_radix(arg_chars, 10) {
                        Ok(arg_val) => {
                            args.push(arg_val);
                        },
                        _ => return Err("cmd_common::is_valid_arg : invalid argument")
                    }
                }

                num_args += 1;
            },
            None => break
        }

        arg = args_str.next();
    }

    if num_args > 0 {
        Ok((num_args, Some(args)))
    }
    else {
        Ok((num_args, None))
    }
}

pub fn is_valid_arg_count(num_args: &str) -> Result<usize, &str> {
    match usize::from_str_radix(num_args, 10) {
        Ok(arg_count) => Ok(arg_count),
        _ => Err("cmd_common::is_valid_arg_count : argument count must be a decimal value")
    }
}

pub fn is_valid_arg_bounds(lines: Lines) -> Result<Vec<(u8, u8)>, &str> {
    let mut arg_bounds_split: SplitWhitespace;
    let mut arg_bound: Option<&str>;
    let mut is_hex: bool;
    let mut arg_index: u8;
    let mut arg_bounds_index = 0;
    let mut arg_bounds: Vec<(u8, u8)> = Vec::new();

    for line in lines {
        arg_bounds_split = line.split_whitespace();

        if arg_bounds_split.count() != 2 {
            return Err("cmd_common::is_valid_arg_bounds : there must be two bounds for an argument")
        }

        arg_bounds_split = line.split_whitespace();
        arg_bound = arg_bounds_split.next();
        arg_index = 0;

        loop {
            match arg_bound {
                Some(arg_chars) => {
                    is_hex = false;

                    for char in arg_chars.chars() {
                        if !char.is_ascii_digit() && !char.is_ascii_hexdigit() {
                            return Err("cmd_common::is_valid_arg_bounds : arg bounds must be valid decimal or hex digits")
                        }

                        if !char.is_ascii_digit() && char.is_ascii_hexdigit() {
                            is_hex = true;
                        }
                    }

                    if is_hex {
                        match u8::from_str_radix(arg_chars, 16) {
                            Ok(arg_val) => {
                                if arg_index == 0 {
                                    arg_bounds.push((arg_val, 0));
                                    arg_index += 1;
                                }
                                else {
                                    arg_bounds[arg_bounds_index].1 = arg_val;
                                    arg_bounds_index += 1;
                                }
                            },
                            _ => return Err("cmd_common::is_valid_arg_arg_bounds : invalid argument")
                        }
                    }
                    else {
                        match u8::from_str_radix(arg_chars, 10) {
                            Ok(arg_val) => {
                                if arg_index == 0 {
                                    arg_bounds.push((arg_val, 0));
                                    arg_index += 1;
                                }
                                else {
                                    arg_bounds[arg_bounds_index].1 = arg_val;
                                    arg_bounds_index += 1;
                                }
                            },
                            _ => return Err("cmd_common::is_valid_arg_bounds : invalid argument")
                        }
                    }
                },
                None => break
            }

            arg_bound = arg_bounds_split.next();
        }
    }

    Ok(arg_bounds)
}

pub fn read_base_file(file: Result<DirEntry, Error>) -> Result<CmdEntry, &'static str> {
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
                            Ok(CmdEntry::new(name, num_args, None, None))
                        }
                        else {
                            Ok(CmdEntry::new(name, num_args, Some(is_valid_arg_bounds(lines).unwrap()), None))
                        }
                    },
                    _ => Err("cmd_parser::read_custom_file : command file must have at least three lines")
                }
            },
            _ => Err("cmd_parser::read_base_file : custom command file must have at least three lines")
        }
    }
    else {
        Err("cmd_parser::read_base_file : command files must end in .cmd extension")
    }
}

