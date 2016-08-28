object SearchResForm: TSearchResForm
  Left = 102
  Top = 213
  AutoScroll = False
  Caption = ' Search Results'
  ClientHeight = 330
  ClientWidth = 286
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCloseQuery = FormCloseQuery
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 286
    Height = 311
    Align = alClient
    BevelOuter = bvLowered
    Caption = 'Reading ...'
    Color = clWindow
    TabOrder = 2
  end
  object Grid: TDrawGrid
    Left = 0
    Top = 0
    Width = 286
    Height = 311
    Align = alClient
    ColCount = 2
    DefaultColWidth = 85
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goDrawFocusSelected, goColSizing, goRowSelect, goThumbTracking]
    PopupMenu = PopupMenu1
    TabOrder = 0
    OnDblClick = GridDblClick
    OnDrawCell = GridDrawCell
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 311
    Width = 286
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object PopupMenu1: TPopupMenu
    Left = 240
    Top = 8
    object View1: TMenuItem
      Caption = 'View as Data'
      ShortCut = 13
      OnClick = View1Click
    end
    object ViewasCode1: TMenuItem
      Caption = 'View as Code'
      ShortCut = 16397
      OnClick = ViewasCode1Click
    end
    object Removefromlist1: TMenuItem
      Caption = 'Remove from list'
      ShortCut = 46
      OnClick = Removefromlist1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object RestartSearch1: TMenuItem
      Caption = 'Restart Search'
      ShortCut = 16466
      OnClick = RestartSearch1Click
    end
    object Refresh1: TMenuItem
      Caption = 'Refresh Data'
      ShortCut = 116
      OnClick = Refresh1Click
    end
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 5
    OnTimer = Timer1Timer
    Left = 240
    Top = 48
  end
end
