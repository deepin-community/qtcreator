---@meta TextEditor
local textEditor = {}

---@class Position
---@field line integer The line number.
---@field column integer The column number.
local Position = {}

---@class Range
---@field from Position The beginning position of the range.
---@field to Position The end position of the range.
local Range = {}

---@class TextCursor
local TextCursor = {}

---Returns the position of the cursor.
---@return integer position The position of the cursor.
function TextCursor:position() end

---Returns the block (line) and column for the cursor.
---@return integer block The block (line) of the cursor.
function TextCursor:blockNumber() end

---Returns the column for the cursor.
---@return integer column The column of the cursor.
function TextCursor:columnNumber() end

---Returns true if the cursor has a selection, false otherwise.
---@return boolean hasSelection True if the cursor has a selection, false otherwise.
function TextCursor:hasSelection() end

---Returns the selected text of the cursor.
---@return string selectedText The selected text of the cursor.
function TextCursor:selectedText() end

---Returns the range of selected text of the cursor.
---@return Range selectionRange The range of selected text of the cursor.
function TextCursor:selectionRange() end

---Inserts the passed text at the cursors position overwriting any selected text.
---@param text string The text to insert.
function TextCursor:insertText(text) end

---@class MultiTextCursor
local MultiTextCursor = {}

---Returns the main cursor.
---@return TextCursor mainCursor The main cursor.
function MultiTextCursor:mainCursor() end

---Returns the cursors.
---@return TextCursor[] cursors The cursors.
function MultiTextCursor:cursors() end

---Inserts the passed text at all cursor positions overwriting any selected text.
---@param text string The text to insert.
function MultiTextCursor:insertText(text) end

---@class Suggestion
local Suggestion = {}

---@class SuggestionParams 
---@field text string The text of the suggestion.
---@field position Position The cursor position where the suggestion should be inserted.
---@field range Range The range of the text preceding the suggestion.
SuggestionParams = {}

---Creates Suggestion.
---@param params SuggestionParams Parameters for creating the suggestion.
---@return Suggestion suggestion The created suggestion.
function Suggestion:create(params) end

---@class TextDocument
local TextDocument = {}

---Returns the file path of the document.
---@return FilePath filePath The file path of the document.
function TextDocument:file() end

---Returns the block (line) and column for the given position.
---@param position integer The position to convert.
---@return integer block The block (line) of the position.
---@return integer column The column of the position.
function TextDocument:blockAndColumn(position) end

---Returns the number of blocks (lines) in the document.
---@return integer blockCount The number of blocks in the document.
function TextDocument:blockCount() end

---Sets the suggestions for the document and enables toolTip on the mouse cursor hover.
---@param suggestions Suggestion[] A list of possible suggestions to display.
function TextDocument:setSuggestions(suggestions) end

---@class TextEditor
local TextEditor = {}

---Returns the document of the editor.
---@return TextDocument document The document of the editor.
function TextEditor:document() end

---Returns the cursor of the editor.
---@return MultiTextCursor cursor The cursor of the editor.
function TextEditor:cursor() end

---Adds a floating widget at the specified position in the text editor.
---The widget will be positioned at the location corresponding to the given position in the
---text document and will be automatically managed to stay pined to that position.
---@param widget Widget|Layout The widget to be added as a floating widget.
---@param position integer The position in the document where the widget should appear.
---@param margins integer[] Four integers, representing left, top, right, bottom margins
---@param xPos integer Sets widget to fixed x position if x != -1, otherwise automatic x position calculation is done
---@param fillWidth boolean If true, the widget will fill remaining space from its x position to size of the TextEditor viewport
function TextEditor:addFloatingWidget(widget, position, xPos, margins, fillWidth) end

---Checks if the current suggestion is locked. The suggestion is locked when the user can use it.
---@return boolean True if the suggestion is locked, false otherwise.
function TextEditor:hasLockedSuggestion() end

---Inserts the passed text at all cursor positions overwriting any selected text.
---@param text string The text to insert.
function TextEditor:insertText(text) end

---Returns the current editor or nil.
---@return TextEditor|nil editor The currently active editor or nil if there is none.
function textEditor.currentEditor() end


return textEditor
