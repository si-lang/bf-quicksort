#!/bin/perl

# usage: test_exec_plot.pl exec_file out_file num_exp p beg_n end_n test_kind

require "exp/constants.pl";

$dir_results="results";

$exec_file = $ARGV[0];
$out_file = $ARGV[1];
$num_exp = $ARGV[2];
$p = $ARGV[3];
$beg_n = $ARGV[4];
$end_n = $ARGV[5];
$p_qs = $ARGV[6];
if ($p_qs == 1){
    @algo = @algo_sort;
}

if ($ARGV[7]){
    $min_block = $ARGV[7];
    $max_block = $ARGV[8];
}
else{
    $min_block = 1000;
    $max_block = 10000;
}



#variables
my @n = ();
my $factor_n = 10;
for (my $ii=$beg_n; $ii <=$end_n; $ii=$ii*$factor_n){
    push @n, $ii;
}


my @block_size = ();
my $factor_block = 10;
for (my $ii=$min_block; $ii <$max_block; $ii=$ii*$factor_block){
    push @block_size, $ii;
    push @block_size, $ii*5;
}
push @block_size, $max_block;


foreach my $data (@data_type){
    foreach my $kind (@input_kind){
        $file_name=get_filename($out_file, $data, $kind);
        `mkdir -p $dir_results`;
        open(FILE,">$dir_results/$file_name")|| die("can't open datafile: $!");
        foreach my $nn (@n){
            foreach $exper ((1..$num_exp)){
                my $seed=time+$nn;    
                foreach my $bs (@block_size){
                    foreach my $alg (@algo){        
                        foreach my $procs ((1..$p)){
                            #print FILE "$seed $procs $nn $data $kind $alg $bs ";
                            if ($bs <= $nn){
                                @info=`$exec_file $seed $procs $nn $data $kind $alg $bs $p_qs 2>&1`;
                                if (@info){
                                    print FILE @info;
                                }
                                else{
                                    print FILE "error\n";
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

close(FILE);


#creates a name for a filename to store results
sub get_filename{
    my ($name, $data, $input_kind)=@_;
    my $str;
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $str= $name . "_" . $data . "_" . $input_kind . "-$hour-$min-$sec-$yday.out";
    return $str;
}

