#!/bin/perl

# usage: test_plot_eng.pl $filename $plot_type $plot_format 

require "exp/constants.pl";

#progam parametres
$filename = $ARGV[0];
if ($ARGV[1] ne ""){
    $speed_abs_cnt = $ARGV[1];
    if ($ARGV[2]){
        $png_ps = $ARGV[2];
    }
    else{
        $png_ps = "png";
    }
}
else{
    $speed_abs_cnt = 0;
    $png_ps = "png";
}

$dir_plot="plots_" . $png_ps;

#data manipulation to prepare a multiarray for averages
#arrays of sizes
my @n = ();
my @block_size = ();
my @num_procs = ();

$exp_data="";
$exp_kind="";

#preprocess the file
open (DATA_FILE,$filename);
while($line=<DATA_FILE>){
    chomp($line);   
    if ($line ne "error"){ 
        if ($speed_abs_cnt<=1){
            ($seed, $procs, $nn, $data, $kind, $alg, $bs, $part_sort, $t) = split(/ /,$line);
        }
        else{
            ($seed, $procs, $nn, $data, $kind, $alg, $bs, $part_sort, $t, $comp, $s2, $c2) = split(/ /,$line);
        }
        if ($exp_data eq ""){
            $exp_data = $data;
            $exp_kind = $kind;
        }
        if ($nn ne ""){
            @n = &find_and_insert($nn, @n);
            @num_procs = &find_and_insert($procs, @num_procs);
            @block_size = &find_and_insert($bs, @block_size);
        }
    }
}
close(DATA_FILE);


#initialization of averages (multiarray)
foreach $i_nn (0..$#n){
    foreach $i_procs (0..$#num_procs){
        foreach $i_alg (0..$#algo){
            foreach $i_bs (0..$#block_size){
                $avg1->[$i_nn][$i_procs][$i_alg][$i_bs] = 0;
                $avg2->[$i_nn][$i_procs][$i_alg][$i_bs] = 0;
                $count->[$i_nn][$i_procs][$i_alg][$i_bs] = 0;
            }
        }
    }
}


#read the file again processing times
$part_sort = 0;
open (DATA_FILE,$filename);

while($line=<DATA_FILE>){
    chomp($line);
    if ($line ne "error"){
        if ($speed_abs_cnt<=1){
            ($seed, $procs, $nn, $data, $kind, $alg, $bs, $part_sort, $t) = split(/ /,$line);
        }
        else{
            ($seed, $procs, $nn, $data, $kind, $alg, $bs, $part_sort, $t, $comp, $s2, $c2) = split(/ /,$line);
        }
        my $i_nn = &find($nn, @n);
        my $i_bs = &find($bs, @block_size);
        my $i_procs = &find($procs, @num_procs);
        my $i_alg = 0;
        if ($part_sort != 1){
            $i_alg = &find($alg, @algo);
        }
        elsif ($part_sort == 1){
            $i_alg = &find($alg, @algo_sort);
        }
    
        if ($t ne "error" && $seed ne ""){
            $avg1->[$i_nn][$i_procs][$i_alg][$i_bs] = $avg1->[$i_nn][$i_procs][$i_alg][$i_bs] + $t;
            $avg2->[$i_nn][$i_procs][$i_alg][$i_bs] = $avg2->[$i_nn][$i_procs][$i_alg][$i_bs] + $comp;
            $count->[$i_nn][$i_procs][$i_alg][$i_bs]++;
            #print "$count->[$i_nn][$i_procs][$alg][$i_bs]\n";
        }
    }
}
close(DATA_FILE);

# In a file, part_sort is the same for every execution
if ($part_sort == 1){
    @algo = @algo_sort;
    @algo_names = @algo_names_sort;
}

#calculate averages
foreach $i_nn (0..$#n){
    foreach $i_procs (0..$#num_procs){
        foreach $i_alg (0..$#algo){
            foreach $i_bs (0..$#block_size){
                if ($count->[$i_nn][$i_procs][$i_alg][$i_bs] != 0){
                    $avg1->[$i_nn][$i_procs][$i_alg][$i_bs] = $avg1->[$i_nn][$i_procs][$i_alg][$i_bs]/$count->[$i_nn][$i_procs][$i_alg][$i_bs];                   $avg2->[$i_nn][$i_procs][$i_alg][$i_bs] = $avg2->[$i_nn][$i_procs][$i_alg][$i_bs]/$count->[$i_nn][$i_procs][$i_alg][$i_bs];     
                }
            }
        }
    }
}

my $file_p=&get_filename($filename);
`mkdir -p $dir_plot/$file_p`;

#only plots for fixed initial list size

my $cs = 0;
if ($speed_abs_cnt == 2){
    $cs = 1;
}

foreach my $i(0..$cs){
    &make_plots_type($i,$file_p, $exp_data, $exp_kind, $ALG_i, $PROCS_i, $N_i, $BS_i); # lines = $alg, xaxis = $procs   
    #&make_plots_type($i,$file_p, $exp_data, $exp_kind, $ALG_i, $N_i, $BS_i, $PROCS_i);
    
    #&make_plots_type($i,$file_p, $exp_data, $exp_kind, $N_i, $BS_i, $ALG_i, $PROCS_i);
    #&make_plots_type($i,$file_p, $exp_data, $exp_kind, $BS_i, $N_i, $ALG_i, $PROCS_i);
    #&make_plots_type($i,$file_p, $exp_data, $exp_kind, $PROCS_i, $N_i, $ALG_i, $BS_i);

        #&make_plots_type($i,$file_p, $exp_data, $exp_kind, $PROCS_i, $BS_i, $ALG_i, $N_i);
        &make_plots_type($i,$file_p, $exp_data, $exp_kind, $ALG_i, $BS_i, $N_i, $PROCS_i);

}


if ($png_ps eq "png" || $png_ps eq "eps"){
    &make_html($data, $kind, $file_p);
}
else{
    &make_latex($data, $kind, $file_p);
}


################
# Subroutines #
###############

sub get_array{
    my ($x) = @_;    
    if ($x == $N_i){
        @v= @n;
    }    
    if ($x == $PROCS_i){
        @v = @num_procs;
    }
    elsif ($x == $ALG_i){
        @v = @algo;
    }
    elsif ($x == $BS_i){
        @v = @block_size;
    }
    return @v;    
}

sub make_plots_type{
    my ($swap_comp, $file_p, $data, $kind, $lines, $xaxis, $f0, $f1) = @_;
    @vlines= &get_array($lines);
    @vxaxis = &get_array($xaxis);
    @vf0 = &get_array($f0);  
    @vf1 = &get_array($f1);

    foreach my $i_f0 (0..$#vf0){
        foreach my $i_f1 (0..$#vf1){
            foreach my $i_lines (0..$#vlines){
                $file_tmp = &get_tmp_filename($i_lines);
                open(FILE_TMP,">$dir_plot/$file_p/$file_tmp")|| die("can't open datafile: $!");
                foreach my $i_xaxis (0..$#vxaxis){
                    $index[$lines] = $i_lines;
                    $index[$xaxis] = $i_xaxis;
                    $index[$f0] = $i_f0;
                    $index[$f1] = $i_f1;
                    $time = $avg1->[$index[0]][$index[1]][$index[2]][$index[3]];
                    if ($speed_abs_cnt <=1){
                        if ($speed_abs_cnt == 0){
                            $time = ($avg1->[$index[0]][$index[1]][0][$index[3]]+0.000000001)/($time+0.000000001);                                           
                        }
                        else{
                            $time = $time/$n[$index[$N_i]]; #scaled
                        }                     
                    }
                    else{
                        if ($swap_comp == 1){
                            $time = $avg2->[$index[0]][$index[1]][$index[2]][$index[3]];
                        }
                        $time = $time / $block_size[$index[3]];
                        if ($index[2] == 0){
                            $time = 0;
                        }  
                    }
                    if ($index[2] != 0){
                        if($index[1] == 0){
                            print FILE_TMP "$vxaxis[$i_xaxis] 1\n";
                        }
                        elsif ($xaxis!=$BS_i || $index[2] > 2){
                            print FILE_TMP "$vxaxis[$i_xaxis] $time\n";
                        }
                    }
                }
                close(FILE_TMP);
            }
            &make_plot($swap_comp, $file_p, $data, $kind, $lines, $xaxis, $f0, $f1, $i_f0, $i_f1);
            #remove temporary files 
            foreach my $i_lines(0..$#vlines){
                $file_tmp = &get_tmp_filename($i_lines);
                `rm $dir_plot/$file_p/$file_tmp`;
            }
        }
    }
}


sub get_filename{
    my $original_file=shift;
    my ($file_dp,$oth)=split(/\./,$original_file);
    my ($dir,$file_p)=split(/\//,$file_dp);
    if ($file_p eq ""){
        $file_p = $dir;
    }
    return $file_p;
}

sub get_tmp_filename{
    my $val = shift;
    return $file_tmp = "tmp_" . $val . ".data";
}

sub find{
    my ($i, @array) = @_;
    my $found = @array;
    my $pos = 0;
    foreach $ii (@array){
        if ($ii == $i){
            $found = $pos;
        }
        $pos++;
    }    
    return $found;
}

sub find_and_insert{
    my ($i, @array) = @_;
    my $pos = &find($i, @array);
    if ($pos == @array){
        push @array, $i;
    }
    return @array;
}

sub get_name{
    my ($id, $ind) = @_;
    my $named = "";
    if ($id == $N_i){
        $named = $n[$ind];
    }
    elsif($id == $PROCS_i){
        $named = $num_procs[$ind];
    }
    elsif($id == $ALG_i){
        $named = $algo_names[$ind];
    }
    elsif($id == $BS_i){
        $named = $block_size[$ind];
    }
    return $named;
}

#make plot, joining data for two lists in one
sub make_plot{
    my ($swap_comp, $file_p, $data, $kind, $lines, $xaxis, $f0, $f1, $i_f0, $i_f1) = @_;
    #Get names
    my $data_n = $data_type_names[$data];
    my $kind_n = $input_kind_names[$kind];
    $name[$f0] = &get_name($f0, $i_f0);
    $name[$f1] = &get_name($f1, $i_f1);

    my $ext = $png_ps;
    if ($png_ps eq "epslatex"){
        $ext="eps";
    }
    my $file_plot=$file_p . "/" . $file_p . "-" . $data_n . "-" . "$kind_n" . "-" . $titles[$f0] . $name[$f0] . "-" . "$titles[$f1]" . "$name[$f1]" . "-" . "$swap_comp" . "." . $ext;

    #tmp file for configuration of plot
    open(FILE_TMP_PLOT,">$dir_plot/$file_p/config_plot")|| die("can't open datafile: $!");
    if ($png_ps eq "png"){
        print FILE_TMP_PLOT "set terminal $png_ps\n";
    }
    elsif($png_ps eq "eps") {
        print FILE_TMP_PLOT "set terminal postscript enhanced\n";
    }    
    else{
        print FILE_TMP_PLOT "set terminal epslatex color\n";
    }
    print FILE_TMP_PLOT "set size 1,1\n";

    print FILE_TMP_PLOT "set title \"input($data_n, $kind_n) $titles[$f0]($name[$f0]) $titles[$f1]($name[$f1])\" \n";
    
    if ($speed_abs_cnt == 2){
        #print FILE_TMP_PLOT "set logscale y 2\n";
    }
    else{
        print FILE_TMP_PLOT "set yrange [0:]\n";    
    }
    if ($xaxis == $N_i || $xaxis == $BS_i){
        print FILE_TMP_PLOT "set logscale x 10\n";
    } 
    print FILE_TMP_PLOT "set pointsize 1.25\n"; 
    print FILE_TMP_PLOT "set key left top\n";     
    #print FILE_TMP_PLOT "set key outside\n"; 
    print FILE_TMP_PLOT "set key right bottom\n";
    print FILE_TMP_PLOT "set size 0.75,0.75\n";

    @vaxis= &get_array($xaxis);
    my $xmin = $vaxis[0];
    my $xmax = $vaxis[$#vaxis];
    print FILE_TMP_PLOT "set xrange [$xmin:$xmax]\n";     

    print FILE_TMP_PLOT "set xlabel \"$titles[$xaxis]\"  \n";
  
    if ($speed_abs_cnt == 0){ #speedup
        print FILE_TMP_PLOT "set ylabel \"speedup\" \n";
    }
    elsif ($speed_abs_cnt == 1){
        print FILE_TMP_PLOT "set ylabel \"scaled time (in sec)\" \n";
    }
    elsif ($speed_abs_cnt == 2){
        print FILE_TMP_PLOT "set key left top\n";
        if ($swap_comp == 0){
            print FILE_TMP_PLOT "set ylabel \"extra swap operations (/block size)\" \n";
        }
        else{
            print FILE_TMP_PLOT "set ylabel \"extra comparison operations (/block size)\" \n";
        }
    }
    print FILE_TMP_PLOT "set output \"$dir_plot/$file_plot\"\n";
      
    #lines to print 
    @vlines= &get_array($lines);
    my $extra_name = "";
    my $str="";
    my $is_first = 1;
    foreach $i (0..$#vlines){
        my $j=1+$i;
        $file_tmp = &get_tmp_filename($i);
        $name_line = &get_name($lines, $i);
        if ($is_first == 1){
            $str="plot \"$dir_plot/$file_p/$file_tmp\" using 1:2 title \'$name_line$extra_name\' with linespoints 1";
            $is_first = 0;
        }
        else{
            $str=$str . " , " . "\"$dir_plot/$file_p/$file_tmp\" using 1:2 title \'$name_line$extra_name\' with linespoints $j";
        }
    }

    $str=$str . "\n";
    print FILE_TMP_PLOT $str;
    close(FILE_TMP_PLOT);
    `gnuplot $dir_plot/$file_p/config_plot`;
    #`rm $dir_plot/$file_p/config_plot`;
}

sub make_html{
    my ($cur_op,$cur_sub_op,$file_p)=@_;
    my $file_html=$file_p . ".html";
    
    open(FILE_ALL_PLOT,">$dir_plot/$file_html")|| die("can't open datafile: $!");
    print FILE_ALL_PLOT "<html>\n";
    print FILE_ALL_PLOT "<body>\n";
    
    @file_plots=`ls -1 $dir_plot/$file_p`;
    foreach $line (@file_plots){
    chomp($line);
    if ($line=~/$file_p.*/){
    print FILE_ALL_PLOT "<img src=\"$file_p/$line\">\n<br>\n";
    }
    }
    print FILE_ALL_PLOT "</body>\n";
    print FILE_ALL_PLOT "</html>\n";
    close(FILE_ALL_PLOT);

}

sub make_latex{
    my ($cur_op,$cur_sub_op,$file_p)=@_;
    my $file_latex=$file_p . ".tex";
    
    open(FILE_ALL_PLOT,">$dir_plot/$file_latex")|| die("can't open datafile: $!");
    print FILE_ALL_PLOT "\\documentclass{article}\n";
    print FILE_ALL_PLOT "\\usepackage{graphicx}\n";
    print FILE_ALL_PLOT "\\begin{document}\n";

    @file_plots=`ls -1 $dir_plot/$file_p/*.tex`;
    foreach $line (@file_plots){
    chomp($line);
    if ($line=~/$file_p.*/){
        $s= "\\begin{figure}\n" . "\\input{$line} \n ".
           "\\end{figure}\n";
        print FILE_ALL_PLOT $s;
        }
    }
    print FILE_ALL_PLOT "\\end{document}\n";
    #`latex $dir_plot/$file_latex`;
    #`dvips $dir_plot/$file_p`;
    close(FILE_ALL_PLOT);

}
