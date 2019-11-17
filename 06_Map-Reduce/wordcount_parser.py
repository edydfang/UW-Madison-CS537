import sys
import optparse

parser = optparse.OptionParser()
parser.add_option("--check-num-mappers", action="store", type="int", dest="check_num_mappers", default=None, help="check the number of mappers if set")
parser.add_option("--check-num-reducers", action="store", type="int", dest="check_num_reducers", default=None, help="check the number of reducers if set")
parser.add_option("--input-file", action="store", type="string", dest="input_file", default=None, help="the input file name")
options, args = parser.parse_args()
if options.input_file == None:
    parser.print_help()
    exit(1)

with open(options.input_file, "r") as wordcount_out:
    lines = wordcount_out.readlines()
    mapper_tid_set = set()
    reducer_tid_set = set()
    partition_number_set = set()
    keys_dict = dict()
    for line in lines:
        splitted = line[:-1].split(" ")
        if splitted[0] == "Mapper":
            mapper_tid_set.add(int(splitted[1]))
        elif splitted[0] == "Reducer":
            reducer_tid_set.add(int(splitted[4]))
            partition_number_set.add(int(splitted[3]))
            key = splitted[1]
            value = splitted[2]
            if keys_dict.get(key) != None:
                print("Error: Multiple output of same key", file=sys.stderr)
            keys_dict[key] = value

    if options.check_num_mappers != None:
        if len(mapper_tid_set) != options.check_num_mappers:
            print("Error: Number of mappers not as required", file=sys.stderr)
    
    if options.check_num_reducers != None:
        if len(reducer_tid_set) != options.check_num_reducers:
            print("Error: Number of reducers not as required", file=sys.stderr)

    for key, value in sorted(keys_dict.items()):
        print(key + " " + value)
    