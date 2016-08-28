object SnapsForm: TSnapsForm
  Left = 209
  Top = 153
  ActiveControl = SnapGrid
  AutoScroll = False
  Caption = ' Snappoints'
  ClientHeight = 197
  ClientWidth = 427
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object SnapGrid: TStringGrid
    Left = 0
    Top = 0
    Width = 427
    Height = 197
    Align = alClient
    ColCount = 4
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goRangeSelect, goDrawFocusSelected, goRowSelect, goThumbTracking]
    PopupMenu = PopupMenu1
    ScrollBars = ssVertical
    TabOrder = 0
    OnDblClick = Goto1Click
  end
  object PopupMenu1: TPopupMenu
    Left = 320
    Top = 16
    object Goto1: TMenuItem
      Caption = 'Goto'
      ShortCut = 13
      OnClick = Goto1Click
    end
    object Edit1: TMenuItem
      Caption = 'Edit'
      ShortCut = 113
      OnClick = Edit1Click
    end
    object Remove1: TMenuItem
      Caption = 'Remove'
      ShortCut = 46
      OnClick = Remove1Click
    end
    object Removeall1: TMenuItem
      Caption = 'Remove all'
      ShortCut = 16430
      OnClick = Removeall1Click
    end
    object close1: TMenuItem
      Caption = '(close)'
      ShortCut = 27
      Visible = False
      OnClick = close1Click
    end
  end
end
