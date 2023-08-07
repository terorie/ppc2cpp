
#include <filesystem>
#include <fstream>
#include <format>
#include <unordered_map>
#include <cstdint>

#include <yaml-cpp/yaml.h>

#include "ppc2cpp/common/common.h"
#include "ppc2cpp/common/ProjectPersistenceException.hpp"
#include "ppc2cpp/program_loader/Rel.hpp"
#include "ppc2cpp/model/Symbol.hpp"

#include "labelinfo.pb.h"
#include "relocinfo.pb.h"

#include "ppc2cpp/analysis/PpcdisAnalysis.hpp"

namespace ppc2cpp {
void loadBinaryInfoFromPpcdis(ProgramLoaderPtr pLoader, const std::string& bin_yaml, const std::string& label_proto, const std::string& reloc_proto,
    const std::unordered_map<uint32_t, std::string>& namemap) {
  YAML::Node bin_config = YAML::LoadFile(bin_yaml);
  const std::string bin_path = bin_config["path"].as<std::string>();
  std::string bin_filename = std::filesystem::path(bin_path).filename().string();

  int binary_idx = pLoader->findBinaryIdxByName(bin_filename);
  ASSERT(binary_idx >= 0, "Could not find ppcdis binary named " + bin_filename + " in project");

  // if REL, apply REL load information so that addresses can be resolved into program locations
  if (RelPtr rel = dynamic_pointer_cast<Rel>(pLoader->binaries[binary_idx])) {
    rel->setLoadInfo(bin_config["address"].as<uint32_t>(), bin_config["bss_address"].as<uint32_t>());
  }
  
  ppcdis::LabelInfo labelinfo;
  ppcdis::RelocInfo relocinfo;

  std::ifstream lfstream(label_proto, std::ios::binary | std::ios::in);
  if (!labelinfo.ParseFromIstream(&lfstream)) {
    throw new ProjectPersistenceException("Failed to read ppcdis labels proto");
  }
  std::ifstream rfstream(reloc_proto, std::ios::binary | std::ios::in);
  if (!relocinfo.ParseFromIstream(&rfstream)) {
    throw new ProjectPersistenceException("Failed to read ppcdis relocs proto");
  }

  // import symbol information
  SymbolTable& symtab = pLoader->symtab;
  for (auto it : labelinfo.labels()) {
    uint32_t addr = it.first;
    ASSERT(pLoader->resolveVMA(addr).has_value(), "Could not resolve VMA " + std::format("0x{:x}", addr));
    ProgramLocation symLoc = pLoader->resolveVMA(addr).value();

    // Add symbol if it doesn't exist
    if (symtab.lookupByLocation(symLoc).has_value()) continue;
    std::string name;
    // name the symbol based on the symbol map if it exists, otherwise use sensible default
    if (auto foundname = namemap.find(addr); foundname != namemap.end()) {
      name = foundname->second;
    } else {
      name = std::format("lab_{:x}", addr);
    }
    ppcdis::Label ppcdisLabel = it.second;

    switch (ppcdisLabel.type())
    {
    case ppcdis::DATA:
        symtab.push_back(Symbol(symLoc, ppcdisLabel.size(), name, DATA, GLOBAL));
        break;
    case ppcdis::FUNCTION:
        symtab.push_back(Symbol(symLoc, ppcdisLabel.size(), name, FUNCTION, GLOBAL));
        break;
    
    default:
        break;
    }
  }
  
  // import symbol information
  RelocationTable& reloctab = pLoader->reloctab;
  for (auto it : relocinfo.relocs()) {
    uint32_t addr = it.first;
    ASSERT(pLoader->resolveVMA(addr).has_value(), "Could not resolve VMA " + std::format("0x{:x}", addr));
    ProgramLocation location = pLoader->resolveVMA(addr).value();

    // Add relocation if it doesn't exist
    if (reloctab.lookupBySource(location).has_value()) continue;
    ppcdis::Reloc ppcdisReloc = it.second;
    ASSERT(pLoader->resolveVMA(ppcdisReloc.target()).has_value(), "Could not resolve VMA " + std::format("0x{:x}", ppcdisReloc.target()));
    ProgramLocation target = pLoader->resolveVMA(ppcdisReloc.target()).value();
    throw std::runtime_error("TODO: deduce reloc type");// TODO deduce reloc type
    reloctab.push_back(Relocation(location, target, 69, ppcdisReloc.offset()));
  }
}

void loadAnalysisFromPpcdis(ProgramLoaderPtr pLoader, const std::vector<PpcdisInfoFiles>& ppcdisInfo, const std::string& symmapPath) {
  YAML::Node symmapYaml = YAML::LoadFile(symmapPath);
  YAML::Node globalSyms = symmapYaml["global"];

  // create a hashtable that maps vma to name
  std::unordered_map<uint32_t, std::string> namemap;
  for(YAML::const_iterator it = globalSyms.begin(); it != globalSyms.end(); ++it) {
    namemap[it->first.as<uint32_t>()] = it->second.as<std::string>();
  }

  for (const auto& ppcdisBin : ppcdisInfo) {
    loadBinaryInfoFromPpcdis(pLoader, ppcdisBin.bin_yaml, ppcdisBin.label_proto, ppcdisBin.reloc_proto, namemap);
  }
}
}