#!/usr/bin/perl


my $collecs_file = "collec_list";
my $collecs_MPI_incl = "#include <mpi.h>\n";
my $collecs_MPI_func = "";

if (open (MYIN_FILE, "<", $collecs_file)) {
	while (my $collec = <MYIN_FILE>) {
		chomp $collec;
		
		my $out_hdr_file = "${collec}Bench.h";
		open (MYOUT_HDR_FILE, ">", $out_hdr_file);
		
		open (TMPL_INPUT, "<", "hdr.in");
		while (<TMPL_INPUT>) {
			s/#COLLEC#/$collec/gi;
			print MYOUT_HDR_FILE $_;
		}
		close (TMPL_INPUT);
		close (MYOUT_HDR_FILE);
		
		my $out_src_file = "${collec}Bench.cc";
		open (MYOUT_SRC_FILE, ">", $out_src_file);

		open (TMPL_INPUT, "<", "src.in");
		while (<TMPL_INPUT>) {
			s/#COLLEC#/$collec/gi;
			print MYOUT_SRC_FILE $_;
		}
		close (TMPL_INPUT);		
		close (MYOUT_SRC_FILE);
		
		$collecs_MPI_incl .= "#include \"${collec}Bench.h\"\n";
		
		$collecs_MPI_func .= "//======================================================================\n\n\n";
		$collecs_MPI_func .= "void CMSB::${collec}Bench::performMPICollectiveFunc () {\n\n}\n\n\n";
	}
			
	close (MYIN_FILE);
}

$collecs_MPI_func .= "//======================================================================\n\n\n";

#open (COLLECS_SRC_FILE, ">", "CollectivesMPIFuncs.cc");

#print COLLECS_SRC_FILE $collecs_MPI_incl;
#print COLLECS_SRC_FILE "\n\n\n";
#print COLLECS_SRC_FILE $collecs_MPI_func;

#close (COLLECS_SRC_FILE);
