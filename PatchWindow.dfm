object PatchForm: TPatchForm
  Left = 177
  Top = 43
  BorderStyle = bsDialog
  Caption = ' Patch'
  ClientHeight = 190
  ClientWidth = 324
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  DesignSize = (
    324
    190)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 82
    Height = 13
    Caption = 'Enter patch data:'
  end
  object Dump: TDrawGrid
    Left = 0
    Top = 24
    Width = 324
    Height = 129
    Anchors = [akLeft, akTop, akRight, akBottom]
    ColCount = 16
    Constraints.MinHeight = 40
    DefaultColWidth = 17
    DefaultRowHeight = 15
    FixedCols = 0
    RowCount = 1
    FixedRows = 0
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    Options = [goDrawFocusSelected, goThumbTracking]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    OnDrawCell = DumpDrawCell
    OnKeyPress = DumpKeyPress
    OnSelectCell = DumpSelectCell
  end
  object Button1: TButton
    Left = 163
    Top = 160
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object Button2: TButton
    Left = 243
    Top = 160
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
