object XrefForm: TXrefForm
  Left = 679
  Top = 129
  AutoScroll = False
  Caption = ' References'
  ClientHeight = 265
  ClientWidth = 254
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnClose = FormClose
  OnKeyPress = FormKeyPress
  PixelsPerInch = 96
  TextHeight = 13
  object Grid: TStringGrid
    Left = 0
    Top = 0
    Width = 254
    Height = 246
    Align = alClient
    ColCount = 2
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goColSizing, goRowSelect, goThumbTracking]
    ScrollBars = ssVertical
    TabOrder = 0
    OnDblClick = Goto1Click
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 246
    Width = 254
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object XTimer: TTimer
    Enabled = False
    Interval = 1
    OnTimer = XTimerTimer
    Left = 184
    Top = 8
  end
  object PopupMenu1: TPopupMenu
    Left = 216
    Top = 8
    object Goto1: TMenuItem
      Caption = 'Goto'
      ShortCut = 13
      OnClick = Goto1Click
    end
  end
end
