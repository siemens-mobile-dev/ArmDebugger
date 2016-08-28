object NuForm: TNuForm
  Left = 288
  Top = 195
  AutoScroll = False
  Caption = ' RTOS information'
  ClientHeight = 316
  ClientWidth = 450
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
  object NuPages: TPageControl
    Left = 0
    Top = 0
    Width = 450
    Height = 316
    ActivePage = InfoPage
    Align = alClient
    Constraints.MinHeight = 250
    Constraints.MinWidth = 400
    MultiLine = True
    TabIndex = 0
    TabOrder = 0
    object InfoPage: TTabSheet
      Caption = 'Main'
      DesignSize = (
        442
        288)
      object Label1: TLabel
        Left = 7
        Top = 146
        Width = 42
        Height = 13
        Anchors = [akLeft, akBottom]
        Caption = 'Symbols:'
      end
      object sgSymbols: TStringGrid
        Left = 0
        Top = 159
        Width = 442
        Height = 129
        Align = alBottom
        Anchors = [akLeft, akTop, akRight, akBottom]
        ColCount = 3
        Constraints.MinHeight = 20
        DefaultColWidth = 150
        DefaultRowHeight = 15
        FixedCols = 0
        RowCount = 1
        FixedRows = 0
        Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goRangeSelect, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
      end
      object info1: TStaticText
        Left = 16
        Top = 8
        Width = 193
        Height = 105
        AutoSize = False
        Caption = 'Information not available'
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
      end
    end
    object ProcessPage: TTabSheet
      Caption = 'Processes'
      ImageIndex = 1
      object infoProc: TLabel
        Left = 8
        Top = 3
        Width = 115
        Height = 13
        Caption = 'Information not available'
      end
      object sgProc: TStringGrid
        Left = 0
        Top = 19
        Width = 442
        Height = 269
        Align = alBottom
        Anchors = [akLeft, akTop, akRight, akBottom]
        ColCount = 6
        DefaultColWidth = 96
        DefaultRowHeight = 15
        FixedCols = 0
        Options = [goFixedVertLine, goFixedHorzLine, goColSizing, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
      end
    end
    object HisrPage: TTabSheet
      Caption = 'HISRs'
      ImageIndex = 2
      object infoHisr: TLabel
        Left = 8
        Top = 3
        Width = 115
        Height = 13
        Caption = 'Information not available'
      end
      object sgHisr: TStringGrid
        Left = 0
        Top = 19
        Width = 442
        Height = 269
        Align = alBottom
        Anchors = [akLeft, akTop, akRight, akBottom]
        ColCount = 6
        DefaultColWidth = 96
        DefaultRowHeight = 15
        FixedCols = 0
        Options = [goFixedVertLine, goFixedHorzLine, goColSizing, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
      end
    end
    object DynaPage: TTabSheet
      Caption = 'Memory pools'
      ImageIndex = 3
      object infoDyna: TLabel
        Left = 8
        Top = 3
        Width = 115
        Height = 13
        Caption = 'Information not available'
      end
      object sgDyna: TStringGrid
        Left = 0
        Top = 19
        Width = 442
        Height = 269
        Align = alBottom
        Anchors = [akLeft, akTop, akRight, akBottom]
        ColCount = 6
        DefaultColWidth = 96
        DefaultRowHeight = 15
        FixedCols = 0
        Options = [goFixedVertLine, goFixedHorzLine, goColSizing, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
      end
    end
    object PartPage: TTabSheet
      Caption = 'Partition pools'
      ImageIndex = 4
      object infoPart: TLabel
        Left = 8
        Top = 3
        Width = 115
        Height = 13
        Caption = 'Information not available'
      end
      object sgPart: TStringGrid
        Left = 0
        Top = 19
        Width = 442
        Height = 269
        Align = alBottom
        Anchors = [akLeft, akTop, akRight, akBottom]
        ColCount = 6
        DefaultColWidth = 96
        DefaultRowHeight = 15
        FixedCols = 0
        Options = [goFixedVertLine, goFixedHorzLine, goColSizing, goRowSelect, goThumbTracking]
        ScrollBars = ssVertical
        TabOrder = 0
      end
    end
    object TabSheet7: TTabSheet
      Caption = 'Queues'
      ImageIndex = 6
    end
    object TabSheet10: TTabSheet
      Caption = 'Timers'
      ImageIndex = 9
    end
  end
  object FirstTimer: TTimer
    Enabled = False
    Interval = 200
    OnTimer = FirstTimerTimer
    Left = 376
    Top = 264
  end
  object UpdateTimer: TTimer
    OnTimer = UpdateTimerTimer
    Left = 408
    Top = 264
  end
end
