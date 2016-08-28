object CP15Form: TCP15Form
  Left = 826
  Top = 110
  BorderStyle = bsSingle
  Caption = ' CP15 Registers'
  ClientHeight = 368
  ClientWidth = 439
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  PixelsPerInch = 96
  TextHeight = 13
  object List: TStringGrid
    Left = 0
    Top = 0
    Width = 439
    Height = 368
    Align = alClient
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 3
    FixedRows = 2
    Options = [goFixedVertLine, goVertLine, goRowSelect, goThumbTracking]
    PopupMenu = PopupMenu1
    ScrollBars = ssNone
    TabOrder = 0
  end
  object PopupMenu1: TPopupMenu
    Left = 384
    Top = 24
    object Edit1: TMenuItem
      Caption = 'Edit'
      ShortCut = 13
    end
    object Refresh1: TMenuItem
      Caption = 'Refresh'
      ShortCut = 116
      OnClick = Refresh1Click
    end
  end
end
