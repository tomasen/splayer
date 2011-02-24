#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import optparse
import yaml
import re
import os.path
import zlib
import base64

def main():
  p = optparse.OptionParser()
  p.add_option('--input', '-i', default="")
  options, arguments = p.parse_args()

  if len(options.input) == 0:
    print """
      Usage: compile_rsc.py [OPTIONS]
             --input=\"/path/input.yaml\"     Compile input.yaml into input.yaml_bz
    """
    return 1

  print 'Processing '+options.input+' ...'
  sys.stdout.flush()
  if os.path.isfile(options.input) == False:
    print 'Error: Input file "'+options.input+'" does not exist'
    return 1

  real_input = os.path.realpath(options.input)

  input_stream = file(real_input, 'rb')
  try:
    input_doc = yaml.load(input_stream)
  except yaml.YAMLError, exc:
    if hasattr(exc, 'problem_mark'):
      mark = exc.problem_mark
      print "Error in input file position: (%s:%s)" % (mark.line+1, mark.column+1)
    return 1
  input_stream.close()

  m = re.search(r"^(.+[\\/])+", real_input)
  if m == None:
    print "Error: cannot retrieve path from input file: "+real_input
    return 1
  
  input_path = m.group(1)
  print "Input path is: "+input_path
  sys.stdout.flush()

  #print yaml.dump(input_doc, default_flow_style=False)

  for nodename_1, nodevar_1 in input_doc.items():
    for nodename_2, nodevar_2 in nodevar_1.items():
      m = re.search(r"<<<([^<>]+)>>>", nodevar_2)
      if m != None:
        print "Binary streaming " + m.group(1) + " ..."
        buffer = open(input_path + m.group(1), 'rb').read();
        input_doc[nodename_1][nodename_2] = buffer
        sys.stdout.flush()

  print "Encoding resource yaml ..."
  sys.stdout.flush()
  output_yaml = options.input+"_b"
  output_stream = file(output_yaml, 'wb')
  yaml.safe_dump(input_doc, output_stream, default_flow_style=False, encoding='utf-8', allow_unicode=True, explicit_start=True)
  output_stream_content = yaml.safe_dump(input_doc, default_flow_style=False, encoding='utf-8', allow_unicode=True, explicit_start=True)
  print "Generated "+output_yaml
  sys.stdout.flush()

  output_yamlz = output_yaml+"z"
  output_streamz = file(output_yamlz, 'wb')
  output_streamz.write(zlib.compress(output_stream_content, 9))
  print "Generated "+output_yamlz
  sys.stdout.flush()
  return 0

main()