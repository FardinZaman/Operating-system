#!/bin/bash

#echo "$1"
#echo "$2"

working_dir="working_dir"

if [ $# -eq 2 ];then
    working_dir="$1"
    if [ -f "$2" ];then
          input_file="$2"
          echo "File found"
    else
          echo "File not found"
          exit
    fi
else
    if [ -f "$1" ];then
          input_file="$1"
          echo "File found"
    else
          echo "File not found"
          exit
    fi
fi

tracker=1 
while read line
do
    if [ $tracker -eq 1 ];then
         direction=$line
    elif [ $tracker -eq 2 ];then
         number=$line
    else
         data=$line
    fi 
    tracker=$((tracker+1))
done <$input_file 

echo "$direction"
echo "$number"
echo "$data"

#i=$((number+1))
#echo $i

#expr $number + 1

if [ $direction == "begin" ];then
    echo "start from begin"
elif [ $direction == "end" ];then
    echo "start from end"
fi

output_dir="output"
csv_file="output.csv"

rm -r "$output_dir"
mkdir "$output_dir"

rm "$csv_file"
touch "$csv_file"

echo "File Path,Line Number,The Actual Line" >> "$csv_file"

total_file=0

traverse()
{
    for f in "$1"/*;do
        if [ -d "$f" ];then
            traverse "$f"
        elif [ -f "$f" ];then
            final_line_no=0
            if [ $direction == "begin" ];then
                 head -n "$number" "$f" > temp.txt
                 first_match=$(grep -n -i -m1 "$data" temp.txt)
                 if [ ! "$first_match" == "" ];then
		         line_no=$( echo "$first_match" | cut -d ':' -f1 )
                         final_line_no=$line_no
		         #line_matched=$(grep -h -i -m1 "$data" temp.txt)
                         grep -n -i "$data" temp.txt > temp_matched.txt
                         while read line
                         do
                              line_no=$( echo "$line" | cut -d ':' -f1 )
                              line_matched=$( echo "$line" | cut -d ':' -f2 )
                              echo "${f},${line_no},${line_matched}" >> "$csv_file"
                         done < temp_matched.txt
                         total_file=$((total_file+1))
		         #if [ -n $line_no ];then
		                 #echo "${f},${line_no},${line_matched}" >> "$csv_file"
		         #fi
		         #echo "file : $f"
		         #echo "line : $line_no : $line_matched"
                 fi
            else
                 total_line=$(cat "$f" | wc -l)
                 number2=$number
                 if [ $total_line -lt $number ];then
                         number2=$total_line
                 fi
                 tail -n "$number" "$f" > temp2.txt
                 last_match=$(grep -i -n "$data" temp2.txt | tail -1)
                 if [ ! "$last_match" == "" ];then
		         temp=$( echo "$last_match" | cut -d ':' -f1 )
		         line_no2=$((total_line-number2+temp))
                         final_line_no=$line_no2
                         #line_matched2=$( echo "$last_match" | cut -d ':' -f2 )
                         grep -n -i "$data" temp2.txt > temp_matched2.txt
                         while read line
                         do
                              temp2=$( echo "$line" | cut -d ':' -f1 )
                              line_no2=$((total_line-number2+temp2))
                              line_matched2=$( echo "$line" | cut -d ':' -f2 )
                              echo "${f},${line_no2},${line_matched2}" >> "$csv_file"
                         done < temp_matched2.txt
                         total_file=$((total_file+1))
                         #echo "$line_matched2"
		         #echo "$line_no2"
                         #if [ -n $line_no2 ];then
                              #echo "${f},${line_no2},${line_matched2}" >> "$csv_file"
                         #fi
                 fi
                 
            fi 
        fi

        extension="$(echo ${f} | cut -d '.' -f2)"
        #if [ "$extension" == "$f" ]
        if [ -n "$extension" ];then
            if [ "$extension" == "$f" ];then
                extension=""
            else
                extension=".${extension}"
            fi
        fi

        #echo "$extension"

        filename=$(echo $f | cut -d '.' -f1 | sed -e 's/\//./g' -e 's/ //g')
        nothing=$(echo $filename | cut -d '.' -f1)
        if [ "$nothing" == "" ];then
            filename=$(echo $filename | sed 's/.//')
        fi
        #echo "$filename"
        if [ ! $final_line_no -eq 0 ];then
		newfile="${output_dir}/${filename}_${final_line_no}${extension}"
		#echo $newfile
		touch "$newfile"
		cp "$f" "$newfile"
        fi
    done
} 

traverse $working_dir

#total_matched=$(cat "$csv_file" | wc -l)
echo "Total files : "
echo "$total_file"
#expr $total_matched - 1

