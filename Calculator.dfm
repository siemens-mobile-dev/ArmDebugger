object CalcForm: TCalcForm
  Left = 470
  Top = 124
  ActiveControl = eDec
  BorderStyle = bsToolWindow
  Caption = 'Calculator'
  ClientHeight = 86
  ClientWidth = 243
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 6
    Top = 7
    Width = 20
    Height = 13
    Caption = 'Dec'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7368816
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 6
    Top = 31
    Width = 19
    Height = 13
    Caption = 'Hex'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7368816
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label3: TLabel
    Left = 6
    Top = 63
    Width = 15
    Height = 13
    Caption = 'Bin'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7368816
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object lSign: TLabel
    Left = 112
    Top = 1
    Width = 15
    Height = 15
    Alignment = taCenter
    AutoSize = False
    Caption = ' '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label6: TLabel
    Left = 198
    Top = 50
    Width = 11
    Height = 10
    Caption = '4|3'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label4: TLabel
    Left = 174
    Top = 50
    Width = 11
    Height = 10
    Caption = '8|7'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 146
    Top = 50
    Width = 19
    Height = 10
    Caption = '12|11'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label7: TLabel
    Left = 122
    Top = 50
    Width = 19
    Height = 10
    Caption = '16|15'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label8: TLabel
    Left = 98
    Top = 50
    Width = 19
    Height = 10
    Caption = '20|19'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label9: TLabel
    Left = 74
    Top = 50
    Width = 19
    Height = 10
    Caption = '24|23'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label10: TLabel
    Left = 50
    Top = 50
    Width = 19
    Height = 10
    Caption = '28|27'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = 7368816
    Font.Height = -8
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object SpeedButton1: TSpeedButton
    Left = 127
    Top = 4
    Width = 19
    Height = 19
    Caption = '+'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mPlusClick
  end
  object SpeedButton2: TSpeedButton
    Left = 145
    Top = 4
    Width = 19
    Height = 19
    Caption = '-'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mMinusClick
  end
  object SpeedButton3: TSpeedButton
    Left = 127
    Top = 22
    Width = 19
    Height = 19
    Caption = '*'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mMultClick
  end
  object SpeedButton4: TSpeedButton
    Left = 145
    Top = 22
    Width = 19
    Height = 19
    Caption = '/'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mDivClick
  end
  object SpeedButton5: TSpeedButton
    Left = 163
    Top = 4
    Width = 19
    Height = 19
    Caption = '&&'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mAndClick
  end
  object SpeedButton6: TSpeedButton
    Left = 181
    Top = 4
    Width = 19
    Height = 19
    Caption = '|'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mOrClick
  end
  object SpeedButton7: TSpeedButton
    Left = 163
    Top = 22
    Width = 19
    Height = 19
    Caption = '^'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mXorClick
  end
  object SpeedButton8: TSpeedButton
    Left = 181
    Top = 22
    Width = 19
    Height = 19
    Caption = '~'
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mNotClick
  end
  object SpeedButton9: TSpeedButton
    Left = 220
    Top = 4
    Width = 19
    Height = 19
    Caption = 'C'
    Font.Charset = ANSI_CHARSET
    Font.Color = 16512
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = [fsBold]
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = Clear1Click
  end
  object SpeedButton10: TSpeedButton
    Left = 220
    Top = 22
    Width = 19
    Height = 19
    Caption = '='
    Font.Charset = ANSI_CHARSET
    Font.Color = clGreen
    Font.Height = -11
    Font.Name = 'Lucida Console'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mEqualClick
  end
  object SpeedButton11: TSpeedButton
    Left = 199
    Top = 4
    Width = 19
    Height = 19
    Caption = '<<'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clGreen
    Font.Height = -9
    Font.Name = 'Small Fonts'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mLSHClick
  end
  object SpeedButton12: TSpeedButton
    Left = 199
    Top = 22
    Width = 19
    Height = 19
    Caption = '>>'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clGreen
    Font.Height = -9
    Font.Name = 'Small Fonts'
    Font.Style = []
    Layout = blGlyphRight
    ParentFont = False
    Spacing = 0
    OnClick = mRSHClick
  end
  object eDec: TEdit
    Left = 32
    Top = 4
    Width = 81
    Height = 21
    MaxLength = 10
    PopupMenu = PopupMenu1
    TabOrder = 0
    Text = '0'
    OnChange = eDecChange
  end
  object eHex: TEdit
    Left = 32
    Top = 28
    Width = 81
    Height = 21
    CharCase = ecUpperCase
    MaxLength = 8
    PopupMenu = PopupMenu1
    TabOrder = 1
    Text = '0'
    OnChange = eHexChange
  end
  object eBin: TEdit
    Left = 32
    Top = 60
    Width = 206
    Height = 21
    BiDiMode = bdLeftToRight
    MaxLength = 33
    ParentBiDiMode = False
    PopupMenu = PopupMenu1
    TabOrder = 2
    Text = '00000000000000000000000000000000'
    OnChange = eBinChange
  end
  object PopupMenu1: TPopupMenu
    Left = 192
    Top = 52
    object Copy1: TMenuItem
      Caption = 'Copy'
      ShortCut = 16451
      OnClick = Copy1Click
    end
    object Paste1: TMenuItem
      Caption = 'Paste'
      ShortCut = 16470
      OnClick = Paste1Click
    end
    object Clear1: TMenuItem
      Caption = 'Clear'
      ShortCut = 27
      OnClick = Clear1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object Stayontop1: TMenuItem
      Caption = 'Stay on top'
      ShortCut = 16507
      OnClick = Stayontop1Click
    end
    object mPlus: TMenuItem
      Caption = '(+)'
      Visible = False
      OnClick = mPlusClick
    end
    object mPlus2: TMenuItem
      Caption = '(+)'
      Visible = False
      OnClick = mPlusClick
    end
    object mMinus: TMenuItem
      Caption = '(-)'
      Visible = False
      OnClick = mMinusClick
    end
    object mMinus2: TMenuItem
      Caption = '(-)'
      Visible = False
      OnClick = mMinusClick
    end
    object mMult: TMenuItem
      Caption = '(*)'
      Visible = False
      OnClick = mMultClick
    end
    object mMul2: TMenuItem
      Caption = '(*)'
      Visible = False
      OnClick = mMultClick
    end
    object mDiv: TMenuItem
      Caption = '(/)'
      Visible = False
      OnClick = mDivClick
    end
    object mDiv2: TMenuItem
      Caption = '(/)'
      Visible = False
      OnClick = mDivClick
    end
    object mEqual: TMenuItem
      Caption = '(=)'
      Visible = False
      OnClick = mEqualClick
    end
    object mEqual2: TMenuItem
      Caption = '(=)'
      Visible = False
      OnClick = mEqualClick
    end
    object mAnd: TMenuItem
      Caption = '(&&)'
      Visible = False
      OnClick = mAndClick
    end
    object mOr: TMenuItem
      Caption = '(|)'
      Visible = False
      OnClick = mOrClick
    end
    object mXor: TMenuItem
      Caption = '(^)'
      Visible = False
      OnClick = mXorClick
    end
    object mNot: TMenuItem
      Caption = '(~)'
      Visible = False
      OnClick = mNotClick
    end
    object mLSH: TMenuItem
      Caption = '(<<)'
      Visible = False
      OnClick = mLSHClick
    end
    object mRSH: TMenuItem
      Caption = '(>>)'
      Visible = False
      OnClick = mRSHClick
    end
  end
end
