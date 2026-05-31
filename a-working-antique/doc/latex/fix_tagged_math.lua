-- Pandoc Lua filter: convert display math containing \tag{} to equation*
-- environments, which amsmath allows \tag inside (unlike \[...\]).
function Para(el)
  if #el.content == 1
     and el.content[1].t == "Math"
     and el.content[1].mathtype == "DisplayMath"
     and el.content[1].text:find("\\tag") then
    local body = el.content[1].text
    return pandoc.RawBlock("latex",
      "\\begin{equation*}\n" .. body .. "\n\\end{equation*}")
  end
end
