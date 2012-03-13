#!/usr/bin/ruby


require 'pathname'

puts "CreateDoc JPJsonParser/json/ObjC"

src_root = ENV['JP_ROOT']
dest_dir = ENV['DERIVED_FILE_DIR']
puts "cd #{src_root}"
Dir.chdir src_root

source_path = Pathname.new("#{src_root}/json/ObjC").cleanpath
dest_path = Pathname.new("#{dest_dir}").cleanpath

apple_doc_command = "/usr/local/bin/appledoc -p JPJson -v \"0.1\" -c \"Bit Passion\""
apple_doc_command << " -o " << "\"" << dest_path << "\"" 
apple_doc_command << " --warn-undocumented-object --warn-undocumented-member --warn-empty-description --warn-unknown-directive --warn-invalid-crossref --warn-missing-arg --no-repeat-first-par --explicit-crossref"

apple_doc_command << " --no-create-docset --keep-intermediate-files --create-html"
apple_doc_command << " --logformat xcode --exit-threshold 2"

command = apple_doc_command << " \"" << source_path << "\"" 
puts command
system command
result = $?.success?

openCmd = "open #{dest_path}/html/index.html"
system openCmd
