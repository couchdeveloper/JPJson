#!/usr/bin/ruby


require 'pathname'

puts "CreateDoc JPJsonParser/json/ObjC with #{%x(/usr/local/bin/appledoc --version)}"


src_root = ENV['JPSOURCE_ROOT']
dest_dir = ENV['DERIVED_FILE_DIR']
puts "cd #{src_root}"
Dir.chdir src_root

source_path = Pathname.new("#{src_root}/json/ObjC").cleanpath
dest_path = Pathname.new("#{dest_dir}").cleanpath

appledoc_command = "/usr/local/bin/appledoc"

project_name = "JPJson"
project_version = "0.1"
project_company = "|–|"


exclude_paths = "-x .m --ignore .m"

appledoc_ouput_options = "--no-create-docset --keep-intermediate-files --create-html"

apple_doc_warnings = "--warn-undocumented-object --warn-undocumented-member --warn-empty-description --warn-unknown-directive --warn-invalid-crossref --warn-missing-arg"

apple_doc_options = "--no-repeat-first-par --no-keep-undocumented-objects --no-keep-undocumented-members --prefix-merged-sections --no-search-undocumented-doc --explicit-crossref"


command_options = "-p \"#{project_name}\" -v \"#{project_version}\" -c \"#{project_company}\" -o \"#{dest_path}\""\
                  << " #{exclude_paths} #{appledoc_ouput_options} #{apple_doc_warnings} #{apple_doc_options}"\
                  << "  --logformat xcode --exit-threshold 2"

command = "#{appledoc_command} #{command_options} \"#{source_path}\""
puts command
system command
result = $?.success?

openCmd = "open #{dest_path}/html/index.html"
system openCmd
