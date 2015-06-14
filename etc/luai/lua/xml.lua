local function prettyPrint( node, file, ident )
  file = file or io.stdout
  ident = ident or 0

  if type( node ) == 'table' then
    file:write( ( ' ' ):rep( ident ), '<', node.label )
    
    for attr, value in pairs( node.xarg ) do
      file:write( ' ', attr, '="', value, '"' )
    end
    
    if node.empty then
      file:write( '/>\n' )
    else
      file:write( '>\n' )
      
      for _, child in ipairs( node ) do
        prettyPrint( child, file, ident + 2 )
      end
      
      file:write( ( ' ' ):rep( ident ), '</', node.label, '>\n' )
    end
  else
    file:write( ( ' ' ):rep( ident ), node, '\n' )
  end
end

local function findNode( node, label )
  if type( node ) == 'table' then
    if node.label == label then
      return node
    end
    
    for i = 1, #node do
      local res = findNode( node[ i ], label )
      
      if res then
        return res
      end
    end
  end
end
  
local function parseargs(s)
  local arg = {}
  string.gsub(s, "([%w:]+)=([\"'])(.-)%2", function (w, _, a)
    arg[w] = a
  end)
  return arg
end

return {
  parse = function(s)
    local stack = {}
    local top = {}
    table.insert(stack, top)
    local ni,c,label,xarg, empty
    local i, j = 1, 1
    while true do
      ni,j,c,label,xarg, empty = string.find(s, "<(%/?)([%w:]+)(.-)(%/?)>", i)
      if not ni then break end
      local text = string.sub(s, i, ni-1)
      if not string.find(text, "^%s*$") then
        table.insert(top, text)
      end
      if empty == "/" then  -- empty element tag
        table.insert(top, {label=label, xarg=parseargs(xarg), empty=1})
      elseif c == "" then   -- start tag
        top = {label=label, xarg=parseargs(xarg)}
        table.insert(stack, top)   -- new level
      else  -- end tag
        local toclose = table.remove(stack)  -- remove top
        top = stack[#stack]
        if #stack < 1 then
          error("nothing to close with "..label)
        end
        if toclose.label ~= label then
          error("trying to close "..toclose.label.." with "..label)
        end
        table.insert(top, toclose)
      end
      i = j+1
    end
    local text = string.sub(s, i)
    if not string.find(text, "^%s*$") then
      table.insert(stack[#stack], text)
    end
    if #stack > 1 then
      error("unclosed "..stack[#stack].label)
    end
    return stack[1][1]
  end,

  findNode = findNode,
  
  findAttr = function( node, name )
    for key, value in pairs( node.xarg ) do
      if key == name then
        return value
      end
    end
  end,
  
  prettyPrint = prettyPrint
}
