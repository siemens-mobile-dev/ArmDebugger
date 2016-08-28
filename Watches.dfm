object WatchesForm: TWatchesForm
  Left = 71
  Top = 163
  Width = 210
  Height = 100
  BorderStyle = bsSizeToolWin
  Caption = 'Watches'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnCreate = FormCreate
  OnHide = FormHide
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object List: TDrawGrid
    Left = 0
    Top = 0
    Width = 202
    Height = 73
    Align = alClient
    BorderStyle = bsNone
    ColCount = 2
    DefaultColWidth = 100
    DefaultRowHeight = 14
    FixedCols = 0
    RowCount = 1
    FixedRows = 0
    Options = [goVertLine, goHorzLine, goDrawFocusSelected, goRowSelect, goThumbTracking]
    PopupMenu = PopupMenu1
    ScrollBars = ssVertical
    TabOrder = 0
    OnDblClick = Edit1Click
    OnDrawCell = ListDrawCell
  end
  object PopupMenu1: TPopupMenu
    Left = 160
    Top = 24
    object Newwatch1: TMenuItem
      Caption = 'New watch'
      ShortCut = 45
      OnClick = Newwatch1Click
    end
    object ChangeWatch1: TMenuItem
      Caption = 'Change Watch'
      OnClick = ChangeWatch1Click
    end
    object Delete1: TMenuItem
      Caption = 'Delete Watch'
      ShortCut = 46
      OnClick = Delete1Click
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object View1: TMenuItem
      Caption = 'Hex View'
      ShortCut = 8205
      OnClick = View1Click
    end
    object Edit1: TMenuItem
      Caption = 'Edit value'
      ShortCut = 13
      OnClick = Edit1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object Refresh1: TMenuItem
      Caption = 'Refresh'
      ShortCut = 116
      OnClick = Refresh1Click
    end
    object Monitor1: TMenuItem
      Caption = 'Monitor'
      ShortCut = 77
      OnClick = Monitor1Click
    end
    object Stayontop1: TMenuItem
      Caption = 'Stay on top'
      ShortCut = 16507
      OnClick = Stayontop1Click
    end
    object Hide1: TMenuItem
      Caption = 'Hide'
      ShortCut = 27
      OnClick = Hide1Click
    end
  end
end
