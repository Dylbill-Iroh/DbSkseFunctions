Scriptname DbColorFunctions Hidden 

;get random rgb color in int array.
Int[] Function GetRandomRGB() Global
    Int[] RGB = New Int[3] 
    Int I = 0 
    While I < 3 
        RGB[I] = Utility.RandomInt(0, 255)
        I += 1 
    EndWhile 
    
    Return RGB
EndFunction 

;get a random hsl color in int array.
Int[] Function GetRandomHSL() Global
    Int[] HSL = New Int[3] 
    HSL[0] = Utility.RandomInt(0, 360)
    HSL[1] = Utility.RandomInt(0, 100)
    HSL[2] = Utility.RandomInt(0, 100)
    
    Return HSL
EndFunction

int[] function colorHexToRGB(string colorHex) Global
    return dbColorFunctions.intToRGB(DbMiscFunctions.ConvertHexToInt(colorHex))
EndFunction

String function RGBToColorHex(int[] rgb) Global
    return DbMiscFunctions.ConvertIntToHex(RGBToInt(rgb[0], rgb[1], rgb[2]))
EndFunction

;Convert R G B to single int (base 10 instead of base 16 for hex)
Int Function RGBToInt(Int R, Int G, Int B) Global
    Int ColorInt = (R * 65536) + (G * 256) + (B) 
    Return colorInt
EndFunction   

;Opposite of RGBToInt. Convert RGBInt (base 10) to seperate R G B values and return float array. [0] = R, [1] = G, [2] = B
Int[] Function IntToRGB(Int RGBInt) Global
    Int[] RGB = New Int[3]

    RGB[0] = RGBInt / 65536 ;256 ^ 2
    RGB[1] = (RGBInt / 256) % 256
    RGB[2] = RGBInt % 256
    return RGB
EndFunction

Int[] Function ColorIntToHSL(Int iColor) Global
    int[] RGB = IntToRGB(iColor)
    return RGBToHSL(RGB[0], RGB[1], RGB[2])

EndFunction

;rgb / hsl conversion functions I wrote from following this guide: 
;https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
;convert RGB to HSL format and return in int array. [0] = H [1] = S [2] = L 
;S and L are ints between 0 and 100 (percent)
int[] Function RGBToHSL(int R, int G, int B) Global 
    
    float fR = (R as float) / 255.0 
    float fG = (G as float) / 255.0 
    float fB = (B as float) / 255.0 
    
    float min   = RGBMin(fR, fG, fB)
    float max   = RGBMax(fR, fG, fB)
    float delta = max - min

    float[] hsl = new float[3]

    hsl[2] = (max + min) / 2

    if (delta == 0)
        hsl[0] = 0.0
        hsl[1] = 0.0
        
    else
        if hsl[2] <= 0.5 
            hsl[1] =  (delta / (max + min)) 
        Else 
            hsl[1] =  (delta / (2.0 - max - min)) 
        Endif
           
        if (fR == max)
            hsl[0] = (fG - fB) / delta
        elseif (fG == max)
            hsl[0] = 2.0 + (fB - fR) / (delta)
        else
            hsl[0] = 4.0 + (fR - fG) / (delta)
        Endif
        
        hsl[0] = hsl[0] * 60.0 
        If hsl[0] < 0 
            hsl[0] = (hsl[0] + 360.0)
        Endif
    Endif
    
    int[] ihsl = New Int[3]
    
    ihsl[0] = DbMiscFunctions.RoundAsInt(hsl[0])
    ihsl[1] = DbMiscFunctions.RoundAsInt( (hsl[1] * 100.0) )
    ihsl[2] = DbMiscFunctions.RoundAsInt( (hsl[2] * 100.0) )
    
    return ihsl
EndFunction 

;convert HSL to RGB format and return in int array. [0] = R [1] = G [2] = B 
;S and L input should be between 0 and 100 (percent)
Int[] Function HSLToRGB(int H, int S, int L) Global
    Float[] RGB = New Float[3]
    int[] iRGB = New Int[3] 
    
    Float fH = H as float
    Float fS = S as float
    Float fL = L as float
    
    fS /= 100.0 
    fL /= 100.0 
    
    If fS == 0.0 
        iRGB[0] = DbMiscFunctions.RoundAsInt((fL * 255.0))
        iRGB[1] = iRGB[0]
        iRGB[2] = iRGB[0]
        Return iRGB 
    Endif
    
    Float t1 
    If fL < 0.5 
        t1 = fL * (fS + 1.0) ;.28 * (1.0 + 0.67) = .4646
    Else 
        t1 = (fL + fS) - (fL * fS)
    Endif
     
    Float t2 = (2.0 * fL) - t1
    float Hue = H / 360.0 
    
    RGB[0] = RGB_ClampBetween0and1( (Hue + (1.0 / 3.0)) )
    RGB[1] = RGB_ClampBetween0and1(Hue)
    RGB[2] = RGB_ClampBetween0and1( (Hue - (1.0 / 3.0)) )
    
    iRGB[0] = DbMiscFunctions.RoundAsInt( (RGBChannel(t1, t2, RGB[0]) * 255.0) )
    iRGB[1] = DbMiscFunctions.RoundAsInt((RGBChannel(t1, t2, RGB[1]) * 255.0) )
    iRGB[2] = DbMiscFunctions.RoundAsInt((RGBChannel(t1, t2, RGB[2]) * 255.0) )
    
    Return iRGB
EndFunction


float Function RGBMin(float r, float g, float b) Global
    If r < g && r < b 
        return r 
    Elseif g < r && g < b 
        return g 
    Else 
        return b 
    Endif
Endfunction


float Function RGBMax(float r, float g, float b) Global
    If r > g && r > b 
        return r 
    Elseif g > r && g > b 
        return g 
    Else 
        return b 
    Endif
EndFunction  

Float Function RGBChannel(Float t1, Float t2, Float c) Global 
    Float Channel
    
    If (6.0 * c) < 1.0 
        Channel = t2 + (t1 -t2) * 6.0 * c
        
    ElseIf (2.0 * c) < 1.0 
        Channel = t1 
     
    ElseIf (3.0 * c) < 2.0 
        Channel = (t2 + (t1 - t2) * ((2.0 / 3.0) - c) * 6.0)
        
    Else 
        Channel = t2 
    Endif
    
    return Channel
EndFunction

Float Function RGB_ClampBetween0and1(float f) Global
    If f > 1.0 
        f -= 1.0 
    Elseif f < 0.0 
        f += 1.0 
    Endif 
    
    return f
EndFunction 

;for use in text replacement, such as in books, or MCM text. 
;Adds color font to string
String Function AddColorFont(String s, Int iColor) Global
    return "<font color='#" + DbMiscFunctions.ConvertIntToHex(iColor) +"'>" + s + "</font>"
EndFunction


