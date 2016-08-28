object MMapView: TMMapView
  Left = 569
  Top = 251
  AutoScroll = False
  Caption = ' Memory Map'
  ClientHeight = 346
  ClientWidth = 434
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  PixelsPerInch = 96
  TextHeight = 13
  object pReading: TPanel
    Left = 0
    Top = 0
    Width = 434
    Height = 346
    Align = alClient
    BevelOuter = bvNone
    BorderStyle = bsSingle
    Caption = 'Reading ...'
    Color = clWindow
    TabOrder = 0
    Visible = False
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 434
    Height = 346
    Align = alClient
    BevelOuter = bvNone
    Caption = 'Panel1'
    TabOrder = 1
    object Splitter1: TSplitter
      Left = 0
      Top = 201
      Width = 434
      Height = 3
      Cursor = crVSplit
      Align = alTop
    end
    object L1grid: TStringGrid
      Left = 0
      Top = 0
      Width = 434
      Height = 201
      Align = alTop
      DefaultRowHeight = 16
      FixedCols = 0
      Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRowSelect, goThumbTracking]
      ScrollBars = ssVertical
      TabOrder = 0
      OnDblClick = L1gridClick
      OnSelectCell = L1gridSelectCell
    end
    object Panel2: TPanel
      Left = 0
      Top = 204
      Width = 434
      Height = 142
      Align = alClient
      BevelOuter = bvNone
      BorderStyle = bsSingle
      Color = clWindow
      TabOrder = 1
      object L2grid: TStringGrid
        Left = 0
        Top = 0
        Width = 430
        Height = 138
        Align = alClient
        BorderStyle = bsNone
        DefaultRowHeight = 16
        FixedCols = 0
        Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
        Visible = False
        OnDblClick = L2gridClick
        ColWidths = (
          64
          64
          64
          64
          64)
      end
    end
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 100
    OnTimer = Timer1Timer
    Left = 392
    Top = 8
  end
  object Timer2: TTimer
    Enabled = False
    OnTimer = Timer2Timer
    Left = 392
    Top = 40
  end
end
