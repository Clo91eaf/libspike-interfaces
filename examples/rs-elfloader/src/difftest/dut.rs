use crate::{info, trace};
use fst_native::*;
use serde::Deserialize;
use std::collections::HashSet;

type MyFstReader = FstReader<std::io::BufReader<std::fs::File>>;

pub struct Dut {
	file: String,
	filter: FstFilter,
}

#[derive(Default, Debug)]
struct SignalMetadata {
	module_paths: Vec<Vec<String>>,
	names: Vec<String>,
	handle: Vec<FstSignalHandle>,
}

impl SignalMetadata {
	fn push(&mut self, module_path: Vec<String>, name: String, handle_id: FstSignalHandle) {
		self.module_paths.push(module_path);
		self.names.push(name);
		self.handle.push(handle_id);
	}
}

#[derive(Debug, Deserialize)]
struct Config {
	signals: Vec<String>,
}

impl Dut {
	pub fn new(file: String) -> Self {
		Self {
			file,
			filter: FstFilter::all(),
		}
	}

	pub fn execute(&mut self) -> Result<(), Box<dyn std::error::Error>> {
		let f = std::fs::File::open(self.file.clone())
			.unwrap_or_else(|_| panic!("Failed to open {}", self.file));
		let mut reader = FstReader::open(std::io::BufReader::new(f)).unwrap();

		Ok(())
	}

	fn collect_signals(
		reader: &mut MyFstReader,
		expected: &[String],
	) -> anyhow::Result<SignalMetadata> {
		let mut metadata = SignalMetadata::default();
		let mut module_path: Vec<String> = Vec::new();
		let mut dedup_pool = HashSet::new();
		reader.read_hierarchy(|hier| match hier {
			FstHierarchyEntry::Var { name, handle, .. } => {
				if expected.contains(&name) && !dedup_pool.contains(&handle.get_index()) {
					let id = handle.get_index();
					metadata.push(module_path.clone(), name, handle);
					dedup_pool.insert(id);
				}
			}
			FstHierarchyEntry::Scope { name, .. } => module_path.push(name.to_string()),
			FstHierarchyEntry::UpScope => {
				module_path.pop();
			}
			_ => (),
		})?;

		Ok(metadata)
	}

	pub fn test(&mut self, config: String) -> Result<(), Box<dyn std::error::Error>> {
		let f = std::fs::File::open(self.file.clone())
			.unwrap_or_else(|_| panic!("Failed to open {}", self.file));
		let mut reader = FstReader::open(std::io::BufReader::new(f)).unwrap();

		info!("FST file: {}", self.file);

		let header = reader.get_header();
		trace!(
			version = header.version,
			date = header.date,
			start_time = header.start_time,
			end_time = header.end_time,
			"Header info"
		);

		info!("Reading config from file {}", config);
		let config = std::fs::read(config)?;
		let config: Config = serde_json::from_slice(&config)?;

		info!("Iterating hierachy to get signal information");
		let metadata = Self::collect_signals(&mut reader, &config.signals)?;


		Ok(())
	}
}
