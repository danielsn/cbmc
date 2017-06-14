/*******************************************************************\

Module: Goto Program

Author: Thomas Kiley

\*******************************************************************/

/// \file
/// Goto Program

#include <iostream>
#include <sstream>


#include <util/xml_expr.h>
#include <util/cprover_prefix.h>
#include <util/prefix.h>

#include <langapi/language_util.h>

#include "goto_functions.h"
#include "goto_model.h"

#include "show_goto_functions_xml.h"

/// For outputing the GOTO program in a readable xml format.
/// \param ns: the namespace to use to resolve names with
show_goto_functions_xmlt::show_goto_functions_xmlt(const namespacet &ns):
  ns(ns)
{}

/// Walks through all of the functions in the program and returns an xml object
/// representing all their functions. Produces output like this: <functions>
/// <function name=main, is_body_available=true, is_internal=false>
/// <instructions> <instruction_id=ASSIGN> <location file="main.c" line="14"/>
/// <instruction_value> // 34 file main.c line 1 s = { 'a', 'b', 'c', 0 };
/// </instruction_value> </instruction> </instructions> </function> </functions>
/// \param goto_functions: the goto functions that make up the program
xmlt show_goto_functions_xmlt::convert(
  const goto_functionst &goto_functions)
{
  xmlt xml_functions=xmlt("functions");
  for(const auto &function_entry : goto_functions.function_map)
  {
    const irep_idt &function_name=function_entry.first;
    const goto_functionst::goto_functiont &function=function_entry.second;

    xmlt &xml_function=xml_functions.new_element("function");
    xml_function.set_attribute("name", id2string(function_name));
    xml_function.set_attribute_bool(
      "is_body_available", function.body_available());
    bool is_internal=(has_prefix(id2string(function_name), CPROVER_PREFIX) ||
                      function_name==goto_functions.entry_point());
    xml_function.set_attribute_bool("is_internal", is_internal);

    if(function.body_available())
    {
      xmlt &xml_instructions=xml_function.new_element("instructions");
      for(const goto_programt::instructiont &instruction :
        function.body.instructions)
      {
        xmlt &instruction_entry=xml_instructions.new_element("instruction");

        instruction_entry.set_attribute(
          "instruction_id", instruction.to_string());

        if(instruction.code.source_location().is_not_nil())
        {
          instruction_entry.new_element(
            xml(instruction.code.source_location()));
        }

        std::ostringstream instruction_builder;
        function.body.output_instruction(
          ns, function_name, instruction_builder, instruction);

        xmlt &instruction_value=
          instruction_entry.new_element("instruction_value");
        instruction_value.data=instruction_builder.str();
        instruction_value.elements.clear();
      }
    }
  }
  return xml_functions;
}

/// Print the xml object generated by
/// show_goto_functions_xmlt::show_goto_functions to the provided stream (e.g.
/// std::cout)
/// \param goto_functions: the goto functions that make up the program
/// \param out: the stream to write the object to
/// \param append: should a command and newline be appended to the stream before
///   writing the xml object. Defaults to true
void show_goto_functions_xmlt::operator()(
  const goto_functionst &goto_functions,
  std::ostream &out,
  bool append)
{
  if(append)
  {
    out << "\n";
  }
  out << convert(goto_functions);
}
