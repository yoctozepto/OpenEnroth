--- @meta

--- @diagnostic disable: duplicate-doc-field

--- @class ImGuiBindings
--- @field Begin fun() : boolean
--- @field Begin fun(name:string) : boolean
--- @field Begin fun(name:string, isOpen:boolean) : boolean, boolean
--- @field Begin fun(name:string, isOpen:boolean, flags:ImGuiWindowFlags) : boolean, boolean
--- @field End fun()
--- Window
--- @field GetWindowSize fun() : number, number
--- @field SetNextWindowSize fun(w: number, h: number)
--- @field SetNextWindowSize fun(w: number, h: number, cond:ImGuiCond)
--- @field SetNextWindowPos fun(x: number, h: number)
--- @field SetNextWindowPos fun(x: number, h: number, cond:ImGuiCond)
--- @field SetNextWindowSizeConstraints fun(minX: number, minH: number, maxX: number, maxH: number)
--- @field IsWindowHovered fun(flags:ImGuiHoveredFlags) : boolean
--- Child
--- @field BeginChild fun(name:string, width:number, height:number, showBorder:boolean) : boolean
--- @field EndChild fun()
--- Text
--- @field Text fun(text:string)
--- @field TextUnformatted fun(text:string)
--- @field TextWrapped fun(text:string)
--- Button
--- @field Button fun() : boolean
--- @field Button fun(text:string, width:number, height: number) : boolean
--- InputText
--- @field InputTextWithHint fun(name:string, hint:string, text:string, flags:ImGuiInputTextFlags, callback: fun(event:ImGuiInputTextFlags, param1:any) : string) : string, boolean
--- Checkbox
--- @field Checkbox fun(label:string, value:boolean) : boolean
--- Table
--- @field BeginTable fun(name:string, columns:integer)
--- @field EndTable fun()
--- @field TableNextRow fun()
--- @field TableSetColumnIndex fun(column:integer)
--- @field TableGetColumnCount fun(): integer
--- Scroll
--- @field SetScrollHereY fun(scroll:number)
--- Layout
--- @field SameLine fun()
--- @field GetFrameHeightWithSpacing fun()
--- Focus
--- @field SetKeyboardFocusHere fun(index:integer)
--- @field IsItemFocused fun() : boolean
--- Style
--- @field PushStyleColor fun(col:ImGuiCol, r: number, g: number, b: number, a: number)
--- @field PopStyleColor fun()
--- Demo
--- @field ShowDemoWindow fun()
--- Mouse
--- @field IsMouseHoveringRect fun(x: number, y: number, w: number, h: number) : boolean
--- Enums
--- @field ImGuiCol ImGuiColEnum
--- @field ImGuiCond ImGuiCondEnum
--- @field ImGuiInputTextFlags ImGuiInputTextFlagsEnum
--- @field ImGuiHoveredFlags ImGuiHoveredFlagsEnum
--- @field ImGuiWindowFlags ImGuiWindowFlagsEnum

--- @enum ImGuiCol
--- @class ImGuiColEnum
--- @field Text ImGuiCol
--- @field TextDisabled ImGuiCol
--- @field WindowBg ImGuiCol
--- @field ChildBg ImGuiCol
--- @field PopupBg ImGuiCol
--- @field Border ImGuiCol
--- @field BorderShadow ImGuiCol
--- @field FrameBg ImGuiCol
--- @field FrameBgHovered ImGuiCol
--- @field FrameBgActive ImGuiCol
--- @field TitleBg ImGuiCol
--- @field TitleBgActive ImGuiCol
--- @field TitleBgCollapsed ImGuiCol
--- @field MenuBarBg ImGuiCol,
--- @field ScrollbarBg ImGuiCol
--- @field ScrollbarGrab ImGuiCol
--- @field ScrollbarGrabHovered ImGuiCol
--- @field ScrollbarGrabActive ImGuiCol
--- @field CheckMark ImGuiCol
--- @field SliderGrab ImGuiCol
--- @field SliderGrabActive ImGuiCol
--- @field Button ImGuiCol
--- @field ButtonHovered ImGuiCol
--- @field ButtonActive ImGuiCol
--- @field Header ImGuiCol
--- @field HeaderHovered ImGuiCol
--- @field HeaderActive ImGuiCol
--- @field Separator ImGuiCol
--- @field SeparatorHovered ImGuiCol
--- @field SeparatorActive ImGuiCol
--- @field ResizeGrip ImGuiCol
--- @field ResizeGripHovered ImGuiCol
--- @field ResizeGripActive ImGuiCol
--- @field Tab ImGuiCol
--- @field TabHovered ImGuiCol
--- @field TabActive ImGuiCol
--- @field TabUnfocused ImGuiCol
--- @field TabUnfocusedActive ImGuiCol
--- @field DockingPreview ImGuiCol
--- @field DockingEmptyBg ImGuiCol
--- @field PlotLines ImGuiCol
--- @field PlotLinesHovered ImGuiCol
--- @field PlotHistogram ImGuiCol
--- @field PlotHistogramHovered ImGuiCol
--- @field TableHeaderBg ImGuiCol
--- @field TableBorderStrong ImGuiCol
--- @field TableBorderLight ImGuiCol
--- @field TableRowBg ImGuiCol
--- @field TableRowBgAlt ImGuiCol
--- @field TextSelectedBg ImGuiCol
--- @field DragDropTarget ImGuiCol
--- @field NavHighlight ImGuiCol
--- @field NavWindowingHighlight ImGuiCol
--- @field NavWindowingDimBg ImGuiCol
--- @field ModalWindowDimBg ImGuiCol

--- @enum ImGuiCond
--- @class ImGuiCondEnum
--- @field None ImGuiCond
--- @field Always ImGuiCond
--- @field Once ImGuiCond
--- @field FirstUseEver ImGuiCond
--- @field Appearing ImGuiCond

--- @enum ImGuiInputTextFlags
--- @class ImGuiInputTextFlagsEnum
--- @field None ImGuiInputTextFlags
--- @field CharsDecimal ImGuiInputTextFlags
--- @field CharsHexadecimal ImGuiInputTextFlags
--- @field CharsUppercase ImGuiInputTextFlags
--- @field CharsNoBlank ImGuiInputTextFlags
--- @field AutoSelectAll ImGuiInputTextFlags
--- @field EnterReturnsTrue ImGuiInputTextFlags
--- @field CallbackCompletion ImGuiInputTextFlags
--- @field CallbackHistory ImGuiInputTextFlags
--- @field CallbackAlways ImGuiInputTextFlags
--- @field CallbackCharFilter ImGuiInputTextFlags
--- @field AllowTabInput ImGuiInputTextFlags
--- @field CtrlEnterForNewLine ImGuiInputTextFlags
--- @field NoHorizontalScroll ImGuiInputTextFlags
--- @field AlwaysOverwrite ImGuiInputTextFlags
--- @field ReadOnly ImGuiInputTextFlags
--- @field Password ImGuiInputTextFlags
--- @field NoUndoRedo ImGuiInputTextFlags
--- @field CharsScientific ImGuiInputTextFlags
--- @field CallbackResize ImGuiInputTextFlags

--- @enum ImGuiHoveredFlags
--- @class ImGuiHoveredFlagsEnum
--- @field None ImGuiHoveredFlags
--- @field ChildWindows ImGuiHoveredFlags
--- @field RootWindow ImGuiHoveredFlags
--- @field AnyWindow ImGuiHoveredFlags
--- @field AllowWhenBlockedByPopup ImGuiHoveredFlags
--- @field AllowWhenBlockedByActiveItem ImGuiHoveredFlags
--- @field AllowWhenOverlapped ImGuiHoveredFlags
--- @field AllowWhenDisabled ImGuiHoveredFlags
--- @field RectOnly ImGuiHoveredFlags
--- @field RootAndChildWindows ImGuiHoveredFlags

--- @enum ImGuiWindowFlags
--- @class ImGuiWindowFlagsEnum
--- @field None ImGuiWindowFlags
--- @field NoTitleBar ImGuiWindowFlags
--- @field NoResize ImGuiWindowFlags
--- @field NoMove ImGuiWindowFlags
--- @field NoScrollbar ImGuiWindowFlags
--- @field NoScrollWithMouse ImGuiWindowFlags
--- @field NoCollapse ImGuiWindowFlags
--- @field AlwaysAutoResize ImGuiWindowFlags
--- @field NoBackground ImGuiWindowFlags
--- @field NoSavedSettings ImGuiWindowFlags
--- @field NoMouseInputs ImGuiWindowFlags
--- @field MenuBar ImGuiWindowFlags
--- @field HorizontalScrollbar ImGuiWindowFlags
--- @field NoFocusOnAppearing ImGuiWindowFlags
--- @field NoBringToFrontOnFocus ImGuiWindowFlags
--- @field AlwaysVerticalScrollbar ImGuiWindowFlags
--- @field AlwaysHorizontalScrollbar ImGuiWindowFlags
--- @field AlwaysUseWindowPadding ImGuiWindowFlags
--- @field NoNavInputs ImGuiWindowFlags
--- @field NoNavFocus ImGuiWindowFlags
--- @field UnsavedDocument ImGuiWindowFlags
--- @field NoDocking ImGuiWindowFlags
--- @field NoNav ImGuiWindowFlags
--- @field NoDecoration ImGuiWindowFlags
--- @field NoInputs ImGuiWindowFlags
--- @field NavFlattened ImGuiWindowFlags
--- @field ChildWindow ImGuiWindowFlags
--- @field Tooltip ImGuiWindowFlags
--- @field Popup ImGuiWindowFlags
--- @field Modal ImGuiWindowFlags
--- @field ChildMenu ImGuiWindowFlags
--- @field DockNodeHost ImGuiWindowFlags
