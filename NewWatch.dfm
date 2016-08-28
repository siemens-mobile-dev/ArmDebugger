object NewWatchForm: TNewWatchForm
  Left = 204
  Top = 116
  BorderStyle = bsDialog
  Caption = ' New Watch'
  ClientHeight = 191
  ClientWidth = 321
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 136
    Top = 11
    Width = 53
    Height = 13
    Caption = 'Reference:'
  end
  object Label2: TLabel
    Left = 136
    Top = 34
    Width = 38
    Height = 13
    Caption = 'Address'
  end
  object lOff1: TLabel
    Left = 152
    Top = 57
    Width = 37
    Height = 13
    Caption = 'Offset 1'
  end
  object lOff2: TLabel
    Left = 168
    Top = 80
    Width = 37
    Height = 13
    Caption = 'Offset 2'
  end
  object lOff3: TLabel
    Left = 184
    Top = 103
    Width = 37
    Height = 13
    Caption = 'Offset 3'
  end
  object lOff4: TLabel
    Left = 200
    Top = 126
    Width = 37
    Height = 13
    Caption = 'Offset 4'
  end
  object Label8: TLabel
    Left = 8
    Top = 125
    Width = 69
    Height = 13
    Caption = 'Length/Count:'
  end
  object Bevel1: TBevel
    Left = 7
    Top = 152
    Width = 306
    Height = 2
  end
  object cbRef: TComboBox
    Left = 192
    Top = 8
    Width = 121
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 0
    Text = 'Direct'
    OnChange = cbRefChange
    Items.Strings = (
      'Direct'
      'Pointer'
      '2-ptr chain'
      '3-ptr chain'
      '4-ptr chain')
  end
  object eAddress: TEdit
    Left = 192
    Top = 31
    Width = 121
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MaxLength = 8
    ParentFont = False
    TabOrder = 1
    Text = '0'
  end
  object eOff1: TEdit
    Left = 208
    Top = 54
    Width = 57
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    Enabled = False
    MaxLength = 4
    TabOrder = 2
    Text = '0'
  end
  object eOff2: TEdit
    Left = 224
    Top = 77
    Width = 57
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    Enabled = False
    MaxLength = 4
    TabOrder = 3
    Text = '0'
  end
  object eOff3: TEdit
    Left = 240
    Top = 100
    Width = 57
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    Enabled = False
    MaxLength = 4
    TabOrder = 4
    Text = '0'
  end
  object eOff4: TEdit
    Left = 256
    Top = 123
    Width = 57
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    Enabled = False
    MaxLength = 4
    TabOrder = 5
    Text = '0'
  end
  object bOk: TButton
    Left = 160
    Top = 160
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
  end
  object bCancel: TButton
    Left = 240
    Top = 160
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 7
  end
  object eLength: TEdit
    Left = 88
    Top = 122
    Width = 41
    Height = 21
    AutoSize = False
    CharCase = ecUpperCase
    MaxLength = 3
    TabOrder = 8
    Text = '0'
  end
  object rType: TRadioGroup
    Left = 8
    Top = 8
    Width = 121
    Height = 105
    Caption = ' Type '
    Items.Strings = (
      'Byte'
      'Halfword'
      'Word'
      'ASCII string'
      'Unicode string')
    TabOrder = 9
    OnClick = rTypeClick
  end
end
