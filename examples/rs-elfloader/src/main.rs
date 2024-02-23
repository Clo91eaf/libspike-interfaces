mod difftest;

use clap::Parser;
use difftest::Difftest;
use tracing::{info, trace, Level};
use tracing_subscriber::{EnvFilter, FmtSubscriber};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
	#[arg(short, long)]
	elf_file: String,

	#[arg(short, long)]
	fst_file: String,

	#[arg(short, long)]
	config: String,
}

fn main() -> anyhow::Result<()> {
	let global_logger = FmtSubscriber::builder()
		.with_env_filter(EnvFilter::from_default_env())
		.with_max_level(Level::TRACE)
		.without_time()
		.with_target(false)
		.compact()
		.finish();
	tracing::subscriber::set_global_default(global_logger)
		.expect("internal error: fail to setup log subscriber");

	let args = Args::parse();

	let mut diff = Difftest::new(1usize << 32, &args.elf_file, &args.fst_file);

	loop {
		diff.execute().unwrap();
	}

	Ok(())
}
