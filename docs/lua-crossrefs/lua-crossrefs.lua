-- libzint - the open source barcode library
-- Copyright (C) 2025 Robin Stuart <rstuart114@gmail.com>

-- Hacked from https://github.com/rnwst/pandoc-lua-crossrefs
-- © 2025 R. N. West. Released under the GPL version 2 or greater.
-- SPDX-License-Identifier: GPL-2.0-or-later

PANDOC_VERSION:must_be_at_least '3.8.2'

-- Hacked from "pandoc-lua-crossrefs/init.lua"

-- Table of Ids and corresponding cross-referenceable elements. To be populated
-- by various element numbering functions.
---@type table<string, {type: ('fig'|'tbl'), number: string, caption: string}>
local IDs = {}

-- Hacked from "pandoc-lua-crossrefs/lib/crossrefs.lua"

---Check if AST element is a cross-reference (a cross-reference is a Span with class 'cross-ref').
---@param inline Inline
---@return boolean
local _is_crossref = function(inline)
   local is_crossref_link = inline and inline.tag == 'Link' and inline.attributes['reference-type'] ~= nil
   return is_crossref_link
end

---Parse a cross-reference in Pandoc's Markdown.
---@param str Str
---@return Inline[] | nil
local _parse_crossref = function(str)
   local opening_paren, opening_bracket, prefix_suppressor, id, closing_bracket1, punctuation, closing_bracket2 =
      str.text:match('^(%(?)(%[?)(%-?)#([%a%d-_:%.]-)(%]?)([\\%.!:?,;)]-)(%]?)$')
   if not id or id == '' then return end
   local only_internal_punctuation = id:find('^[%a%d]+[%a%d%-_:%.]*[%a%d]+$') or id:find('^[%a%d]+$')
   if not only_internal_punctuation then return end
   if #closing_bracket1 ~= 0 and #closing_bracket2 ~= 0 then return end

   local crossref = pandoc.Link({}, '#' .. id, '', pandoc.Attr('', {}, { ['reference-type'] = 'ref+label' }))
   if prefix_suppressor == '-' then crossref.attributes['reference-type'] = 'ref' end
   local elts = pandoc.List { crossref }
   if opening_bracket == '[' then elts:insert(1, pandoc.Str('[')) end
   if opening_paren == '(' then elts:insert(1, pandoc.Str('(')) end
   if closing_bracket1 == ']' then elts:insert(pandoc.Str(']')) end
   if punctuation ~= '' then elts:insert(pandoc.Str(punctuation)) end
   if closing_bracket2 == ']' then elts:insert(pandoc.Str(']')) end

   return elts
end

---Parse cross-references in Inlines.
---@param inlines Inlines
---@return (Inline[] | Inlines | nil), boolean?
local parse_crossrefs = function(inlines)
   -- Parse cross-references into Links.
   local new_inlines = inlines:walk { Str = _parse_crossref }

   -- Early return if no cross-references were found!
   if new_inlines == inlines then return end

   inlines = new_inlines

   -- Now separate out any opening or closing brackets in Strs into separate
   -- Strs, in case crossref groups don't begin and end with cross-references.
   inlines = inlines:walk {
      Str = function(str)
         if str.text:find('^%[.') then
            return { pandoc.Str('['), pandoc.Str(str.text:sub(2)) }
         elseif str.text:find('.%]$') then
            return { pandoc.Str(str.text:sub(1, -2)), pandoc.Str(']') }
         end
      end,
   }

   -- Now create crossref groups. Crossref Groups are represented by Spans of
   -- class 'cross-ref-group'.
   ---@type List<Inline>
   new_inlines = pandoc.List {}
   local i = 1
   while inlines[i] do
      if i < #inlines and inlines[i].tag == 'Str' and inlines[i].text == '[' then
         ---@type boolean
         local at_least_one_crossref = false
         ---@type List<Inline>
         local group_content = pandoc.List {}
         ---@type boolean
         local group_valid = false
         local j = i + 1
         while inlines[j] do
            local elt = inlines[j]
            if elt.tag == 'Str' and elt.text == ']' then
               if at_least_one_crossref then
                  group_valid = true
                  break
               else
                  group_valid = false
                  break
               end
               -- Another opening bracket invalidates the group if no cross-reference has yet been
               -- found. This ensures that the smallest possible Crossref Groups are created.
            elseif elt.tag == 'Str' and elt.text == '[' and not at_least_one_crossref then
               group_valid = false
               break
            else
               if _is_crossref(elt) then at_least_one_crossref = true end
               group_content:insert(inlines[j])
            end
            j = j + 1
            if _is_crossref(elt) and inlines[j] and inlines[j].tag == 'Str' then
               if inlines[j].text == ';' and inlines[j + 1] and inlines[j + 1].tag == 'Space' then
                  -- Skip punctuation following crossref if it is ';'.
                  j = j + 2
               elseif inlines[j].text == '\\;' then
                  -- To still allow a semicolon to be used to separate cross-references, an
                  -- escaped semicolon is converted to a semicolon.
                  group_content:insert(pandoc.Str(';'))
                  j = j + 1
               end
            end
         end
         if group_valid then
            -- Insert Crossref Group into inlines.
            local crossref_group = pandoc.Span(group_content, pandoc.Attr('', { 'cross-ref-group' }))
            new_inlines:insert(crossref_group)
            i = j
         else
            new_inlines:insert(inlines[i])
         end
      else
         new_inlines:insert(inlines[i])
      end
      i = i + 1
   end

   return new_inlines, false -- Nested cross-references are not allowed!
end

---Get cross-reference target
---@param crossref Link
---@return { type: 'fig'|'tbl', number: string, caption: string }
local _get_target = function(crossref) return IDs[crossref.target:sub(2)] end

---Resolve cross-reference.
---@param crossref         Link     cross-reference
---@param suppress_prefix? boolean  whether to suppress prefixing the referenced object's type (e.g. 'Fig.' or 'Tbl.')
---@return Link
local _resolve_crossref = function(crossref, suppress_prefix)
   local target = _get_target(crossref)
   local crossref_text = ''
   if target ~= nil then
      if crossref.attributes['reference-type'] == 'ref+label' and not suppress_prefix then
         if target.type == 'fig' then
            crossref_text = 'Figure '
         elseif target.type == 'tbl' then
            crossref_text = 'Table '
         end
      end
      crossref_text = crossref_text .. target.number .. ': ' .. target.caption
   else
      crossref_text = '??'
      pandoc.log.warn('Cross-referenced element with id ' .. tostring(crossref.target) .. ' could not be resolved.')
   end
   local link = pandoc.Link(crossref_text, crossref.target)
   link.attr = pandoc.Attr('', { 'cross-ref' })
   return link
end

---Resolve single cross-references.
---@param link Link
---@return Link?
local write_crossref = function(link)
   if _is_crossref(link) then return _resolve_crossref(link) end
end

-- Hacked from "pandoc-lua-crossrefs/lib/numbering.lua"

local figure_number = 0
local table_number = 0
---Number figure or table.
---@param fig_or_tbl (Figure | Table)
---@return (Figure | Table), false  Numbered Figure or Table, or `nil` if unnumbered
---@overload fun(fig_or_tbl: Figure | Table): nil
local number_fig_or_tbl = function(fig_or_tbl)
   if not fig_or_tbl.classes:includes('unnumbered') then
      ---@type string
      local _type
      ---@type integer
      local number
      ---@type string
      local label_class
      ---@type fun(num: integer): string
      local number_formatter = function(num) return tostring(num) end
      ---@type fun(num: integer): string
      local label_formatter
      ---@type boolean
      local colon_after_label = true

      if fig_or_tbl.tag == 'Figure' then
         _type = 'fig'
         figure_number = figure_number + 1
         number = figure_number
         label_class = 'figure-label'
         label_formatter = function(num) return string.format('Figure %s', num) end
      end

      if fig_or_tbl.tag == 'Table' then
         _type = 'tbl'
         table_number = table_number + 1
         number = table_number
         label_class = 'table-label'
         label_formatter = function(num) return string.format('Table %s', num) end
      end

      ---Add Fig or Tbl to table of Ids, prepend label to caption.
      ---@param elt (Figure | Table)
      local function process_fig_or_tbl(elt)
         if elt.identifier ~= '' then
            local caption = pandoc.utils.stringify(elt.caption.long)
            IDs[elt.identifier] = { type = _type, number = number_formatter(number), caption = caption }
         end
         local caption_prefix = pandoc.Span({ pandoc.Str(label_formatter(number)) }, pandoc.Attr('', { label_class }))
         if FORMAT ~= 'latex' then
            -- If figure or table caption is not empty, append colon to number.
            if #elt.caption.long ~= 0 then
               if colon_after_label then
                  caption_prefix.content[1].text = caption_prefix.content[1].text .. ':'
                  elt.caption.long[1].content:insert(1, pandoc.Space())
               end
               elt.caption.long[1].content:insert(1, caption_prefix)
            else
               elt.caption.long:insert(pandoc.Plain(caption_prefix))
            end
         end
      end

      process_fig_or_tbl(fig_or_tbl)

      -- Number subfigs.
      if _type == 'fig' then
         number = 0
         number_formatter = function(num) return figure_number .. label_formatter(num) end
         label_formatter = function(num) return string.format('(%s)', string.char(96 + num)) end
         colon_after_label = false
         fig_or_tbl = fig_or_tbl:walk {
            Figure = function(subfig)
               number = number + 1
               process_fig_or_tbl(subfig)
               return subfig
            end,
         }
      end

      return fig_or_tbl, false -- Return `false` as second value to avoid processing subfigures again.
   end
end

-- Hacked from "pandoc-lua-crossrefs/init.lua"

---@param doc Pandoc
function Pandoc(doc)
   return doc
      :walk({
         Inlines = parse_crossrefs,
      })
      :walk({
         -- Number cross-referenceable elements and construct table with Ids and numbers.
         traverse = 'topdown', -- needed for subfigs
         Figure = number_fig_or_tbl,
         Table = number_fig_or_tbl,
      })
      :walk {
         -- Resolve single cross-references.
         Link = write_crossref,
      }
end
